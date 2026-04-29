#include "startpage.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QProcess>

StartPage::StartPage(QWidget *parent)
    : QWidget(parent)
{
    // Dark mode background - solid color
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(pal);

    // Main layout with margins
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Welcome label
    QLabel *welcomeLabel = new QLabel("📚 SimpleNotebook", this);
    welcomeLabel->setStyleSheet(
        "font-size: 36px; "
        "font-weight: bold; "
        "color: #D4D4D4; "
        "margin-bottom: 5px;"
    );
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Subtitle
    QLabel *subtitleLabel = new QLabel("Notes saved to ~/SimpleNotebook/", this);
    subtitleLabel->setStyleSheet(
        "font-size: 14px; "
        "color: #808080; "
        "margin-bottom: 40px;"
    );
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // School subjects group
    QGroupBox *subjectsGroup = new QGroupBox("Select a Subject", this);
    subjectsGroup->setStyleSheet(
        "QGroupBox {"
        "   font-size: 14px; "
        "   font-weight: bold;"
        "   color: #D4D4D4;"
        "   border: 2px solid #3E3E42;"
        "   border-radius: 12px;"
        "   margin-top: 15px;"
        "   padding: 20px;"
        "   background-color: #252526;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top center;"
        "   padding: 0 15px;"
        "   background-color: #252526;"
        "}"
    );

    QGridLayout *subjectsLayout = new QGridLayout(subjectsGroup);
    subjectsLayout->setSpacing(15);
    subjectsLayout->setContentsMargins(20, 20, 20, 20);

    // Define school subjects with icons, colors
    struct Subject {
        QString name;
        QString icon;
        QString color;
    };

    QList<Subject> subjects = {
        {"German", "🇩🇪", "#7C3AED"},
        {"Mathematics", "📐", "#3B82F6"},
        {"English", "🇬🇧", "#10B981"},
        {"Biology", "🧬", "#22C55E"},
        {"Chemistry", "⚗️", "#A855F7"},
        {"Physics", "⚡", "#6366F1"},
        {"History", "📜", "#F59E0B"},
        {"Geography", "🌍", "#14B8A6"},
        {"Music", "🎵", "#EC4899"},
        {"Art", "🎨", "#F97316"},
        {"Physical Education", "⚽", "#84CC16"},
        {"Computer Science", "💻", "#64748B"},
    };

    int row = 0, col = 0;
    for (const Subject &subject : subjects) {
        QPushButton *btn = new QPushButton(QString("  %1 %2").arg(subject.icon).arg(subject.name), this);
        btn->setMinimumSize(160, 55);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 10px;"
            "   padding: 12px 16px;"
            "   text-align: left;"
            "}"
            "QPushButton:hover {"
            "   background-color: #FFFFFF;"
            "   color: #1E1E1E;"
            "   padding-left: 20px;"
            "}"
        ).arg(subject.color));

        btn->setProperty("subjectName", subject.name);
        subjectsLayout->addWidget(btn, row, col);
        connect(btn, &QPushButton::clicked, this, &StartPage::onSubjectClicked);

        col++;
        if (col > 2) {
            col = 0;
            row++;
        }
    }

    // Custom folder button
    QPushButton *customBtn = new QPushButton("  📁  Choose Custom Folder", this);
    customBtn->setMinimumWidth(220);
    customBtn->setMinimumHeight(45);
    customBtn->setCursor(Qt::PointingHandCursor);
    customBtn->setStyleSheet(
        "QPushButton {"
        "   font-size: 14px;"
        "   background-color: transparent;"
        "   color: #808080;"
        "   border: 2px solid #3E3E42;"
        "   border-radius: 10px;"
        "   padding: 10px 20px;"
        "   margin-top: 30px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3E3E42;"
        "   color: #D4D4D4;"
        "   border-color: #7C3AED;"
        "}"
    );

    m_folderLabel = new QLabel(this);
    m_folderLabel->setStyleSheet(
        "font-size: 12px; "
        "color: #606060; "
        "margin-top: 15px;"
    );
    m_folderLabel->setAlignment(Qt::AlignCenter);
    m_folderLabel->setWordWrap(true);

    // Add everything to layout
    mainLayout->addStretch();  // Push content to center
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(subjectsGroup);
    mainLayout->addWidget(customBtn);
    mainLayout->addWidget(m_folderLabel);
    mainLayout->addStretch();  // Fill remaining space
    mainLayout->setAlignment(Qt::AlignCenter);

    connect(customBtn, &QPushButton::clicked, this, &StartPage::onSelectFolder);
}

StartPage::~StartPage()
{
}

void StartPage::onSubjectClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
        return;

    QString subjectName = btn->property("subjectName").toString();

    // Get the user's home folder and create a SimpleNotebook subfolder
    QString documentsPath = QDir::homePath() + "/SimpleNotebook/" + subjectName;

    // Create the folder if it doesn't exist
    QDir dir;
    if (!dir.mkpath(documentsPath)) {
        return;
    }
    
    // Initialize git repo if not already
    QDir gitDir(documentsPath);
    if (!gitDir.exists(".git")) {
        QProcess gitInit;
        gitInit.setWorkingDirectory(documentsPath);
        gitInit.start("git", {"init"});
        gitInit.waitForFinished();
    }

    m_folderLabel->setText("📂 " + documentsPath);
    emit folderSelected(documentsPath);
}

void StartPage::onSelectFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Notes Folder",
        QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folderPath.isEmpty()) {
        // Initialize git repo if not already
        QDir gitDir(folderPath);
        if (!gitDir.exists(".git")) {
            QProcess gitInit;
            gitInit.setWorkingDirectory(folderPath);
            gitInit.start("git", {"init"});
            gitInit.waitForFinished();
        }
        
        m_folderLabel->setText("📂 " + folderPath);
        emit folderSelected(folderPath);
    }
}
