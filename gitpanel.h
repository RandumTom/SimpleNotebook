#ifndef GITPANEL_H
#define GITPANEL_H

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
#include <QComboBox>
#include <QMenu>
#include <QDockWidget>

class GitPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit GitPanel(const QString &repoPath, QWidget *parent = nullptr);
    ~GitPanel();
    void refresh();
    void setRepoPath(const QString &repoPath) { m_repoPath = repoPath; refresh(); }

private slots:
    void onFetch();
    void onFetchPrune();
    void onStageAll();
    void onCommit();
    void onCommitAmend();
    void onCommitStaged();
    void onPush();
    void onPull();
    void onUndoCommit();
    void onGitFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void autoRefresh();

private:
    void runGit(const QStringList &args);
    void refreshStatus();
    void refreshBranches();
    void refreshRecentCommits();
    
    QString m_repoPath;
    QProcess *m_gitProcess;
    
    QWidget *m_content;
    QWidget *m_header;
    QComboBox *m_projectDropdown;
    QLabel *m_branchIndicator;
    
    QLabel *m_changesHeader;
    QPushButton *m_optionsBtn;
    QPushButton *m_stageAllBtn;
    QListWidget *m_changesList;
    QLabel *m_noChangesLabel;
    
    QWidget *m_syncSection;
    QLabel *m_pathLabel;
    QPushButton *m_fetchBtn;
    QMenu *m_fetchMenu;
    
    QPlainTextEdit *m_commitInput;
    QPushButton *m_expandBtn;
    QPushButton *m_commitBtn;
    QMenu *m_commitMenu;
    
    QListWidget *m_recentCommits;
    QPushButton *m_undoBtn;
    
    QLabel *m_statusLabel;
    QTimer *m_autoRefresh;
};

#endif // GITPANEL_H
