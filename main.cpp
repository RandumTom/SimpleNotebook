#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SimpleNotebook");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SimpleNotebook");
    
    // Global dark mode styling for native dialogs
    app.setStyle("Fusion");
    app.setPalette(QPalette(QColor(30, 30, 30)));
    
    MainWindow window;
    window.show();

    return app.exec();
}
