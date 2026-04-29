#include "gitpanel.h"
#include <QProcess>
#include <QPushButton>
#include <QGroupBox>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QMenuBar>

GitPanel::GitPanel(const QString &repoPath, QWidget *parent)
    : QDockWidget(tr("Source Control"), parent)
    , m_repoPath(repoPath)
    , m_gitProcess(new QProcess(this))
{
    setWindowTitle(tr("Source Control"));
    setObjectName("GitPanel");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // Dark theme matching Zed
    setStyleSheet(R"(
        QDockWidget {
            background-color: #1E1E1E;
            color: #D4D4D4;
            titlebar-close-icon: url(none);
        }
        QDockWidget::title {
            background-color: #252526;
            text-align: left;
            padding: 8px;
        }
        QWidget {
            background-color: #1E1E1E;
            color: #D4D4D4;
            font-family: "Segoe UI", -apple-system, sans-serif;
            font-size: 13px;
        }
        QLabel {
            color: #D4D4D4;
            background-color: transparent;
        }
        QPushButton {
            background-color: transparent;
            color: #D4D4D4;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
            text-align: left;
        }
        QPushButton:hover {
            background-color: #2D2D2D;
        }
        QPushButton:disabled {
            color: #606060;
        }
        QPushButton.accent {
            background-color: #3730A3;
            color: white;
            text-align: center;
        }
        QPushButton.accent:hover {
            background-color: #4F46E5;
        }
        QComboBox {
            background-color: #2D2D30;
            color: #D4D4D4;
            border: 1px solid #3E3E42;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QComboBox::drop-down {
            border: none;
        }
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            padding: 6px 8px;
            border-radius: 4px;
            color: #D4D4D4;
        }
        QListWidget::item:hover {
            background-color: #2D2D2D;
        }
        QListWidget::item:selected {
            background-color: #3730A3;
        }
        QPlainTextEdit {
            background-color: #2D2D30;
            color: #D4D4D4;
            border: 1px solid #3E3E42;
            border-radius: 6px;
            padding: 8px;
            font-family: "Consolas", monospace;
            font-size: 12px;
        }
        QPlainTextEdit:focus {
            border-color: #7C3AED;
        }
        QMenu {
            background-color: #2D2D30;
            color: #D4D4D4;
            border: 1px solid #3E3E42;
            border-radius: 6px;
        }
        QMenu::item {
            padding: 8px 16px;
        }
        QMenu::item:selected {
            background-color: #3730A3;
        }
    )");

    // Main content widget
    m_content = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_content);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === HEADER ===
    m_header = new QWidget(m_content);
    m_header->setStyleSheet("background-color: #252526; padding: 8px 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(12, 8, 12, 8);
    headerLayout->setSpacing(8);
    
    m_projectDropdown = new QComboBox(m_header);
    m_projectDropdown->addItem("📁 " + QFileInfo(repoPath).fileName());
    m_projectDropdown->setMinimumWidth(140);
    headerLayout->addWidget(m_projectDropdown);
    
    headerLayout->addStretch();
    
    m_branchIndicator = new QLabel("main → main", m_header);
    m_branchIndicator->setStyleSheet("color: #10B981; font-size: 12px; padding: 4px 8px; background-color: #1A1A1A; border-radius: 4px;");
    headerLayout->addWidget(m_branchIndicator);
    
    mainLayout->addWidget(m_header);

    // === CHANGES SECTION ===
    QWidget *changesSection = new QWidget(m_content);
    changesSection->setStyleSheet("padding: 8px 12px;");
    QVBoxLayout *changesLayout = new QVBoxLayout(changesSection);
    changesLayout->setContentsMargins(12, 8, 12, 0);
    changesLayout->setSpacing(8);
    
    // Changes header row
    QHBoxLayout *changesHeaderLayout = new QHBoxLayout();
    changesHeaderLayout->setSpacing(8);
    
    m_changesHeader = new QLabel("Changes", changesSection);
    m_changesHeader->setStyleSheet("font-weight: 600; color: #808080; font-size: 11px; text-transform: uppercase;");
    changesHeaderLayout->addWidget(m_changesHeader);
    
    changesHeaderLayout->addStretch();
    
    m_optionsBtn = new QPushButton("···", changesSection);
    m_optionsBtn->setFixedWidth(28);
    m_optionsBtn->setStyleSheet("QPushButton { font-weight: bold; }");
    changesHeaderLayout->addWidget(m_optionsBtn);
    
    m_stageAllBtn = new QPushButton("Stage All", changesSection);
    m_stageAllBtn->setStyleSheet("color: #10B981; font-size: 12px;");
    changesHeaderLayout->addWidget(m_stageAllBtn);
    
    changesLayout->addLayout(changesHeaderLayout);
    
    // Changes list
    m_changesList = new QListWidget(changesSection);
    m_changesList->setMinimumHeight(100);
    m_changesList->setMaximumHeight(150);
    changesLayout->addWidget(m_changesList);
    
    m_noChangesLabel = new QLabel("✓ No changes to commit", changesSection);
    m_noChangesLabel->setStyleSheet("color: #10B981; padding: 16px; text-align: center;");
    m_noChangesLabel->setAlignment(Qt::AlignCenter);
    changesLayout->addWidget(m_noChangesLabel);
    
    mainLayout->addWidget(changesSection);

    // === SYNC CONTROLS ===
    m_syncSection = new QWidget(m_content);
    m_syncSection->setStyleSheet("padding: 0 12px;");
    QHBoxLayout *syncLayout = new QHBoxLayout(m_syncSection);
    syncLayout->setContentsMargins(12, 0, 12, 0);
    syncLayout->setSpacing(8);
    
    m_pathLabel = new QLabel(QFileInfo(repoPath).fileName() + " / main", m_syncSection);
    m_pathLabel->setStyleSheet("color: #808080; font-size: 12px;");
    syncLayout->addWidget(m_pathLabel);
    
    syncLayout->addStretch();
    
    // Fetch button with dropdown
    m_fetchBtn = new QPushButton("↻ Fetch", m_syncSection);
    m_fetchBtn->setStyleSheet("color: #10B981; font-size: 12px; padding: 4px 8px;");
    syncLayout->addWidget(m_fetchBtn);
    
    m_fetchMenu = new QMenu(m_fetchBtn);
    m_fetchMenu->addAction("Fetch", this, [this]() { runGit({"fetch"}); });
    m_fetchMenu->addAction("Fetch Prune", this, [this]() { runGit({"fetch", "--prune"}); });
    m_fetchMenu->addAction("Fetch All", this, [this]() { runGit({"fetch", "--all"}); });
    m_fetchBtn->setMenu(m_fetchMenu);
    
    mainLayout->addWidget(m_syncSection);

    // === COMMIT INPUT ===
    QWidget *commitSection = new QWidget(m_content);
    commitSection->setStyleSheet("padding: 8px 12px;");
    QVBoxLayout *commitLayout = new QVBoxLayout(commitSection);
    commitLayout->setContentsMargins(12, 0, 12, 0);
    commitLayout->setSpacing(8);
    
    // Commit input with expand button
    QHBoxLayout *commitInputLayout = new QHBoxLayout();
    commitInputLayout->setSpacing(8);
    
    m_commitInput = new QPlainTextEdit(commitSection);
    m_commitInput->setPlaceholderText("Enter commit message...");
    m_commitInput->setMaximumHeight(80);
    commitInputLayout->addWidget(m_commitInput, 1);
    
    m_expandBtn = new QPushButton("⤢", commitSection);
    m_expandBtn->setFixedSize(28, 28);
    m_expandBtn->setStyleSheet("font-size: 14px;");
    commitInputLayout->addWidget(m_expandBtn);
    
    commitLayout->addLayout(commitInputLayout);
    
    // Commit actions row
    QHBoxLayout *commitActionsLayout = new QHBoxLayout();
    commitActionsLayout->setSpacing(8);
    
    commitActionsLayout->addStretch();
    
    // Commit button with dropdown
    m_commitBtn = new QPushButton("Commit", commitSection);
    m_commitBtn->setStyleSheet("QPushButton { background-color: #3730A3; color: white; padding: 6px 16px; border-radius: 6px; } QPushButton:hover { background-color: #4F46E5; }");
    commitActionsLayout->addWidget(m_commitBtn);
    
    m_commitMenu = new QMenu(m_commitBtn);
    m_commitMenu->addAction("Commit Tracked", this, &GitPanel::onCommit);
    m_commitMenu->addAction("Commit Staged", this, &GitPanel::onCommitStaged);
    m_commitMenu->addAction("Amend", this, &GitPanel::onCommitAmend);
    m_commitBtn->setMenu(m_commitMenu);
    
    commitLayout->addLayout(commitActionsLayout);
    
    mainLayout->addWidget(commitSection);

    // === RECENT COMMITS ===
    QWidget *historySection = new QWidget(m_content);
    historySection->setStyleSheet("padding: 8px 12px;");
    QVBoxLayout *historyLayout = new QVBoxLayout(historySection);
    historyLayout->setContentsMargins(12, 0, 12, 0);
    historyLayout->setSpacing(8);
    
    QLabel *historyHeader = new QLabel("History", historySection);
    historyHeader->setStyleSheet("font-weight: 600; color: #808080; font-size: 11px; text-transform: uppercase;");
    historyLayout->addWidget(historyHeader);
    
    m_recentCommits = new QListWidget(historySection);
    historyLayout->addWidget(m_recentCommits, 1);
    
    mainLayout->addWidget(historySection, 1);

    // === STATUS BAR ===
    m_statusLabel = new QLabel("Ready", m_content);
    m_statusLabel->setStyleSheet("background-color: #252526; color: #808080; padding: 6px 12px; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);

    setWidget(m_content);
    
    // === CONNECTIONS ===
    connect(m_stageAllBtn, &QPushButton::clicked, this, &GitPanel::onStageAll);
    connect(m_commitBtn, &QPushButton::clicked, this, &GitPanel::onCommit);
    connect(m_gitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GitPanel::onGitFinished);
    
    // Auto-refresh every 30 seconds
    m_autoRefresh = new QTimer(this);
    connect(m_autoRefresh, &QTimer::timeout, this, &GitPanel::autoRefresh);
    m_autoRefresh->start(30000);
    
    // Initial load
    refresh();
}

GitPanel::~GitPanel()
{
    m_autoRefresh->stop();
}

void GitPanel::runGit(const QStringList &args)
{
    m_statusLabel->setText("Running: git " + args.join(' '));
    m_gitProcess->setWorkingDirectory(m_repoPath);
    m_gitProcess->start("git", args);
}

void GitPanel::refresh()
{
    refreshStatus();
    refreshBranches();
    refreshRecentCommits();
}

void GitPanel::autoRefresh()
{
    refreshStatus();
    refreshBranches();
}

void GitPanel::refreshStatus()
{
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start("git", {"status", "--porcelain"});
    proc.waitForFinished();
    
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    
    m_changesList->clear();
    
    if (lines.isEmpty()) {
        m_noChangesLabel->setVisible(true);
        m_changesList->setVisible(false);
        m_stageAllBtn->setEnabled(false);
    } else {
        m_noChangesLabel->setVisible(false);
        m_changesList->setVisible(true);
        m_stageAllBtn->setEnabled(true);
        
        for (const QString &line : lines) {
            if (line.length() < 3) continue;
            
            QString status = line.left(2);
            QString file = line.mid(3);
            QString icon;
            QString color;
            
            if (status == "M " || status == "MM") {
                icon = "📝";
                color = "#F59E0B";
            } else if (status == "A " || status == "AM" || status == "MM") {
                icon = "➕";
                color = "#10B981";
            } else if (status == "D " || status == " D") {
                icon = "❌";
                color = "#EF4444";
            } else if (status == "??") {
                icon = "❓";
                color = "#808080";
            } else if (status.startsWith("R")) {
                icon = "📍";
                color = "#3B82F6";
            } else {
                icon = "📄";
                color = "#D4D4D4";
            }
            
            QListWidgetItem *item = new QListWidgetItem(QString("%1  %2").arg(icon).arg(file));
            item->setForeground(QColor(color));
            m_changesList->addItem(item);
        }
    }
}

void GitPanel::refreshBranches()
{
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    
    // Get current branch
    proc.start("git", {"rev-parse", "--abbrev-ref", "HEAD"});
    proc.waitForFinished();
    QString currentBranch = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
    
    // Get upstream
    proc.start("git", {"rev-parse", "--abbrev-ref", "@{upstream}"});
    proc.waitForFinished();
    QString upstream = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
    
    if (currentBranch.isEmpty()) currentBranch = "(detached)";
    if (upstream.isEmpty()) upstream = currentBranch;
    
    m_branchIndicator->setText(QString("%1 → %2").arg(currentBranch).arg(upstream));
    m_pathLabel->setText(QString("%1 / %2").arg(QFileInfo(m_repoPath).fileName()).arg(currentBranch));
}

void GitPanel::refreshRecentCommits()
{
    m_recentCommits->clear();
    
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start("git", {"log", "--oneline", "-10", "--decorate"});
    proc.waitForFinished();
    
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        QString commit = line.trimmed();
        if (commit.isEmpty()) continue;
        
        QListWidgetItem *item = new QListWidgetItem("  " + commit);
        item->setForeground(QColor("#D4D4D4"));
        
        if (commit.contains("(HEAD")) {
            item->setForeground(QColor("#7C3AED"));
        }
        
        m_recentCommits->addItem(item);
    }
}

