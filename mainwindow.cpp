#include "mainwindow.h"

#include "startpage.h"
#include "editorview.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("SimpleNotebook");
    setMinimumSize(600, 400);
    resize(900, 650);

    // Create stacked widget
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // Create pages
    m_startPage = new StartPage(this);
    m_editorView = new EditorView(this);

    m_stackedWidget->addWidget(m_startPage);  // index 0
    m_stackedWidget->addWidget(m_editorView); // index 1

    // Connections
    connect(m_startPage, &StartPage::folderSelected, this, &MainWindow::onFolderSelected);
    connect(m_editorView, &EditorView::backRequested, [this]() {
        m_stackedWidget->setCurrentIndex(0);
    });

    // Show start page
    m_stackedWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onFolderSelected(const QString &folderPath)
{
    m_editorView->setFolder(folderPath);
    m_stackedWidget->setCurrentIndex(1);
}
