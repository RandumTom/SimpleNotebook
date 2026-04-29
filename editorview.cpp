#include "editorview.h"

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
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QPushButton>

EditorView::EditorView(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left sidebar
    QWidget *sidebar = new QWidget(this);
    sidebar->setMaximumWidth(220);
    sidebar->setMinimumWidth(150);
    sidebar->setStyleSheet("background-color: #F5F5F5;");
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(8, 8, 8, 8);
    sidebarLayout->setSpacing(8);

    // Back button
    QPushButton *backBtn = new QPushButton("← Back", sidebar);
    backBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: 1px solid #CCC;"
        "   border-radius: 4px;"
        "   padding: 6px 12px;"
        "   text-align: left;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E0E0E0;"
        "}"
    );
    sidebarLayout->addWidget(backBtn);

    // Folder label
    m_folderLabel = new QLabel(sidebar);
    m_folderLabel->setStyleSheet("font-size: 11px; color: #666;");
    m_folderLabel->setWordWrap(true);
    sidebarLayout->addWidget(m_folderLabel);

    // File list
    m_fileList = new QListWidget(sidebar);
    m_fileList->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #DDD;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QListWidget::item {"
        "   padding: 6px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #4A90D9;"
        "   color: white;"
        "}"
    );
    sidebarLayout->addWidget(m_fileList, 1);

    // Add button
    QPushButton *addBtn = new QPushButton("+ New Note", sidebar);
    addBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45A049;"
        "}"
    );
    sidebarLayout->addWidget(addBtn);

    mainLayout->addWidget(sidebar);

    // Right side - editor
    QWidget *editorArea = new QWidget(this);
    QVBoxLayout *editorLayout = new QVBoxLayout(editorArea);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    // Toolbar
    m_toolbar = new QToolBar(editorArea);
    m_toolbar->setMovable(false);

    QAction *newAct = new QAction("New", this);
    newAct->setShortcut(QKeySequence::New);
    m_toolbar->addAction(newAct);

    QAction *saveAct = new QAction("Save", this);
    saveAct->setShortcut(QKeySequence::Save);
    m_toolbar->addAction(saveAct);

    editorLayout->addWidget(m_toolbar);

    // Text editor
    m_textEdit = new QPlainTextEdit(editorArea);
    m_textEdit->setFont(QFont("monospace", 12));
    m_textEdit->setStyleSheet(
        "QPlainTextEdit {"
        "   border: none;"
        "   padding: 10px;"
        "}"
    );
    editorLayout->addWidget(m_textEdit, 1);

    // Status bar
    m_statusBar = new QStatusBar(editorArea);
    editorLayout->addWidget(m_statusBar);

    mainLayout->addWidget(editorArea, 1);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &EditorView::backRequested);
    connect(addBtn, &QPushButton::clicked, this, &EditorView::onAddClicked);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &EditorView::onFileSelected);
    connect(newAct, &QAction::triggered, this, &EditorView::onNew);
    connect(saveAct, &QAction::triggered, this, &EditorView::onSave);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &EditorView::onTextChanged);

    updateStatusBar();
}

EditorView::~EditorView()
{
}

void EditorView::setFolder(const QString &folderPath)
{
    m_folderPath = folderPath;
    m_folderLabel->setText(QFileInfo(folderPath).fileName());
    refreshFileList();
    m_textEdit->clear();
    m_currentFile.clear();
    m_isUnsaved = false;
    updateStatusBar();
}

void EditorView::refreshFileList()
{
    m_fileList->clear();
    QDir dir(m_folderPath);
    QStringList filters;
    filters << "*.txt";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QFileInfo &file : files) {
        QString name = file.baseName();  // without .txt extension
        m_fileList->addItem(name);
    }

    if (files.isEmpty()) {
        m_fileList->addItem("(No notes yet)");
    }
}

void EditorView::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // Save current if unsaved
    if (m_isUnsaved && !m_currentFile.isEmpty()) {
        saveCurrentFile();
    }

    QTextStream in(&file);
    m_textEdit->setPlainText(in.readAll());
    file.close();

    m_currentFile = filePath;
    m_isUnsaved = false;
    updateStatusBar();
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

        // Create and load new file
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

    saveCurrentFile();
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

void EditorView::updateStatusBar()
{
    QString fileName = m_currentFile.isEmpty() 
        ? "Untitled" 
        : QFileInfo(m_currentFile).baseName();
    
    int charCount = m_textEdit->toPlainText().length();
    
    QString status = QString("  %1%2  |  %3 characters")
        .arg(fileName)
        .arg(m_isUnsaved ? " *" : "")
        .arg(charCount);
    
    m_statusBar->showMessage(status);
}
