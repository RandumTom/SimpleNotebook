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
#include <QPlainTextEdit>
#include <QLabel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QScrollBar>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextCursor>
#include <QColor>

EditorView::EditorView(QWidget *parent)
    : QWidget(parent)
    , m_autoSaveTimer(new QTimer(this))
{
    // Dark mode stylesheet
    setStyleSheet(
        "QWidget {"
        "   background-color: #1E1E1E;"
        "   color: #D4D4D4;"
        "}"
        "QToolBar {"
        "   background-color: #252526;"
        "   border: none;"
        "   spacing: 5px;"
        "   padding: 5px;"
        "}"
        "QStatusBar {"
        "   background-color: #007ACC;"
        "   color: #FFFFFF;"
        "   border: none;"
        "}"
        "QListWidget {"
        "   background-color: #1E1E1E;"
        "   color: #D4D4D4;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 6px;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   padding: 10px 8px;"
        "   border-bottom: 1px solid #2D2D2D;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #3730A3;"
        "   color: #FFFFFF;"
        "}"
        "QListWidget::item:hover:!selected {"
        "   background-color: #2D2D2D;"
        "}"
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #D4D4D4;"
        "   border: none;"
        "   padding: 8px 12px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3E3E42;"
        "}"
    );

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ============= LEFT SIDEBAR =============
    m_sidebar = new QWidget(this);
    m_sidebar->setMaximumWidth(220);
    m_sidebar->setMinimumWidth(160);
    m_sidebar->setStyleSheet("background-color: #252526;");
    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    sidebarLayout->setSpacing(10);

    // Back button
    QPushButton *backBtn = new QPushButton("  ←  Back to Subjects", m_sidebar);
    backBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #D4D4D4;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 6px;"
        "   padding: 10px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3E3E42;"
        "   border-color: #7C3AED;"
        "}"
    );
    sidebarLayout->addWidget(backBtn);

    // Folder label
    m_folderLabel = new QLabel(m_sidebar);
    m_folderLabel->setStyleSheet(
        "font-size: 11px; "
        "color: #808080;"
        "padding: 5px 0;"
    );
    m_folderLabel->setWordWrap(true);
    sidebarLayout->addWidget(m_folderLabel);

    // File list label
    QLabel *filesLabel = new QLabel("  📝 Notes", m_sidebar);
    filesLabel->setStyleSheet(
        "font-size: 12px; "
        "font-weight: bold; "
        "color: #808080;"
        "padding: 10px 0 5px 0;"
    );
    sidebarLayout->addWidget(filesLabel);

    // File list
    m_fileList = new QListWidget(m_sidebar);
    m_fileList->setFont(QFont("Segoe UI", 11));
    sidebarLayout->addWidget(m_fileList, 1);

    // Add button
    QPushButton *addBtn = new QPushButton("  +  New Note", m_sidebar);
    addBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #10B981;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 12px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #059669;"
        "}"
    );
    sidebarLayout->addWidget(addBtn);

    mainLayout->addWidget(m_sidebar);

    // ============= RIGHT SIDE - EDITOR =============
    QWidget *editorArea = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorArea);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    // Toolbar
    m_toolbar = new QToolBar(editorArea);
    m_toolbar->setMovable(false);

    QAction *newAct = new QAction("New", this);
    newAct->setShortcut(QKeySequence::New);
    newAct->setIconText("New");

    QAction *saveAct = new QAction("Save", this);
    saveAct->setShortcut(QKeySequence::Save);

    QAction *searchAct = new QAction("Search", this);
    searchAct->setShortcut(QKeySequence::Find);
    searchAct->setCheckable(true);

    m_toolbar->addAction(newAct);
    m_toolbar->addAction(saveAct);
    m_toolbar->addSeparator();
    m_toolbar->addAction(searchAct);

    editorLayout->addWidget(m_toolbar);

    // Search bar (hidden by default)
    m_searchBar = new QWidget(editorArea);
    m_searchBar->setStyleSheet(
        "background-color: #2D2D30;"
        "border-bottom: 1px solid #3E3E42;"
        "padding: 8px;"
    );
    m_searchBar->setVisible(false);

    QHBoxLayout *searchLayout = new QHBoxLayout(m_searchBar);
    searchLayout->setContentsMargins(10, 5, 10, 5);
    searchLayout->setSpacing(10);

    QLabel *searchIcon = new QLabel("🔍", m_searchBar);
    searchIcon->setStyleSheet("font-size: 14px;");
    searchLayout->addWidget(searchIcon);

    m_searchInput = new QLineEdit(m_searchBar);
    m_searchInput->setPlaceholderText("Search in document...");
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "   background-color: #1E1E1E;"
        "   color: #D4D4D4;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "   padding: 6px 10px;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #7C3AED;"
        "}"
    );
    m_searchInput->setMinimumWidth(250);
    searchLayout->addWidget(m_searchInput);

    m_searchLabel = new QLabel("", m_searchBar);
    m_searchLabel->setStyleSheet("color: #808080; font-size: 12px;");
    searchLayout->addWidget(m_searchLabel);

    searchLayout->addStretch();

    m_searchPrevBtn = new QPushButton("▲", m_searchBar);
    m_searchPrevBtn->setFixedSize(30, 30);
    m_searchPrevBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #3E3E42; }"
    );
    searchLayout->addWidget(m_searchPrevBtn);

    m_searchNextBtn = new QPushButton("▼", m_searchBar);
    m_searchNextBtn->setFixedSize(30, 30);
    m_searchNextBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: 1px solid #3E3E42;"
        "   border-radius: 4px;"
        "}"
        "QPushButton:hover { background-color: #3E3E42; }"
    );
    searchLayout->addWidget(m_searchNextBtn);

    m_searchCloseBtn = new QPushButton("✕", m_searchBar);
    m_searchCloseBtn->setFixedSize(30, 30);
    m_searchCloseBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #808080;"
        "   border: none;"
        "}"
        "QPushButton:hover { color: #D4D4D4; background-color: #EF4444; }"
    );
    searchLayout->addWidget(m_searchCloseBtn);

    editorLayout->addWidget(m_searchBar);

    // Text editor with line numbers
    QWidget *editorContainer = new QWidget(editorArea);
    QHBoxLayout *editorContainerLayout = new QHBoxLayout(editorContainer);
    editorContainerLayout->setContentsMargins(0, 0, 0, 0);
    editorContainerLayout->setSpacing(0);

    // Line numbers area (we'll use a custom approach with the text edit)
    m_textEdit = new QPlainTextEdit(editorContainer);
    m_textEdit->setFont(QFont("Consolas", 14));
    m_textEdit->setStyleSheet(
        "QPlainTextEdit {"
        "   background-color: #1E1E1E;"
        "   color: #D4D4D4;"
        "   border: none;"
        "   padding: 10px;"
        "   padding-left: 50px;"
        "   selection-background-color: #3730A3;"
        "   selection-color: #FFFFFF;"
        "}"
        "QPlainTextEdit:focus {"
        "   outline: none;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: #1E1E1E;"
        "   width: 12px;"
        "   border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #424242;"
        "   border-radius: 6px;"
        "   min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #606060;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "   background-color: #1E1E1E;"
        "   height: 12px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "   background-color: #424242;"
        "   border-radius: 6px;"
        "   min-width: 30px;"
        "}"
    );
    
    // Enable word wrap
    m_textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    editorContainerLayout->addWidget(m_textEdit);

    editorLayout->addWidget(editorContainer, 1);

    // Status bar
    m_statusBar = new QStatusBar(editorArea);
    editorLayout->addWidget(m_statusBar);

    mainLayout->addWidget(editorArea, 1);

    // ============= CONNECTIONS =============
    connect(backBtn, &QPushButton::clicked, this, &EditorView::backRequested);
    connect(addBtn, &QPushButton::clicked, this, &EditorView::onAddClicked);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &EditorView::onFileSelected);
    connect(newAct, &QAction::triggered, this, &EditorView::onNew);
    connect(saveAct, &QAction::triggered, this, &EditorView::onSave);
    connect(searchAct, &QAction::triggered, this, &EditorView::onSearchToggle);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextChanged);
    connect(m_textEdit, &QPlainTextEdit::cursorPositionChanged, this, &EditorView::onCursorPositionChanged);
    
    // Search connections
    connect(m_searchInput, &QLineEdit::textChanged, this, &EditorView::onSearchTextChanged);
    connect(m_searchNextBtn, &QPushButton::clicked, this, &EditorView::onSearchNext);
    connect(m_searchPrevBtn, &QPushButton::clicked, this, &EditorView::onSearchPrevious);
    connect(m_searchCloseBtn, &QPushButton::clicked, this, &EditorView::onSearchToggle);
    
    // Auto-save timer
    connect(m_autoSaveTimer, &QTimer::timeout, this, &EditorView::autoSave);
    m_autoSaveTimer->start(m_autoSaveInterval);

    updateStatusBar();
}

