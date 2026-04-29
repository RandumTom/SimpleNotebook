#include "editorview.h"
#include "MathConverter.h"

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
#include <QClipboard>

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

    QAction *terminalAct = new QAction("Terminal (Ctrl+Ö)", this);
    terminalAct->setCheckable(true);
    terminalAct->setShortcut(Qt::CTRL | Qt::Key_O);

    QAction *calcAct = new QAction("Calculator", this);
    calcAct->setCheckable(true);

    QAction *ghosttyAct = new QAction("Ghostty", this);

    m_toolbar->addAction(newAct);
    m_toolbar->addAction(saveAct);
    m_toolbar->addSeparator();
    m_toolbar->addAction(convertAct);
    m_toolbar->addAction(searchAct);
    m_toolbar->addSeparator();
    m_toolbar->addAction(calcAct);
    m_toolbar->addAction(terminalAct);
    m_toolbar->addAction(ghosttyAct);

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

    // Terminal Dock (hidden by default)
    m_terminalDock = new QDockWidget("Terminal", this);
    m_terminalDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_terminalDock->setMaximumHeight(200);
    m_terminalDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_terminalDock->setStyleSheet("QDockWidget { background-color: #0D0D0D; color: #00FF00; }");

    QWidget *terminalWidget = new QWidget(m_terminalDock);
    QVBoxLayout *terminalLayout = new QVBoxLayout(terminalWidget);
    terminalLayout->setContentsMargins(5, 5, 5, 5);

    m_terminalOutput = new QTextEdit(terminalWidget);
    m_terminalOutput->setFont(QFont("Consolas", 11));
    m_terminalOutput->setStyleSheet("background-color: #0D0D0D; color: #00FF00; border: none;");
    m_terminalOutput->setReadOnly(true);
    terminalLayout->addWidget(m_terminalOutput, 1);

    QHBoxLayout *terminalInputLayout = new QHBoxLayout();
    terminalInputLayout->addWidget(new QLabel("$ ", terminalWidget));
    m_terminalInput = new QLineEdit(terminalWidget);
    m_terminalInput->setStyleSheet("background-color: #1A1A1A; color: #00FF00; border: 1px solid #333; padding: 4px;");
    terminalInputLayout->addWidget(m_terminalInput, 1);
    terminalLayout->addLayout(terminalInputLayout);

    m_terminalDock->setWidget(terminalWidget);
    m_terminalDock->hide();

    // Calculator Dock (hidden by default)
    m_calcDock = new QDockWidget("Calculator", this);
    m_calcDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_calcDock->setMaximumHeight(250);
    m_calcDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);

    QWidget *calcWidget = new QWidget(m_calcDock);
    QVBoxLayout *calcLayout = new QVBoxLayout(calcWidget);
    calcLayout->setContentsMargins(10, 10, 10, 10);

    m_calcDisplay = new QLineEdit(calcWidget);
    m_calcDisplay->setFont(QFont("Consolas", 18));
    m_calcDisplay->setStyleSheet("background-color: #1A1A1A; color: #00FF00; border: 2px solid #333; border-radius: 5px; padding: 10px; text-align: right;");
    m_calcDisplay->setReadOnly(true);
    m_calcDisplay->setPlaceholderText("0");
    calcLayout->addWidget(m_calcDisplay);

    QGridLayout *calcButtons = new QGridLayout();
    QStringList buttons = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+"};
    QStringList styles = {
        "QPushButton { background-color: #333; color: white; border: none; border-radius: 5px; padding: 15px; font-size: 16px; } QPushButton:hover { background-color: #444; }",
        "QPushButton { background-color: #7C3AED; color: white; border: none; border-radius: 5px; padding: 15px; font-size: 16px; } QPushButton:hover { background-color: #8B5CF6; }",
        "QPushButton { background-color: #EF4444; color: white; border: none; border-radius: 5px; padding: 15px; font-size: 16px; } QPushButton:hover { background-color: #DC2626; }"
    };

    for (int i = 0; i < buttons.size(); i++) {
        QPushButton *btn = new QPushButton(buttons[i], calcWidget);
        btn->setFont(QFont("Consolas", 16));
        
        if (buttons[i] == "=") {
            btn->setStyleSheet(styles[1]);
        } else if (buttons[i] == "C") {
            btn->setStyleSheet(styles[2]);
        } else if (buttons[i] == "/" || buttons[i] == "*" || buttons[i] == "-" || buttons[i] == "+") {
            btn->setStyleSheet("QPushButton { background-color: #4A90D9; color: white; border: none; border-radius: 5px; padding: 15px; font-size: 16px; } QPushButton:hover { background-color: #5BA0E9; }");
        } else {
            btn->setStyleSheet(styles[0]);
        }
        
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            QString text = btn->text();
            if (text == "=") {
                onCalcEquals();
            } else if (text == "C") {
                onCalcClear();
            } else {
                m_calcCurrent += text;
                m_calcDisplay->setText(m_calcCurrent);
            }
        });
        
        int row = i / 4;
        int col = i % 4;
        if (buttons[i] == "=") {
            calcButtons->addWidget(btn, row, 0, 1, 4);
        } else {
            calcButtons->addWidget(btn, row, col);
        }
    }
    calcLayout->addLayout(calcButtons);

    m_calcDock->setWidget(calcWidget);
    m_calcDock->hide();

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
    connect(calcAct, &QAction::toggled, [this](bool checked) {
        m_calcDock->setVisible(checked);
    });
    connect(ghosttyAct, &QAction::triggered, this, &EditorView::openSystemTerminal);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextChanged);
    connect(m_textEdit, &QPlainTextEdit::cursorPositionChanged, this, &EditorView::onCursorPositionChanged);
    connect(m_searchInput, &QLineEdit::textChanged, this, &EditorView::onSearchTextChanged);
    connect(m_searchNextBtn, &QPushButton::clicked, this, &EditorView::onSearchNext);
    connect(m_searchPrevBtn, &QPushButton::clicked, this, &EditorView::onSearchPrevious);
    connect(m_searchCloseBtn, &QPushButton::clicked, this, &EditorView::onSearchToggle);
    connect(m_terminalInput, &QLineEdit::returnPressed, this, &EditorView::onTerminalCommand);
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
}

