#include "editorview.h"
#include "MathConverter.h"
#include "gitpanel.h"

#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QToolBar>
#include <QListWidget>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QFont>
#include <QScrollBar>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextCursor>
#include <QColor>
#include <QDockWidget>
#include <QProcess>
#include <QGridLayout>
#include <QDebug>
#include <QApplication>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPageSize>
#include <QClipboard>
#include <QMenu>
#include <QWidgetAction>

EditorView::EditorView(QWidget *parent)
    : QWidget(parent)
    , m_autoSaveTimer(new QTimer(this))
{
    setStyleSheet(
        "QWidget { background-color: #1E1E1E; color: #D4D4D4; }"
        "QToolBar { background-color: #252526; border: none; spacing: 5px; padding: 5px; }"
        "QStatusBar { background-color: #007ACC; color: #FFFFFF; border: none; }"
        "QListWidget { background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3E3E42; border-radius: 6px; }"
        "QListWidget::item { padding: 10px 8px; }"
        "QListWidget::item:selected { background-color: #3730A3; color: #FFFFFF; }"
        "QListWidget::item:hover:!selected { background-color: #2D2D2D; }"
        "QPushButton { background-color: transparent; color: #D4D4D4; border: none; padding: 8px 12px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3E3E42; }"
    );

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // LEFT SIDEBAR
    m_sidebar = new QWidget(this);
    m_sidebar->setMaximumWidth(220);
    m_sidebar->setMinimumWidth(160);
    m_sidebar->setStyleSheet("background-color: #252526;");
    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    sidebarLayout->setSpacing(10);

    QPushButton *backBtn = new QPushButton("  ←  Back to Subjects", m_sidebar);
    backBtn->setStyleSheet("QPushButton { background-color: transparent; color: #D4D4D4; border: 1px solid #3E3E42; border-radius: 6px; padding: 10px; text-align: left; } QPushButton:hover { background-color: #3E3E42; border-color: #7C3AED; }");
    sidebarLayout->addWidget(backBtn);

    m_folderLabel = new QLabel(m_sidebar);
    m_folderLabel->setStyleSheet("font-size: 11px; color: #808080; padding: 5px 0;");
    m_folderLabel->setWordWrap(true);
    sidebarLayout->addWidget(m_folderLabel);

    QLabel *filesLabel = new QLabel("  📝 Notes", m_sidebar);
    filesLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: #808080; padding: 10px 0 5px 0;");
    sidebarLayout->addWidget(filesLabel);

    m_fileList = new QListWidget(m_sidebar);
    m_fileList->setFont(QFont("Segoe UI", 11));
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_fileList->installEventFilter(this);
    connect(m_fileList, &QListWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QListWidgetItem *item = m_fileList->itemAt(pos);
        if (!item) return;
        
        QMenu menu(m_fileList);
        menu.addAction("Copy", this, [this, item]() {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(item->text());
        });
        menu.addAction("Cut", this, [this, item]() {
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(item->text());
            m_cutItem = item->text();
            m_cutSourceFolder = m_folderPath;
        });
        menu.addAction("Paste", this, [this]() {
            pasteCutItem();
        });
        menu.addSeparator();
        menu.addAction("Rename", this, [this, item]() {
            bool ok;
            QString newName = QInputDialog::getText(this, "Rename Note", "Enter new name:", QLineEdit::Normal, item->text(), &ok);
            if (ok && !newName.isEmpty() && newName != item->text()) {
                if (newName.endsWith(".md")) newName.chop(3);
                QString oldPath = m_folderPath + "/" + item->text() + ".md";
                QString newPath = m_folderPath + "/" + newName + ".md";
                if (QFile::rename(oldPath, newPath)) {
                    refreshFileList();
                }
            }
        });
        menu.addAction("Delete", this, [this, item]() {
            QString filePath = m_folderPath + "/" + item->text() + ".md";
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Note",
                "Delete '" + item->text() + "'?", QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                if (QFile::remove(filePath)) {
                    refreshFileList();
                }
            }
        });
        menu.exec(QCursor::pos());
    });
    sidebarLayout->addWidget(m_fileList, 1);

    QPushButton *addBtn = new QPushButton("  +  New Note", m_sidebar);
    addBtn->setStyleSheet("QPushButton { background-color: #10B981; color: white; border: none; border-radius: 8px; padding: 12px; font-weight: 500; } QPushButton:hover { background-color: #059669; }");
    sidebarLayout->addWidget(addBtn);

    mainLayout->addWidget(m_sidebar);

    // RIGHT SIDE
    QWidget *editorArea = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorArea);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    // Toolbar
    m_toolbar = new QToolBar(editorArea);
    m_toolbar->setMovable(false);

    QAction *newAct = new QAction("New", this);
    newAct->setShortcut(QKeySequence::New);

    QAction *saveAct = new QAction("Save", this);
    saveAct->setShortcut(QKeySequence::Save);

    QAction *convertAct = new QAction("Convert (F5)", this);
    convertAct->setShortcut(Qt::Key_F5);

    QAction *searchAct = new QAction("Search", this);
    searchAct->setShortcut(QKeySequence::Find);
    searchAct->setCheckable(true);

    // Export button with dropdown
    QPushButton *exportBtn = new QPushButton("📥 Export", m_toolbar);
    exportBtn->setStyleSheet("QPushButton { background-color: transparent; color: #D4D4D4; border: none; padding: 8px 12px; border-radius: 4px; } QPushButton:hover { background-color: #3E3E42; }");
    QMenu *exportMenu = new QMenu(m_toolbar);
    exportMenu->addAction("Download as .md", this, [this]() {
        if (m_currentFile.isEmpty()) {
            QMessageBox::warning(this, "No File", "Create or open a note first.");
            return;
        }
        QString fileName = QFileInfo(m_currentFile).fileName();
        QString savePath = QFileDialog::getSaveFileName(this, "Save as .md", 
            QDir::homePath() + "/" + fileName, "Markdown (*.md)");
        if (!savePath.isEmpty()) {
            QFile::copy(m_currentFile, savePath);
            m_statusBar->showMessage("Exported to: " + savePath, 3000);
        }
    });
    exportMenu->addAction("Export as PDF", this, [this]() {
        if (m_currentFile.isEmpty()) {
            QMessageBox::warning(this, "No File", "Create or open a note first.");
            return;
        }
        QString fileName = QFileInfo(m_currentFile).baseName();
        QString savePath = QFileDialog::getSaveFileName(this, "Save as PDF",
            QDir::homePath() + "/" + fileName + ".pdf", "PDF (*.pdf)");
        if (!savePath.isEmpty()) {
            // Create PDF using QTextDocument
            QTextDocument doc;
            doc.setMarkdown(m_textEdit->toPlainText());
            
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFileName(savePath);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setPageSize(QPageSize::A4);
            
            doc.print(&printer);
            m_statusBar->showMessage("Exported PDF to: " + savePath, 3000);
        }
    });
    exportBtn->setMenu(exportMenu);

    QAction *terminalAct = new QAction("Terminal (Ctrl+J)", this);
    terminalAct->setCheckable(true);
    terminalAct->setShortcut(Qt::CTRL | Qt::Key_J);

    // AI Agents menu button
    QWidgetAction *agentsBtn = new QWidgetAction(m_toolbar);
    QPushButton *agentsMenuBtn = new QPushButton("🤖 Agents", m_toolbar);
    agentsMenuBtn->setStyleSheet("QPushButton { background-color: transparent; color: #D4D4D4; border: none; padding: 8px 12px; border-radius: 4px; } QPushButton:hover { background-color: #3E3E42; }");
    QMenu *agentsMenu = new QMenu(m_toolbar);
    agentsMenu->addAction("PI (pi-coding-agent)", [this]() { spawnAgent("pi"); });
    agentsMenu->addAction("Claude CLI", [this]() { spawnAgent("claude"); });
    agentsMenu->addAction("Ghostty (Terminal)", [this]() { spawnAgent("ghostty"); });
    agentsMenuBtn->setMenu(agentsMenu);
    agentsBtn->setDefaultWidget(agentsMenuBtn);

    // Git panel - docked on right side
    m_gitDock = new QDockWidget("Source Control", this);
    m_gitDock->setAllowedAreas(Qt::RightDockWidgetArea);
    m_gitDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_gitDock->setStyleSheet("QDockWidget { background-color: #1E1E1E; } QDockWidget::title { background-color: #252526; } ");
    m_gitPanel = new GitPanel(m_folderPath, this);
    m_gitDock->setWidget(m_gitPanel);
    m_gitDock->setMinimumWidth(320);
    m_gitDock->setMaximumWidth(400);
    
    // Git button (toggle)
    QAction *gitAct = new QAction("🔀 Git", this);
    gitAct->setCheckable(true);
    connect(gitAct, &QAction::toggled, [this](bool checked) {
        m_gitDock->setVisible(checked);
    });

    m_toolbar->addAction(newAct);
    m_toolbar->addAction(saveAct);
    m_toolbar->addSeparator();
    m_toolbar->addAction(convertAct);
    m_toolbar->addAction(searchAct);
    m_toolbar->addSeparator();
    
    // Add export button to toolbar
    QWidgetAction *exportAction = new QWidgetAction(m_toolbar);
    exportAction->setDefaultWidget(exportBtn);
    m_toolbar->addAction(exportAction);
    
    m_toolbar->addAction(terminalAct);
    m_toolbar->addAction(agentsBtn);
    m_toolbar->addAction(gitAct);

    editorLayout->addWidget(m_toolbar);

    // Search bar
    m_searchBar = new QWidget(editorArea);
    m_searchBar->setStyleSheet("background-color: #2D2D30; border-bottom: 1px solid #3E3E42; padding: 8px;");
    m_searchBar->setVisible(false);

    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchBar);
    searchLayout->setContentsMargins(10, 5, 10, 5);
    searchLayout->setSpacing(10);

    m_searchInput = new QLineEdit(m_searchBar);
    m_searchInput->setPlaceholderText("Search...");
    m_searchInput->setStyleSheet("QLineEdit { background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3E3E42; border-radius: 4px; padding: 6px; } QLineEdit:focus { border-color: #7C3AED; }");
    m_searchInput->setMinimumWidth(250);
    searchLayout->addWidget(m_searchInput);

    m_searchLabel = new QLabel("", m_searchBar);
    m_searchLabel->setStyleSheet("color: #808080;");
    searchLayout->addWidget(m_searchLabel);
    searchLayout->addStretch();

    m_searchPrevBtn = new QPushButton("▲", m_searchBar);
    m_searchPrevBtn->setFixedSize(30, 30);
    searchLayout->addWidget(m_searchPrevBtn);

    m_searchNextBtn = new QPushButton("▼", m_searchBar);
    m_searchNextBtn->setFixedSize(30, 30);
    searchLayout->addWidget(m_searchNextBtn);

    m_searchCloseBtn = new QPushButton("✕", m_searchBar);
    m_searchCloseBtn->setFixedSize(30, 30);
    searchLayout->addWidget(m_searchCloseBtn);

    editorLayout->addWidget(m_searchBar);

    // Text editor
    m_textEdit = new QPlainTextEdit(editorArea);
    m_textEdit->setFont(QFont("Consolas", 14));
    m_textEdit->setStyleSheet(
        "QPlainTextEdit { background-color: #1E1E1E; color: #D4D4D4; border: none; padding: 10px; }"
        "QScrollBar:vertical { background-color: #1E1E1E; width: 12px; }"
        "QScrollBar::handle:vertical { background-color: #424242; border-radius: 6px; }"
    );
    m_textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    editorLayout->addWidget(m_textEdit, 1);

    // Status bar
    m_statusBar = new QStatusBar(editorArea);
    editorLayout->addWidget(m_statusBar);

    editorArea->setLayout(editorLayout);
    mainLayout->addWidget(editorArea, 1);

    // Terminal Dock (hidden by default) - using QTermWidget for real embedded terminal
    m_terminalDock = new QDockWidget("Terminal", this);
    m_terminalDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_terminalDock->setMaximumHeight(300);
    m_terminalDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_terminalDock->setStyleSheet("QDockWidget { background-color: #1E1E1E; }");

    // Create real terminal widget
    m_terminal = new QTermWidget(m_terminalDock);
    
    // Set terminal color scheme to dark
    m_terminal->setColorScheme("DarkPastels");
    
    // Set initial working directory
    if (!m_folderPath.isEmpty()) {
        m_terminal->setWorkingDirectory(m_folderPath);
    }
    
    // Start shell with default shell
    m_terminal->startShellProgram();
    
    m_terminalDock->setWidget(m_terminal);
    m_terminalDock->hide();

    // CONNECTIONS
    connect(backBtn, &QPushButton::clicked, this, &EditorView::backRequested);
    connect(addBtn, &QPushButton::clicked, this, &EditorView::onAddClicked);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &EditorView::onFileSelected);
    connect(newAct, &QAction::triggered, this, &EditorView::onNew);
    connect(saveAct, &QAction::triggered, this, &EditorView::onSave);
    connect(convertAct, &QAction::triggered, this, &EditorView::convertMathInput);
    connect(searchAct, &QAction::triggered, this, &EditorView::onSearchToggle);
    connect(terminalAct, &QAction::toggled, [this](bool checked) {
        m_terminalDock->setVisible(checked);
    });
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextChanged);
    connect(m_textEdit, &QPlainTextEdit::cursorPositionChanged, this, &EditorView::onCursorPositionChanged);
    connect(m_searchInput, &QLineEdit::textChanged, this, &EditorView::onSearchTextChanged);
    connect(m_searchNextBtn, &QPushButton::clicked, this, &EditorView::onSearchNext);
    connect(m_searchPrevBtn, &QPushButton::clicked, this, &EditorView::onSearchPrevious);
    connect(m_searchCloseBtn, &QPushButton::clicked, this, &EditorView::onSearchToggle);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &EditorView::autoSave);
    m_autoSaveTimer->start(m_autoSaveInterval);

    updateStatusBar();
}