EditorView::~EditorView()
{
    m_autoSaveTimer->stop();
}

// ============= PUBLIC METHODS =============

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
    // Save current if unsaved
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

// ============= PRIVATE SLOTS =============

void EditorView::onNew()
{
    // Save current if unsaved
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
    QString baseName = QInputDialog::getText(
        this, "New Note", "Enter note name:", QLineEdit::Normal, "untitled", &ok
    );

    if (ok && !baseName.isEmpty()) {
        // Remove .txt if user included it
        if (baseName.endsWith(".txt"))
            baseName.chop(4);

        QString filePath = m_folderPath + "/" + baseName + ".txt";

        // Check if exists
        if (QFileInfo::exists(filePath)) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "File Exists",
                "A note with this name already exists. Overwrite?",
                QMessageBox::Yes | QMessageBox::No
            );
            if (reply != QMessageBox::Yes)
                return;
        }

        // Save current if unsaved
        if (m_isUnsaved && !m_currentFile.isEmpty()) {
            saveCurrentFile();
        }

        // Create new file
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

    QString filePath = m_folderPath + "/" + item->text() + ".txt";
    loadFile(filePath);
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

// ============= SEARCH FUNCTIONS =============

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
        // Clear highlights
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
    if (cursor.isNull()) {
        // Wrap around to beginning
        cursor = doc->find(m_searchInput->text());
    }
    
    if (!cursor.isNull()) {
        m_textEdit->setTextCursor(cursor);
    }
}

