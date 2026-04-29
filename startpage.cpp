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
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Welcome label
    QLabel *welcomeLabel = new QLabel("📚 Welcome to SimpleNotebook", this);
    welcomeLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin-bottom: 10px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Subtitle
    QLabel *subtitleLabel = new QLabel("Your personal school notes organizer", this);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #666; margin-bottom: 30px;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // School subjects group
    QGroupBox *subjectsGroup = new QGroupBox("Select a Subject", this);
    subjectsGroup->setStyleSheet(
        "QGroupBox {"
        "   font-size: 14px; font-weight: bold;"
        "   border: 2px solid #4A90D9;"
        "   border-radius: 10px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top center;"
        "   padding: 0 10px;"
        "}"
    );

    QGridLayout *subjectsLayout = new QGridLayout(subjectsGroup);
    subjectsLayout->setSpacing(10);

    // Define school subjects with icons and colors
    struct Subject {
        QString name;
        QString icon;
        QString color;
    };

    QList<Subject> subjects = {
        {"German", "🇩🇪", "#4A90D9"},      // Deutsch
        {"Mathematics", "📐", "#E74C3C"},   // Mathe
        {"English", "🇬🇧", "#27AE60"},      // Englisch
        {"Biology", "🧬", "#2ECC71"},       // Biologie
        {"Chemistry", "⚗️", "#9B59B6"},    // Chemie
        {"Physics", "⚡", "#3498DB"},       // Physik
        {"History", "📜", "#D35400"},      // Geschichte
        {"Geography", "🌍", "#1ABC9C"},    // Geografie
        {"Music", "🎵", "#E91E63"},         // Musik
        {"Art", "🎨", "#FF5722"},          // Kunst
        {"Physical Education", "⚽", "#4CAF50"}, // Sport
        {"Computer Science", "💻", "#607D8B"},  // Informatik
    };

    int row = 0, col = 0;
    for (const Subject &subject : subjects) {
        QPushButton *btn = new QPushButton(QString("%1 %2").arg(subject.icon).arg(subject.name), this);
        btn->setMinimumSize(150, 50);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "   font-size: 14px;"
            "   background-color: %1;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "}"
            "QPushButton:hover {"
            "   background-color: rgba(0,0,0,0.2);"
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
    QPushButton *customBtn = new QPushButton("📁 Choose Custom Folder", this);
    customBtn->setMinimumWidth(200);
    customBtn->setMinimumHeight(40);
    customBtn->setStyleSheet(
        "QPushButton {"
        "   font-size: 14px;"
        "   background-color: #95A5A6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   margin-top: 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7F8C8D;"
        "}"
    );

    m_folderLabel = new QLabel(this);
    m_folderLabel->setStyleSheet("font-size: 12px; color: #888; margin-top: 10px;");
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

    m_folderLabel->setText(documentsPath);
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
        m_folderLabel->setText(folderPath);
        emit folderSelected(folderPath);
    }
}