EditorView::~EditorView()
{
    m_autoSaveTimer->stop();
}

void EditorView::setFolder(const QString &folderPath)
{
    m_folderPath = folderPath;
    m_folderLabel->setText("📂 " + QFileInfo(folderPath).fileName());
    refreshFileList();
    m_textEdit->clear();
    m_currentFile.clear();
    m_isUnsaved = false;
    updateStatusBar();

    // QTermWidget cannot retroactively change a running shell's PWD,
    // so send a `cd` to the live shell. Escape single-quotes in the path.
    if (m_terminal && !folderPath.isEmpty()) {
        QString escaped = folderPath;
        escaped.replace('\'', "'\\''");
        m_terminal->sendText("cd '" + escaped + "'\n");
    }

    if (m_gitPanel) {
        m_gitPanel->setRepoPath(folderPath);
    }
}

void EditorView::loadFile(const QString &filePath)
{
    if (m_isUnsaved && !m_currentFile.isEmpty()) {
        if (!saveCurrentFile()) {
            QMessageBox::StandardButton reply = QMessageBox::warning(
                this, "Save Failed",
                "Could not save the current note. Discard unsaved changes and continue?",
                QMessageBox::Discard | QMessageBox::Cancel);
            if (reply != QMessageBox::Discard)
                return;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Failed",
            "Could not open '" + QFileInfo(filePath).fileName() + "': " + file.errorString());
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    m_textEdit->setPlainText(in.readAll());
    file.close();

    m_currentFile = filePath;
    m_isUnsaved = false;
    updateStatusBar();
}

void EditorView::onNew()
{
    if (m_isUnsaved && !m_currentFile.isEmpty()) {
        saveCurrentFile();
    }
    m_textEdit->clear();
    m_currentFile.clear();
    m_isUnsaved = false;
    updateStatusBar();
}

void EditorView::onSave()
{
    if (m_currentFile.isEmpty()) {
        onAddClicked();
        return;
    }

    if (saveCurrentFile()) {
        m_statusBar->showMessage("✓ Saved", 2000);
    }
}

void EditorView::onAddClicked()
{
    bool ok;
    QString baseName = QInputDialog::getText(this, "New Note", "Enter note name:", QLineEdit::Normal, "untitled", &ok);

    if (ok && !baseName.isEmpty()) {
        if (baseName.endsWith(".md"))
            baseName.chop(3);

        QString filePath = m_folderPath + "/" + baseName + ".md";

        if (QFileInfo::exists(filePath)) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "File Exists",
                "A note with this name already exists. Overwrite?", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes)
                return;
        }

        if (m_isUnsaved && !m_currentFile.isEmpty()) {
            saveCurrentFile();
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Create Failed",
                "Could not create note: " + file.errorString());
            return;
        }
        file.close();

        m_textEdit->clear();
        m_currentFile = filePath;
        m_isUnsaved = false;
        refreshFileList();
        updateStatusBar();
    }
}

