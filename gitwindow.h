#ifndef GITWINDOW_H
#define GITWINDOW_H

#include <QWidget>
#include <QString>
#include <QProcess>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

class GitWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GitWindow(const QString &repoPath, QWidget *parent = nullptr);
    ~GitWindow();

private slots:
    void onRefresh();
    void onStageAll();
    void onCommit();
    void onPush();
    void onPull();
    void onFetch();
    void onGitFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReadOutput();
    void onReadError();
    void autoRefresh();

private:
    void runGit(const QStringList &args, const QString &description);
    void refreshStatus();
    void refreshBranches();
    void refreshRecentCommits();
    
    QString m_repoPath;
    QProcess *m_gitProcess;
    
    QLabel *m_headerLabel;
    QLabel *m_branchLabel;
    QPushButton *m_fetchBtn;
    
    QLabel *m_changesHeader;
    QPushButton *m_stageAllBtn;
    QListWidget *m_changesList;
    QLabel *m_noChangesLabel;
    
    QPlainTextEdit *m_commitInput;
    QPushButton *m_commitBtn;
    QPushButton *m_undoCommitBtn;
    
    QListWidget *m_recentCommits;
    QLabel *m_statusBar;
    
    QTimer *m_autoRefresh;
};

#endif // GITWINDOW_H
