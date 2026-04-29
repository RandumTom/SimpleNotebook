#include "startpage.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>

StartPage::StartPage(QWidget *parent)
    : QWidget(parent)
{
    // Dark mode background
    setStyleSheet(
        "QWidget {"
        "   background-color: #1E1E1E;"
        "}"
        "QLabel {"
        "   color: #D4D4D4;"
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Welcome label with gradient effect via shadow
    QLabel *welcomeLabel = new QLabel("📚 SimpleNotebook", this);
    welcomeLabel->setStyleSheet(
        "font-size: 36px; "
        "font-weight: bold; "
        "color: #D4D4D4; "
        "margin-bottom: 5px;"
    );
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Subtitle
    QLabel *subtitleLabel = new QLabel("Your dark-themed school notes organizer", this);
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
        {"German", "🇩🇪", "#7C3AED"},         // Purple
        {"Mathematics", "📐", "#3B82F6"},     // Blue
        {"English", "🇬🇧", "#10B981"},        // Green
        {"Biology", "🧬", "#22C55E"},         // Lime
        {"Chemistry", "⚗️", "#A855F7"},       // Violet
        {"Physics", "⚡", "#6366F1"},          // Indigo
        {"History", "📜", "#F59E0B"},         // Amber
        {"Geography", "🌍", "#14B8A6"},       // Teal
        {"Music", "🎵", "#EC4899"},            // Pink
        {"Art", "🎨", "#F97316"},              // Orange
        {"Physical Education", "⚽", "#84CC16"}, // Lime
        {"Computer Science", "💻", "#64748B"}, // Slate
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

    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(subjectsGroup);

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

    mainLayout->addWidget(customBtn);
    mainLayout->addWidget(m_folderLabel);

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

    // Get the user's documents folder and create a SchoolNotes subfolder
    QString documentsPath = QDir::homePath() + "/Documents/SchoolNotes/" + subjectName;

    // Create the folder if it doesn't exist
    QDir dir;
    if (!dir.mkpath(documentsPath)) {
        return;
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
        m_folderLabel->setText("📂 " + folderPath);
        emit folderSelected(folderPath);
    }
}