void EditorView::onFileSelected(QListWidgetItem *item)
{
    if (item->text() == "(No notes yet)")
        return;
    loadFile(m_folderPath + "/" + item->text() + ".md");
}

void EditorView::onTextChanged()
{
    if (!m_isUnsaved) {
        m_isUnsaved = true;
        updateStatusBar();
    }
    
    // Auto-calculate: check if current word ends with =
    autoCalcResult();
}

void EditorView::autoCalcResult()
{
    if (m_isCalculating) return;
    
    QTextCursor cursor = m_textEdit->textCursor();
    QString fullText = m_textEdit->toPlainText();
    int cursorPos = cursor.position();
    
    if (cursorPos == 0) return;
    
    // Find the current word (chars before cursor)
    int wordEnd = cursorPos;
    while (wordEnd > 0 && !fullText[wordEnd - 1].isSpace() && fullText[wordEnd - 1] != '\n') {
        wordEnd--;
    }
    
    QString word = fullText.mid(wordEnd, cursorPos - wordEnd);
    
    // Check if word ENDS with = (user just typed =)
    if (word.endsWith('=')) {
        // Extract expression before =
        QString expr = word.left(word.length() - 1).trimmed();
        if (expr.isEmpty()) return;
        
        // Check if result already exists after =
        int eqStart = wordEnd + word.length() - 1;
        int nextCharPos = eqStart + 1;
        if (nextCharPos < fullText.length()) {
            QChar nextChar = fullText[nextCharPos];
            if (nextChar.isDigit()) {
                // Result already there
                return;
            }
        }
        
        // Evaluate the expression
        int charsToDelete = 0;
        QString result = MathConverter::evaluate(expr + "=", charsToDelete);
        
        if (!result.isEmpty()) {
            m_isCalculating = true;
            cursor.insertText(" " + result);
            m_textEdit->setTextCursor(cursor);
            m_isCalculating = false;
        }
    }
}