void GitPanel::onFetch()
{
    runGit({"fetch"});
}

void GitPanel::onFetchPrune()
{
    runGit({"fetch", "--prune"});
}

void GitPanel::onStageAll()
{
    runGit({"add", "-A"});
    QTimer::singleShot(500, this, &GitPanel::refreshStatus);
}

void GitPanel::onCommit()
{
    QString message = m_commitInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        m_statusLabel->setText("Enter a commit message");
        return;
    }
    
    runGit({"commit", "-m", message});
    QTimer::singleShot(500, this, [this]() {
        m_commitInput->clear();
        refreshStatus();
        refreshRecentCommits();
    });
}

void GitPanel::onCommitStaged()
{
    QString message = m_commitInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        m_statusLabel->setText("Enter a commit message");
        return;
    }
    
    runGit({"commit", "-m", message});
    QTimer::singleShot(500, this, [this]() {
        m_commitInput->clear();
        refreshStatus();
        refreshRecentCommits();
    });
}

void GitPanel::onCommitAmend()
{
    runGit({"commit", "--amend", "--no-edit"});
    QTimer::singleShot(500, this, [this]() {
        refreshRecentCommits();
    });
}

void GitPanel::onPush()
{
    runGit({"push"});
}

void GitPanel::onPull()
{
    runGit({"pull"});
}

void GitPanel::onUndoCommit()
{
    runGit({"reset", "--soft", "HEAD~1"});
    QTimer::singleShot(500, this, [this]() {
        refreshStatus();
        refreshRecentCommits();
    });
}

void GitPanel::onGitFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        m_statusLabel->setText("✓ Done");
    } else {
        m_statusLabel->setText(QString("✗ Failed (%1)").arg(exitCode));
    }
    refresh();
}