void EditorView::onSearchPrevious()
{
    QTextDocument *doc = m_textEdit->document();
    QTextCursor cursor = m_textEdit->textCursor();
    
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    cursor = doc->find(m_searchInput->text(), cursor, flags);
    
    if (cursor.isNull()) {
        // Wrap around to end
        cursor = doc->find(m_searchInput->text(), QTextCursor(doc), flags);
    }
    
    if (!cursor.isNull()) {
        m_textEdit->setTextCursor(cursor);
    }
}

void EditorView::highlightAllMatches(const QString &searchText)
{
    QList<QTextEdit::ExtraSelection> highlights;
    
    if (!searchText.isEmpty()) {
        QTextDocument *doc = m_textEdit->document();
        QTextCursor cursor(doc);
        int matchCount = 0;
        
        cursor.beginEditBlock();
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
        cursor.endEditBlock();
        
        m_searchLabel->setText(QString("%1 match%2").arg(matchCount).arg(matchCount == 1 ? "" : "es"));
    } else {
        m_searchLabel->setText("");
    }
    
    m_textEdit->setExtraSelections(highlights);
}

// ============= HELPER FUNCTIONS =============

void EditorView::refreshFileList()
{
    m_fileList->clear();
    QDir dir(m_folderPath);
    QStringList filters;
    filters << "*.txt";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QFileInfo &file : files) {
        QString name = file.baseName();
        m_fileList->addItem(name);
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
    QString fileName = m_currentFile.isEmpty() 
        ? "Untitled" 
        : QFileInfo(m_currentFile).baseName();
    
    int charCount = m_textEdit->toPlainText().length();
    QString text = m_textEdit->toPlainText();
    int wordCount = text.isEmpty() ? 0 : text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    
    QString unsaved = m_isUnsaved ? " ●" : "";
    
    m_statusBar->showMessage(
        QString("  %1%2  |  Words: %3  |  Characters: %4  |  Line: %5  Col: %6")
            .arg(fileName)
            .arg(unsaved)
            .arg(wordCount)
            .arg(charCount)
            .arg(m_textEdit->textCursor().blockNumber() + 1)
            .arg(m_textEdit->textCursor().columnNumber() + 1)
    );
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
        
        QColor lineColor = QColor("#2D2D30");
        selection.format.setBackground(lineColor);
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
    
    // Ctrl+Backspace to delete entire line
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Backspace) {
        deleteCurrentLine();
        event->accept();
        return;
    }
    
    // Ctrl+Delete to delete entire line (alternative)
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Delete) {
        deleteCurrentLine();
        event->accept();
        return;
    }
    
    // Alt+Backspace to delete entire line
    if (event->modifiers() & Qt::AltModifier && event->key() == Qt::Key_Backspace) {
        deleteCurrentLine();
        event->accept();
        return;
    }
    
    // Alt+Delete to delete entire line
    if (event->modifiers() & Qt::AltModifier && event->key() == Qt::Key_Delete) {
        deleteCurrentLine();
        event->accept();
        return;
    }
    
    // Space triggers math conversion
    if (event->key() == Qt::Key_Space && !(event->modifiers() & ~Qt::ShiftModifier)) {
        if (convertMathInput()) {
            event->accept();
            return;
        }
    }
    
    QWidget::keyPressEvent(event);
}