void EditorView::onCursorPositionChanged()
{
    updateLineColumnIndicator();
    highlightCurrentLine();
}

void EditorView::autoSave()
{
    if (m_isUnsaved && !m_currentFile.isEmpty()) {
        saveCurrentFile();
        m_statusBar->showMessage("✓ Auto-saved", 1500);
    }
}

void EditorView::onSearchToggle()
{
    m_searchVisible = !m_searchVisible;
    m_searchBar->setVisible(m_searchVisible);
    if (m_searchVisible) {
        m_searchInput->setFocus();
        m_searchInput->selectAll();
    } else {
        m_searchInput->clear();
        m_searchLabel->clear();
        QList<QTextEdit::ExtraSelection> highlights;
        m_textEdit->setExtraSelections(highlights);
    }
}

void EditorView::onSearchTextChanged(const QString &text)
{
    highlightAllMatches(text);
}

void EditorView::onSearchNext()
{
    QTextDocument *doc = m_textEdit->document();
    QTextCursor cursor = m_textEdit->textCursor();
    cursor = doc->find(m_searchInput->text(), cursor);
    if (cursor.isNull())
        cursor = doc->find(m_searchInput->text());
    if (!cursor.isNull())
        m_textEdit->setTextCursor(cursor);
}

void EditorView::onSearchPrevious()
{
    QTextDocument *doc = m_textEdit->document();
    QTextCursor cursor = m_textEdit->textCursor();
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    cursor = doc->find(m_searchInput->text(), cursor, flags);
    if (cursor.isNull())
        cursor = doc->find(m_searchInput->text(), QTextCursor(doc), flags);
    if (!cursor.isNull())
        m_textEdit->setTextCursor(cursor);
}

