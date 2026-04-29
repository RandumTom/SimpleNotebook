#include "gitwindow.h"
#include <QProcess>
#include <QPushButton>
#include <QGroupBox>
#include <QSplitter>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>

GitWindow::GitWindow(const QString &repoPath, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , m_repoPath(repoPath)
    , m_gitProcess(new QProcess(this))
{
    setWindowTitle("Source Control - " + QFileInfo(repoPath).fileName());
    setMinimumSize(600, 400);
    resize(700, 500);
    
    // Dark theme matching the app
    setStyleSheet(R"(
        QWidget {
            background-color: #1E1E1E;
            color: #D4D4D4;
            font-family: "Segoe UI", sans-serif;
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
        }
        QPushButton:hover {
            background-color: #2D2D2D;
        }
        QPushButton.primary {
            background-color: #3730A3;
            color: white;
            font-weight: 500;
        }
        QPushButton.primary:hover {
            background-color: #4F46E5;
        }
        QPushButton.success {
            background-color: #059669;
            color: white;
        }
        QPushButton.success:hover {
            background-color: #10B981;
        }
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            padding: 8px 12px;
            border-radius: 4px;
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
        }
        QPlainTextEdit:focus {
            border-color: #7C3AED;
        }
        QScrollBar:vertical {
            background-color: transparent;
            width: 8px;
        }
        QScrollBar::handle:vertical {
            background-color: #3E3E42;
            border-radius: 4px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // === HEADER ===
    QWidget *header = new QWidget(this);
    header->setStyleSheet("background-color: #252526; padding: 12px;");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 12, 8);
    
    m_headerLabel = new QLabel("📁 " + QFileInfo(repoPath).fileName(), this);
    m_headerLabel->setStyleSheet("font-weight: 600; font-size: 14px;");
    headerLayout->addWidget(m_headerLabel);
    
    m_branchLabel = new QLabel("main → main", this);
    m_branchLabel->setStyleSheet("color: #10B981; font-size: 12px; padding: 4px 8px; background-color: #1A1A1A; border-radius: 4px;");
    headerLayout->addWidget(m_branchLabel);
    
    m_fetchBtn = new QPushButton("↻ Fetch", this);
    m_fetchBtn->setStyleSheet("color: #10B981;");
    headerLayout->addWidget(m_fetchBtn);
    
    mainLayout->addWidget(header);

    // === CHANGES SECTION ===
    QWidget *changesSection = new QWidget(this);
    changesSection->setStyleSheet("padding: 12px;");
    QVBoxLayout *changesLayout = new QVBoxLayout(changesSection);
    changesLayout->setContentsMargins(12, 12, 12, 0);
    
    // Changes header
    QHBoxLayout *changesHeaderLayout = new QHBoxLayout();
    m_changesHeader = new QLabel("Changes", this);
    m_changesHeader->setStyleSheet("font-weight: 600; color: #808080; font-size: 11px; text-transform: uppercase;");
    changesHeaderLayout->addWidget(m_changesHeader);
    
    QPushButton *optionsBtn = new QPushButton("⋮", this);
    optionsBtn->setFixedWidth(30);
    changesHeaderLayout->addWidget(optionsBtn);
    
    m_stageAllBtn = new QPushButton("Stage All", this);
    m_stageAllBtn->setStyleSheet("color: #10B981; font-size: 12px;");
    changesHeaderLayout->addWidget(m_stageAllBtn);
    
    changesHeaderLayout->addStretch();
    changesLayout->addLayout(changesHeaderLayout);
    
    // Changes list
    m_changesList = new QListWidget(this);
    m_changesList->setMinimumHeight(120);
    changesLayout->addWidget(m_changesList);
    
    m_noChangesLabel = new QLabel("✓ No changes to commit", this);
    m_noChangesLabel->setStyleSheet("color: #10B981; padding: 20px; text-align: center;");
    m_noChangesLabel->setAlignment(Qt::AlignCenter);
    changesLayout->addWidget(m_noChangesLabel);
    
    mainLayout->addWidget(changesSection);

    // === COMMIT INPUT ===
    QWidget *commitSection = new QWidget(this);
    commitSection->setStyleSheet("padding: 0 12px;");
    QVBoxLayout *commitLayout = new QVBoxLayout(commitSection);
    commitLayout->setContentsMargins(12, 0, 12, 0);
    commitLayout->setSpacing(8);
    
    m_commitInput = new QPlainTextEdit(this);
    m_commitInput->setPlaceholderText("Enter commit message...");
    m_commitInput->setMaximumHeight(80);
    commitLayout->addWidget(m_commitInput);
    
    // Commit buttons row
    QHBoxLayout *commitBtnsLayout = new QHBoxLayout();
    
    QPushButton *aiCommitBtn = new QPushButton("✨ AI Commit", this);
    aiCommitBtn->setStyleSheet("color: #A855F7;");
    commitBtnsLayout->addWidget(aiCommitBtn);
    
    commitBtnsLayout->addStretch();
    
    m_commitBtn = new QPushButton("Commit", this);
    m_commitBtn->setStyleSheet("QPushButton { background-color: #3730A3; color: white; padding: 8px 16px; border-radius: 6px; } QPushButton:hover { background-color: #4F46E5; }");
    commitBtnsLayout->addWidget(m_commitBtn);
    
    commitLayout->addLayout(commitBtnsLayout);
    
    mainLayout->addWidget(commitSection);

    // === RECENT COMMITS ===
    QWidget *commitsSection = new QWidget(this);
    commitsSection->setStyleSheet("padding: 12px;");
    QVBoxLayout *commitsLayout = new QVBoxLayout(commitsSection);
    commitsLayout->setContentsMargins(12, 12, 12, 0);
    
    QLabel *recentHeader = new QLabel("Recent", this);
    recentHeader->setStyleSheet("font-weight: 600; color: #808080; font-size: 11px; text-transform: uppercase;");
    commitsLayout->addWidget(recentHeader);
    
    m_recentCommits = new QListWidget(this);
    commitsLayout->addWidget(m_recentCommits, 1);
    
    mainLayout->addWidget(commitsSection, 1);

    // === STATUS BAR ===
    m_statusBar = new QLabel("Ready", this);
    m_statusBar->setStyleSheet("background-color: #252526; color: #808080; padding: 6px 12px; font-size: 11px;");
    mainLayout->addWidget(m_statusBar);

    // === CONNECTIONS ===
    connect(m_fetchBtn, &QPushButton::clicked, this, &GitWindow::onFetch);
    connect(m_stageAllBtn, &QPushButton::clicked, this, &GitWindow::onStageAll);
    connect(m_commitBtn, &QPushButton::clicked, this, &GitWindow::onCommit);
    connect(m_gitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GitWindow::onGitFinished);
    connect(m_gitProcess, &QProcess::readyReadStandardOutput, this, &GitWindow::onReadOutput);
    connect(m_gitProcess, &QProcess::readyReadStandardError, this, &GitWindow::onReadError);

    // Auto-refresh every 30 seconds
    m_autoRefresh = new QTimer(this);
    connect(m_autoRefresh, &QTimer::timeout, this, &GitWindow::autoRefresh);
    m_autoRefresh->start(30000);

    // Initial load
    onRefresh();
}

GitWindow::~GitWindow()
{
    m_autoRefresh->stop();
}

void GitWindow::runGit(const QStringList &args, const QString &description)
{
    m_statusBar->setText("Running: git " + args.join(' '));
    m_gitProcess->setWorkingDirectory(m_repoPath);
    m_gitProcess->start("git", args);
}

void GitWindow::onRefresh()
{
    refreshStatus();
    refreshBranches();
    refreshRecentCommits();
}

void GitWindow::autoRefresh()
{
    refreshStatus();
    refreshBranches();
}

void GitWindow::refreshStatus()
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
            } else if (status == "A " || status == "AM") {
                icon = "➕";
                color = "#10B981";
            } else if (status == "D ") {
                icon = "❌";
                color = "#EF4444";
            } else if (status == "??") {
                icon = "❓";
                color = "#808080";
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

void GitWindow::refreshBranches()
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
    
    m_branchLabel->setText(QString("%1 → %2").arg(currentBranch).arg(upstream));
}

