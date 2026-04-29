#include "mainwindow.h"
#include "startpage.h"
#include "editorview.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("SimpleNotebook");
    setStyleSheet("QMainWindow { background-color: #1E1E1E; }");

    // Hide bars BEFORE creating widgets
    statusBar()->hide();
    menuBar()->hide();
    
    // Remove all margins
    setContentsMargins(0, 0, 0, 0);
    setDockOptions(QMainWindow::DockOption::AnimatedDocks | QMainWindow::DockOption::AllowNestedDocks);

    // Create pages
    m_startPage = new StartPage(this);
    m_editorView = new EditorView(this);
    
    // Start with start page
    setCentralWidget(m_startPage);
    
    // Ensure no margins around central widget
    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);

    // Connections
    connect(m_startPage, &StartPage::folderSelected, this, &MainWindow::onFolderSelected);
    
    // Start maximized
    showMaximized();
}

MainWindow::~MainWindow()
{
}

void MainWindow::onFolderSelected(const QString &folderPath)
{
    // Add docks before setting central widget
    if (m_editorView->terminalDock()) {
        addDockWidget(Qt::BottomDockWidgetArea, m_editorView->terminalDock());
    }
    
    m_editorView->setFolder(folderPath);
    setCentralWidget(m_editorView);
    
    // Show status bar for editor
    statusBar()->show();
}