void EditorView::loadFile(const QString &filePath)
{
    if (m_isUnsaved && !m_currentFile.isEmpty()) {
        saveCurrentFile();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
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
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
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

void EditorView::onCalcEquals()
{
    if (m_calcCurrent.isEmpty()) return;
    
    // Evaluate the expression safely
    QString expr = m_calcCurrent;
    expr.replace("×", "*").replace("÷", "/");
    
    // Simple eval using QScript (or manual parsing)
    bool ok;
    double result = 0;
    
    // Very simple parser for +, -, *, /
    QStringList tokens;
    QString currentNum;
    QString lastOp = "+";
    
    for (int i = 0; i < expr.length(); i++) {
        QChar c = expr[i];
        if (c.isDigit() || c == '.') {
            currentNum += c;
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            if (!currentNum.isEmpty()) {
                double num = currentNum.toDouble();
                if (lastOp == "+") result += num;
                else if (lastOp == "-") result -= num;
                else if (lastOp == "*") result *= num;
                else if (lastOp == "/") result /= num;
                currentNum.clear();
            }
            lastOp = c;
        }
    }
    
    // Process last number
    if (!currentNum.isEmpty()) {
        double num = currentNum.toDouble();
        if (lastOp == "+") result += num;
        else if (lastOp == "-") result -= num;
        else if (lastOp == "*") result *= num;
        else if (lastOp == "/") result /= num;
    }
    
    // Format result
    QString resultStr = QString::number(result);
    if (resultStr.endsWith(".0")) {
        resultStr.chop(2);
    }
    
    m_calcDisplay->setText(resultStr);
    m_calcCurrent = resultStr;
    
    // Copy to clipboard and insert into editor
    QApplication::clipboard()->setText(resultStr);
    m_textEdit->insertPlainText(resultStr);
}

void EditorView::onCalcClear()
{
    m_calcCurrent.clear();
    m_calcDisplay->setText("0");
}

void EditorView::onTerminalCommand()
{
    QString command = m_terminalInput->text().trimmed();
    if (command.isEmpty()) return;
    
    m_terminalOutput->append("$ " + command);
    m_terminalInput->clear();
    
    // Execute command
    QProcess *process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        m_terminalOutput->append(QString::fromLocal8Bit(process->readAllStandardOutput()));
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        m_terminalOutput->append("<span style='color: #FF6B6B;'>" + QString::fromLocal8Bit(process->readAllStandardError()) + "</span>");
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int, QProcess::ExitStatus) {
        m_terminalOutput->append("");
    });
    
    if (!m_folderPath.isEmpty()) {
        process->setWorkingDirectory(m_folderPath);
    }
    process->start("/bin/sh", QStringList() << "-c" << command);
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
        while (!cursor.isNull() && !cursor.atEnd()) {
            cursor = doc->find(searchText, cursor);
            if (!cursor.isNull()) {
                QTextEdit::ExtraSelection selection;
                selection.format.setBackground(QColor("#4B4B00"));
                selection.format.setForeground(QColor("#FFFFFF"));
                selection.cursor = cursor;
                selection.cursor.clearSelection();
                highlights.append(selection);
                matchCount++;
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            }
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
    
    // Ctrl+Ö (Ctrl+O) for terminal toggle - German keyboard
    if (event->modifiers() & Qt::ControlModifier && (event->key() == Qt::Key_O || event->key() == Qt::Key_Odiaeresis)) {
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
    QString converted = MathConverter::convert(word, charsToDelete);
    
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
    QProcess *process = new QProcess(this);
    
    if (!m_folderPath.isEmpty()) {
        process->setWorkingDirectory(m_folderPath);
    }
    
    process->start("ghostty");
}
