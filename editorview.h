#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QListWidgetItem>

class QPlainTextEdit;
class QListWidget;
class QPushButton;
class QLabel;
class QToolBar;
class QStatusBar;
class QFileInfo;

class EditorView : public QWidget
{
    Q_OBJECT

public:
    explicit EditorView(QWidget *parent = nullptr);
    ~EditorView();

    void setFolder(const QString &folderPath);
    void loadFile(const QString &filePath);
    QString currentFile() const { return m_currentFile; }

signals:
    void backRequested();

private slots:
    void onNew();
    void onSave();
    void onFileSelected(QListWidgetItem *item);
    void onAddClicked();
    void onTextChanged();

private:
    void refreshFileList();
    void updateStatusBar();
    bool saveCurrentFile();
    QString getCurrentFolder() const { return m_folderPath; }

    QString m_folderPath;
    QString m_currentFile;
    bool m_isUnsaved = false;

    QToolBar *m_toolbar;
    QListWidget *m_fileList;
    QPlainTextEdit *m_textEdit;
    QLabel *m_folderLabel;
    QStatusBar *m_statusBar;
};

#endif // EDITORVIEW_H