void EditorView::highlightAllMatches(const QString &searchText)
{
    QList<QTextEdit::ExtraSelection> highlights;
    if (!searchText.isEmpty()) {
        QTextDocument *doc = m_textEdit->document();
        QTextCursor cursor(doc);
        int matchCount = 0;
        while (true) {
            cursor = doc->find(searchText, cursor);
            if (cursor.isNull())
                break;
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor("#4B4B00"));
            selection.format.setForeground(QColor("#FFFFFF"));
            selection.cursor = cursor;
            highlights.append(selection);
            matchCount++;
        }
        m_searchLabel->setText(QString("%1 match%2").arg(matchCount).arg(matchCount == 1 ? "" : "es"));
    } else {
        m_searchLabel->setText("");
    }
    m_textEdit->setExtraSelections(highlights);
}

void EditorView::refreshFileList()
{
    m_fileList->clear();
    QDir dir(m_folderPath);
    QStringList filters;
    filters << "*.md";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QFileInfo &file : files) {
        m_fileList->addItem(file.baseName());
    }

    if (files.isEmpty()) {
        m_fileList->addItem("(No notes yet)");
    }
}

bool EditorView::saveCurrentFile()
{
    if (m_currentFile.isEmpty())
        return false;

    QFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_textEdit->toPlainText();
    file.close();

    m_isUnsaved = false;
    updateStatusBar();
    return true;
}