bool EditorView::convertMathInput()
{
    QTextCursor cursor = m_textEdit->textCursor();
    QString text = m_textEdit->toPlainText();
    int cursorPos = cursor.position();
    
    if (cursorPos == 0)
        return false;
    
    // Find the word before cursor (word boundary to word boundary)
    int start = cursorPos - 1;
    while (start > 0 && !text[start - 1].isSpace() && text[start - 1] != '(' && text[start - 1] != ')' && text[start - 1] != ',') {
        start--;
    }
    
    // Don't process if there are brackets involved
    QString word = text.mid(start, cursorPos - start);
    
    // Try to get a complete word (from last space/bracket to cursor)
    int wordStart = cursorPos;
    while (wordStart > 0) {
        wordStart--;
        QChar c = text[wordStart];
        if (c.isSpace() || c == '(' || c == ')' || c == ',' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            wordStart++;
            break;
        }
    }
    if (wordStart == cursorPos)
        return false;
    
    word = text.mid(wordStart, cursorPos - wordStart);
    if (word.isEmpty())
        return false;
    
    // Also include brackets for patterns like "root("
    int charsToDelete = 0;
    QString converted = MathConverter::convert(word, charsToDelete);
    
    if (!converted.isEmpty() && charsToDelete > 0) {
        // Delete the word and replace with converted
        QTextCursor deleteCursor(m_textEdit->document());
        deleteCursor.setPosition(wordStart);
        deleteCursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
        deleteCursor.insertText(converted);
        
        // Move cursor to end of inserted text
        QTextCursor newCursor(m_textEdit->document());
        newCursor.setPosition(wordStart + converted.length());
        m_textEdit->setTextCursor(newCursor);
        
        onTextChanged();
        return true;
    }
    
    return false;
}

void EditorView::deleteCurrentLine()
{
    QTextCursor cursor = m_textEdit->textCursor();
    
    // Select the entire line
    cursor.select(QTextCursor::LineUnderCursor);
    
    // Also select the newline character at the end (unless at end of document)
    cursor.movePosition(QTextCursor::EndOfLine);
    cursor.insertText("");
    
    // Re-select and delete
    cursor.select(QTextCursor::LineUnderCursor);
    
    // If there's a newline after, include it
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    QString selected = cursor.selectedText();
    if (selected.isEmpty() || selected == "\n") {
        // Already at end, just delete the line
        cursor.select(QTextCursor::LineUnderCursor);
    } else if (!selected.contains(QChar(QChar::CarriageReturn)) && !selected.contains('\n')) {
        // No newline found, extend selection
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    }
    
    // Remove selection but don't delete yet - instead rebuild
    QTextCursor tempCursor = m_textEdit->textCursor();
    tempCursor.select(QTextCursor::LineUnderCursor);
    
    // Get text before and after the line
    QTextCursor startCursor = tempCursor;
    startCursor.movePosition(QTextCursor::StartOfLine);
    startCursor.setPosition(tempCursor.position(), QTextCursor::KeepAnchor);
    
    // Get content before this line
    QTextCursor beforeCursor(startCursor);
    beforeCursor.setPosition(0);
    beforeCursor.setPosition(startCursor.position(), QTextCursor::KeepAnchor);
    QString beforeText = beforeCursor.selectedText();
    
    // Get content after this line (including next line's content)
    QTextCursor afterCursor(tempCursor);
    afterCursor.movePosition(QTextCursor::EndOfLine);
    afterCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
    afterCursor.setPosition(tempCursor.position(), QTextCursor::MoveAnchor);
    afterCursor.setPosition(m_textEdit->document()->characterCount(), QTextCursor::KeepAnchor);
    QString afterText = afterCursor.selectedText();
    
    // Replace entire content
    m_textEdit->setPlainText(beforeText + afterText);
    
    // Position cursor at end of what was before
    QTextCursor newCursor = m_textEdit->textCursor();
    newCursor.setPosition(beforeText.length());
    m_textEdit->setTextCursor(newCursor);
    
    onTextChanged();
}
