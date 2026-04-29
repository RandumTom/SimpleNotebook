#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QListWidgetItem>

class QPlainTextEdit;
class QListWidget;
class QPushButton;
class QLabel;
class QToolBar;
class QStatusBar;
class QLineEdit;

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

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onNew();
    void onSave();
    void onFileSelected(QListWidgetItem *item);
    void onAddClicked();
    void onTextChanged();
    void onCursorPositionChanged();
    void autoSave();
    
    // Search
    void onSearchToggle();
    void onSearchTextChanged(const QString &text);
    void onSearchNext();
    void onSearchPrevious();
    void highlightAllMatches(const QString &searchText);

private:
    void refreshFileList();
    bool saveCurrentFile();
    void updateStatusBar();
    void updateLineColumnIndicator();
    QString getCurrentFolder() const { return m_folderPath; }
    
    // Highlight current line
    void highlightCurrentLine();

    QString m_folderPath;
    QString m_currentFile;
    bool m_isUnsaved = false;
    QTimer *m_autoSaveTimer;
    int m_autoSaveInterval = 30000; // 30 seconds

    QWidget *m_sidebar;
    QToolBar *m_toolbar;
    QListWidget *m_fileList;
    QPlainTextEdit *m_textEdit;
    QLabel *m_folderLabel;
    QStatusBar *m_statusBar;
    
    // Search widgets
    QWidget *m_searchBar;
    QLineEdit *m_searchInput;
    QLabel *m_searchLabel;
    QPushButton *m_searchPrevBtn;
    QPushButton *m_searchNextBtn;
    QPushButton *m_searchCloseBtn;
    bool m_searchVisible = false;
    int m_currentMatchIndex = 0;
};

#endif // EDITORVIEW_H