void EditorView::updateStatusBar()
{
    QString fileName = m_currentFile.isEmpty() ? "Untitled" : QFileInfo(m_currentFile).baseName();
    int charCount = m_textEdit->toPlainText().length();
    QString text = m_textEdit->toPlainText();
    int wordCount = text.isEmpty() ? 0 : text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    QString unsaved = m_isUnsaved ? " ●" : "";
    
    m_statusBar->showMessage(QString("  %1%2  |  Words: %3  |  Chars: %4  |  Ln: %5  Col: %6")
        .arg(fileName).arg(unsaved).arg(wordCount).arg(charCount)
        .arg(m_textEdit->textCursor().blockNumber() + 1)
        .arg(m_textEdit->textCursor().columnNumber() + 1));
}

void EditorView::updateLineColumnIndicator()
{
    updateStatusBar();
}

void EditorView::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!m_textEdit->isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor("#2D2D30"));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = m_textEdit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    m_textEdit->setExtraSelections(extraSelections);
}

void EditorView::keyPressEvent(QKeyEvent *event)
{
    // Ctrl+S to save
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_S) {
        onSave();
        event->accept();
        return;
    }
    
    // Ctrl+F to search
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_F) {
        onSearchToggle();
        event->accept();
        return;
    }
    
    // Ctrl+N for new
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_N) {
        onNew();
        event->accept();
        return;
    }
    
    // Ctrl+Backspace / Alt+Backspace / Alt+Delete / Ctrl+Delete to delete line
    if ((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) && 
        (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)) {
        deleteCurrentLine();
        event->accept();
        return;
    }
    
    // Ctrl+J for terminal toggle (portable across keyboard layouts)
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_J) {
        bool visible = m_terminalDock->isVisible();
        m_terminalDock->setVisible(!visible);
        event->accept();
        return;
    }
    
    // F5 triggers math conversion
    if (event->key() == Qt::Key_F5) {
        convertMathInput();
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);
}

bool EditorView::convertMathInput()
{
    QTextCursor cursor = m_textEdit->textCursor();
    QString fullText = m_textEdit->toPlainText();
    int cursorPos = cursor.position();
    
    if (cursorPos == 0) {
        return false;
    }
    
    int wordEnd = cursorPos;
    while (wordEnd > 0 && !fullText[wordEnd - 1].isSpace()) {
        wordEnd--;
    }
    
    QString word = fullText.mid(wordEnd, cursorPos - wordEnd);
    
    if (word.isEmpty()) {
        return false;
    }
    
    int charsToDelete = 0;
    QString converted;
    
    // First try inline calculator (expressions with =)
    if (word.contains('=')) {
        converted = MathConverter::evaluate(word, charsToDelete);
    }
    
    // If not an expression, try math symbol conversion
    if (converted.isEmpty()) {
        converted = MathConverter::convert(word, charsToDelete);
    }
    
    if (!converted.isEmpty() && charsToDelete > 0) {
        QTextCursor delCursor(m_textEdit->document());
        delCursor.setPosition(wordEnd);
        delCursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
        delCursor.insertText(converted);
        
        onTextChanged();
        return true;
    }
    
    return false;
}

void EditorView::deleteCurrentLine()
{
    QTextCursor cursor = m_textEdit->textCursor();
    QString fullText = m_textEdit->toPlainText();
    int cursorPos = cursor.position();
    
    int lineStart = cursorPos;
    while (lineStart > 0 && fullText[lineStart - 1] != '\n') {
        lineStart--;
    }
    
    int lineEnd = cursorPos;
    while (lineEnd < fullText.length() && fullText[lineEnd] != '\n') {
        lineEnd++;
    }
    
    if (lineEnd < fullText.length() && fullText[lineEnd] == '\n') {
        lineEnd++;
    }
    
    QString before = fullText.left(lineStart);
    QString after = fullText.mid(lineEnd);
    
    m_textEdit->setPlainText(before + after);
    
    int newPos = qMin(lineStart, before.length());
    QTextCursor newCursor = m_textEdit->textCursor();
    newCursor.setPosition(newPos);
    m_textEdit->setTextCursor(newCursor);
    
    onTextChanged();
}

