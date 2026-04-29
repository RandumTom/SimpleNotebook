#include "gitwindow.h"
#include <QProcess>
#include <QPushButton>
#include <QGroupBox>
#include <QSplitter>
#include <QCoreApplication>

GitWindow::GitWindow(const QString &repoPath, QWidget *parent)
    : QWidget(parent, Qt::Window)
    , m_repoPath(repoPath)
    , m_gitProcess(new QProcess(this))
{
    setWindowTitle("Git - " + repoPath);
    setMinimumSize(800, 500);
    resize(900, 600);
    
    setStyleSheet(
        "QWidget { background-color: #1E1E1E; color: #D4D4D4; }"
        "QTextEdit { background-color: #0D0D0D; color: #00FF00; border: 1px solid #3E3E42; font-family: Consolas; font-size: 12px; }"
        "QListWidget { background-color: #1E1E1E; color: #D4D4D4; border: 1px solid #3E3E42; }"
        "QListWidget::item:selected { background-color: #3730A3; }"
        "QLineEdit { background-color: #2D2D30; color: #D4D4D4; border: 1px solid #3E3E42; padding: 8px; }"
        "QPushButton { background-color: #3730A3; color: white; border: none; padding: 10px 16px; border-radius: 6px; font-weight: 500; } QPushButton:hover { background-color: #4F46E5; }"
        "QLabel { color: #D4D4D4; }"
        "QGroupBox { border: 1px solid #3E3E42; border-radius: 8px; padding: 10px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header with repo path
    QLabel *pathLabel = new QLabel("📁 " + m_repoPath, this);
    pathLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 10px;");
    mainLayout->addWidget(pathLabel);

    // Buttons row
    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *refreshBtn = new QPushButton("🔄 Refresh", this);
    QPushButton *statusBtn = new QPushButton("📊 Status", this);
    QPushButton *commitBtn = new QPushButton("💾 Commit", this);
    QPushButton *pushBtn = new QPushButton("⬆️ Push", this);
    QPushButton *pullBtn = new QPushButton("⬇️ Pull", this);
    QPushButton *logBtn = new QPushButton("📜 Log", this);
    
    btnRow->addWidget(refreshBtn);
    btnRow->addWidget(statusBtn);
    btnRow->addWidget(commitBtn);
    btnRow->addWidget(pushBtn);
    btnRow->addWidget(pullBtn);
    btnRow->addWidget(logBtn);
    btnRow->addStretch();
    mainLayout->addLayout(btnRow);

    // Commit message section
    QGroupBox *commitGroup = new QGroupBox("Commit Message", this);
    QVBoxLayout *commitLayout = new QVBoxLayout(commitGroup);
    
    m_commitMsgInput = new QLineEdit(this);
    m_commitMsgInput->setPlaceholderText("Enter commit message...");
    commitLayout->addWidget(m_commitMsgInput);
    
    QHBoxLayout *commitBtns = new QHBoxLayout();
    QPushButton *commitWithMsgBtn = new QPushButton("✓ Commit", this);
    QPushButton *commitAmendBtn = new QPushButton("✏️ Amend", this);
    commitBtns->addWidget(commitWithMsgBtn);
    commitBtns->addWidget(commitAmendBtn);
    commitBtns->addStretch();
    commitLayout->addLayout(commitBtns);
    mainLayout->addWidget(commitGroup);

    // Branches and Log side by side
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // Branches panel
    QWidget *branchPanel = new QWidget(this);
    QVBoxLayout *branchLayout = new QVBoxLayout(branchPanel);
    branchLayout->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout *branchHeader = new QHBoxLayout();
    QLabel *branchTitle = new QLabel("🌿 Branches", this);
    QPushButton *checkoutBtn = new QPushButton("Checkout", this);
    checkoutBtn->setStyleSheet("QPushButton { background-color: #10B981; padding: 5px 10px; } QPushButton:hover { background-color: #059669; }");
    branchHeader->addWidget(branchTitle);
    branchHeader->addWidget(checkoutBtn);
    branchLayout->addLayout(branchHeader);
    
    m_branchList = new QListWidget(this);
    branchLayout->addWidget(m_branchList);
    
    splitter->addWidget(branchPanel);

    // Log panel
    QWidget *logPanel = new QWidget(this);
    QVBoxLayout *logLayout = new QVBoxLayout(logPanel);
    logLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *logTitle = new QLabel("📋 Commit History", this);
    logLayout->addWidget(logTitle);
    
    m_logList = new QListWidget(this);
    logLayout->addWidget(m_logList);
    
    splitter->addWidget(logPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    
    mainLayout->addWidget(splitter, 1);

    // Output area
    m_outputEdit = new QTextEdit(this);
    m_outputEdit->setReadOnly(true);
    mainLayout->addWidget(m_outputEdit, 1);

    // Status bar
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("padding: 5px; background-color: #252526;");
    mainLayout->addWidget(m_statusLabel);

    // Connections
    connect(refreshBtn, &QPushButton::clicked, this, &GitWindow::onRefresh);
    connect(statusBtn, &QPushButton::clicked, this, &GitWindow::onStatus);
    connect(commitBtn, &QPushButton::clicked, [this]() {
        if (!m_commitMsgInput->text().isEmpty()) {
            runGit("commit -m \"" + m_commitMsgInput->text() + "\"", "Commit");
        }
    });
    connect(commitWithMsgBtn, &QPushButton::clicked, [this]() {
        if (!m_commitMsgInput->text().isEmpty()) {
            runGit("commit -m \"" + m_commitMsgInput->text() + "\"", "Commit");
        }
    });
    connect(commitAmendBtn, &QPushButton::clicked, [this]() {
        runGit("commit --amend", "Amend");
    });
    connect(pushBtn, &QPushButton::clicked, [this]() {
        runGit("push", "Push");
    });
    connect(pullBtn, &QPushButton::clicked, [this]() {
        runGit("pull", "Pull");
    });
    connect(logBtn, &QPushButton::clicked, this, &GitWindow::onLog);
    connect(checkoutBtn, &QPushButton::clicked, this, &GitWindow::onCheckout);
    connect(m_gitProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GitWindow::onGitFinished);
    connect(m_gitProcess, &QProcess::readyReadStandardOutput, this, &GitWindow::onReadOutput);
    connect(m_gitProcess, &QProcess::readyReadStandardError, this, &GitWindow::onReadError);

    // Initial load
    onRefresh();
}

GitWindow::~GitWindow()
{
}

void GitWindow::runGit(const QString &args, const QString &description)
{
    m_outputEdit->append("<span style='color: #7C3AED;'>=== " + description + " ===</span>");
    m_statusLabel->setText("Running: git " + args);
    
    m_gitProcess->setWorkingDirectory(m_repoPath);
    m_gitProcess->start("git", args.split(' ', Qt::SkipEmptyParts));
}

void GitWindow::onRefresh()
{
    m_statusLabel->setText("Refreshing...");
    refreshBranches();
    refreshLog();
    m_statusLabel->setText("Ready");
}

void GitWindow::refreshBranches()
{
    m_branchList->clear();
    
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start("git", {"branch", "-a"});
    proc.waitForFinished();
    
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    QString currentBranch;
    
    // Get current branch
    proc.start("git", {"rev-parse", "--abbrev-ref", "HEAD"});
    proc.waitForFinished();
    currentBranch = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
    
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        QString branch = line.trimmed();
        if (branch.startsWith("* ")) {
            branch = branch.mid(2);
            QListWidgetItem *item = new QListWidgetItem("● " + branch, m_branchList);
            item->setForeground(QColor("#10B981"));
        } else if (!branch.isEmpty()) {
            m_branchList->addItem("  " + branch);
        }
    }
}

void GitWindow::refreshLog()
{
    m_logList->clear();
    
    QProcess proc;
    proc.setWorkingDirectory(m_repoPath);
    proc.start("git", {"log", "--oneline", "--graph", "--decorate", "-20"});
    proc.waitForFinished();
    
    QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    for (const QString &line : output.split('\n', Qt::SkipEmptyParts)) {
        m_logList->addItem(line);
    }
}

void GitWindow::onStatus()
{
    runGit("status", "Status");
}

void GitWindow::onLog()
{
    runGit("log --oneline -10", "Log");
}

void GitWindow::onCommit()
{
    if (!m_commitMsgInput->text().isEmpty()) {
        runGit("commit -m \"" + m_commitMsgInput->text() + "\"", "Commit");
    }
}

void GitWindow::onPush()
{
    runGit("push", "Push");
}

void GitWindow::onPull()
{
    runGit("pull", "Pull");
}

void GitWindow::onBranch()
{
    runGit("branch -a", "Branches");
}

void GitWindow::onCheckout()
{
    QListWidgetItem *item = m_branchList->currentItem();
    if (item) {
        QString branch = item->text().trimmed();
        branch.replace("* ", "");
        runGit("checkout " + branch, "Checkout " + branch);
    }
}

void GitWindow::onGitFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit) {
        m_statusLabel->setText("✓ Done (exit code: " + QString::number(exitCode) + ")");
    } else {
        m_statusLabel->setText("✗ Failed (exit code: " + QString::number(exitCode) + ")");
    }
    onRefresh();
}

void GitWindow::onReadOutput()
{
    QString output = QString::fromLocal8Bit(m_gitProcess->readAllStandardOutput());
    m_outputEdit->append("<span style='color: #00FF00;'>" + output.toHtmlEscaped() + "</span>");
}

void GitWindow::onReadError()
{
    QString error = QString::fromLocal8Bit(m_gitProcess->readAllStandardError());
    if (!error.isEmpty()) {
        m_outputEdit->append("<span style='color: #EF4444;'>" + error.toHtmlEscaped() + "</span>");
    }
}
