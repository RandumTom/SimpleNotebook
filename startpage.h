#ifndef STARTPAGE_H
#define STARTPAGE_H

#include <QWidget>
#include <QString>
#include <QPaintEvent>

class QPushButton;
class QVBoxLayout;
class QLabel;

class StartPage : public QWidget
{
    Q_OBJECT

public:
    explicit StartPage(QWidget *parent = nullptr);
    ~StartPage();

protected:
    void paintEvent(QPaintEvent *event) override;

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