void EditorView::openSystemTerminal()
{
    spawnAgent("ghostty");
}

void EditorView::spawnAgent(const QString &agent)
{
    QString command = "ghostty";
    QStringList args;

    if (agent == "pi") {
        args << "-e" << "pi";
    } else if (agent == "claude") {
        args << "-e" << "claude";
    } else if (agent != "ghostty") {
        args << "-e" << agent;
    }

    QProcess *process = new QProcess(this);
    if (!m_folderPath.isEmpty()) {
        process->setWorkingDirectory(m_folderPath);
    }
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            process, &QObject::deleteLater);
    connect(process, &QProcess::errorOccurred, process, &QObject::deleteLater);
    process->start(command, args);
}

void EditorView::pasteCutItem()
{
    if (m_cutItem.isEmpty())
        return;

    QString srcFolder = m_cutSourceFolder.isEmpty() ? m_folderPath : m_cutSourceFolder;
    QString srcPath = srcFolder + "/" + m_cutItem + ".md";

    if (!QFileInfo::exists(srcPath)) {
        m_statusBar->showMessage("Cut source no longer exists", 3000);
        m_cutItem.clear();
        m_cutSourceFolder.clear();
        return;
    }

    QString newPath = m_folderPath + "/" + m_cutItem + ".md";
    QString baseName = m_cutItem;
    int counter = 1;
    while (QFileInfo::exists(newPath) && QFileInfo(newPath).absoluteFilePath() != QFileInfo(srcPath).absoluteFilePath()) {
        baseName = QString("%1 (%2)").arg(m_cutItem).arg(counter++);
        newPath = m_folderPath + "/" + baseName + ".md";
    }

    if (QFileInfo(srcPath).absoluteFilePath() == QFileInfo(newPath).absoluteFilePath()) {
        m_cutItem.clear();
        m_cutSourceFolder.clear();
        return;
    }

    if (!QFile::copy(srcPath, newPath)) {
        QMessageBox::warning(this, "Paste Failed", "Could not copy '" + m_cutItem + "' to the destination folder.");
        return;
    }

    if (!QFile::remove(srcPath)) {
        m_statusBar->showMessage("Pasted, but could not remove the original", 3000);
    }

    if (m_currentFile == srcPath) {
        m_currentFile = newPath;
    }

    m_cutItem.clear();
    m_cutSourceFolder.clear();
    refreshFileList();
}

bool EditorView::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && obj == m_fileList) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QListWidgetItem *item = m_fileList->currentItem();
        
        if (!item) return QWidget::eventFilter(obj, event);
        
        if (keyEvent->modifiers() & Qt::ControlModifier) {
            if (keyEvent->key() == Qt::Key_C) { // Copy
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(item->text());
                return true;
            }
            if (keyEvent->key() == Qt::Key_X) { // Cut
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(item->text());
                m_cutItem = item->text();
                m_cutSourceFolder = m_folderPath;
                return true;
            }
            if (keyEvent->key() == Qt::Key_V) { // Paste
                pasteCutItem();
                return true;
            }
        }
        
        if (keyEvent->key() == Qt::Key_Delete) { // Delete
            QString filePath = m_folderPath + "/" + item->text() + ".md";
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Note",
                "Delete '" + item->text() + "'?", QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                if (QFile::remove(filePath)) {
                    refreshFileList();
                }
            }
            return true;
        }
        
        if (keyEvent->key() == Qt::Key_F2) { // Rename
            bool ok;
            QString newName = QInputDialog::getText(this, "Rename Note", "Enter new name:", QLineEdit::Normal, item->text(), &ok);
            if (ok && !newName.isEmpty() && newName != item->text()) {
                if (newName.endsWith(".md")) newName.chop(3);
                QString oldPath = m_folderPath + "/" + item->text() + ".md";
                QString newPath = m_folderPath + "/" + newName + ".md";
                if (QFile::rename(oldPath, newPath)) {
                    refreshFileList();
                }
            }
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
}
