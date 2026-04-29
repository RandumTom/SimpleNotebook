#ifndef GITWINDOW_H
#define GITWINDOW_H

#include <QWidget>
#include <QString>
#include <QProcess>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class GitWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GitWindow(const QString &repoPath, QWidget *parent = nullptr);
    ~GitWindow();

private slots:
    void onRefresh();
    void onCommit();
    void onPush();
    void onPull();
    void onStatus();
    void onBranch();
    void onCheckout();
    void onLog();
    void onGitFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadOutput();
    void onReadError();

private:
    void runGit(const QString &args, const QString &description);
    void refreshLog();
    void refreshBranches();
    
    QString m_repoPath;
    QProcess *m_gitProcess;
    
    QTextEdit *m_outputEdit;
    QLineEdit *m_commitMsgInput;
    QListWidget *m_branchList;
    QListWidget *m_logList;
    QLabel *m_statusLabel;
};

#endif // GITWINDOW_H
