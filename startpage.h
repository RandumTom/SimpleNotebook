#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>
#include <QString>

class QPushButton;
class QVBoxLayout;
class QLabel;

class StartPage : public QWidget
{
    Q_OBJECT

public:
    explicit StartPage(QWidget *parent = nullptr);
    ~StartPage();

signals:
    void folderSelected(const QString &folderPath);

private slots:
    void onSubjectClicked();
    void onSelectFolder();

private:
    QPushButton *m_selectFolderBtn;
    QLabel *m_folderLabel;
};

#endif // STARTPAGE_H
