#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class StartPage;
class EditorView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFolderSelected(const QString &folderPath);

private:
    StartPage *m_startPage;
    EditorView *m_editorView;
};

#endif // MAINWINDOW_H
