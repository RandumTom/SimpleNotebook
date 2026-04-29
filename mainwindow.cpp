#include "mainwindow.h"
#include "startpage.h"
#include "editorview.h"
#include <QStatusBar>
#include <QMenuBar>
#include <QLayout>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("SimpleNotebook");
    setWindowIcon(QIcon(":/Title_Image.png"));
    if (windowIcon().isNull()) {
        setWindowIcon(QIcon("Title_Image.png"));
    }
    setStyleSheet("QMainWindow { background-color: #1E1E1E; border: none; }");

    // Hide bars BEFORE creating widgets
    statusBar()->hide();
    menuBar()->hide();
    
    // Remove all margins
    setContentsMargins(0, 0, 0, 0);
    
    // Create pages
    m_startPage = new StartPage(this);
    m_editorView = new EditorView(this);
    
    // Start with start page
    setCentralWidget(m_startPage);
    
    // Force resize to fill window
    QTimer::singleShot(0, this, [this]() {
        m_startPage->resize(size());
    });

    // Connections
    connect(m_startPage, &StartPage::folderSelected, this, &MainWindow::onFolderSelected);
    
    // Start maximized
    showMaximized();
}

MainWindow::~MainWindow()
{
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // Ensure central widget fills the window
    if (centralWidget()) {
        centralWidget()->resize(event->size());
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::onFolderSelected(const QString &folderPath)
{
    // Add docks before setting central widget
    if (m_editorView->terminalDock()) {
        addDockWidget(Qt::BottomDockWidgetArea, m_editorView->terminalDock());
    }
    
    m_editorView->setFolder(folderPath);
    setCentralWidget(m_editorView);
    
    // Add Git panel on the RIGHT side
    addDockWidget(Qt::RightDockWidgetArea, m_editorView->gitDock());
    
    // Show status bar for editor
    statusBar()->show();
}
