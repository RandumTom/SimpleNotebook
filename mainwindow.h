#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QResizeEvent>

class StartPage;
class EditorView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onFolderSelected(const QString &folderPath);

private:
    StartPage *m_startPage;
    EditorView *m_editorView;
};

#endif // MAINWINDOW_H