void GitWindow::refreshRecentCommits()
{
    m_recentCommits->clear();
    
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start("git", {"log", "--oneline", "-10", "--decorate"});
    proc.waitForFinished();
    
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        // Parse: [icon] hash message (branch)
        QString commit = line.trimmed();
        if (commit.isEmpty()) continue;
        
        QListWidgetItem *item = new QListWidgetItem("  " + commit);
        item->setForeground(QColor("#D4D4D4"));
        
        // Highlight HEAD commit
        if (commit.contains("(HEAD")) {
            item->setForeground(QColor("#7C3AED"));
        }
        
        m_recentCommits->addItem(item);
    }
}

void GitWindow::onFetch()
{
    runGit({"fetch", "--all"}, "Fetch");
}

void GitWindow::onStageAll()
{
    runGit({"add", "-A"}, "Stage All");
    QTimer::singleShot(500, this, &GitWindow::refreshStatus);
}

void GitWindow::onCommit()
{
    QString message = m_commitInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        m_statusBar->setText("⚠ Enter a commit message");
        return;
    }
    
    // Escape message for shell
    message.replace("\"", "\\\"");
    runGit({"commit", "-m", message}, "Commit");
    QTimer::singleShot(500, this, [this]() {
        m_commitInput->clear();
        refreshStatus();
        refreshRecentCommits();
    });
}

void GitWindow::onPush()
{
    runGit({"push"}, "Push");
}

void GitWindow::onPull()
{
    runGit({"pull"}, "Pull");
}

void GitWindow::onGitFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        m_statusBar->setText("✓ Done");
    } else {
        m_statusBar->setText(QString("✗ Failed (exit code: %1)").arg(exitCode));
    }
}

void GitWindow::onReadOutput()
{
    // Git output already handled
}

void GitWindow::onReadError()
{
    QString error = QString::fromLocal8Bit(m_gitProcess->readAllStandardError());
    if (!error.isEmpty()) {
        m_statusBar->setText("Error: " + error.trimmed().left(50));
    }
}
