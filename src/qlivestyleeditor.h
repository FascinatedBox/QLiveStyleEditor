#ifndef QLIVESTYLEEDITOR_H
# define QLIVESTYLEEDITOR_H
# include <QApplication>
# include <QKeyEvent>
# include <QMainWindow>
# include <QTextEdit>

class QLiveStyleEditorEdit : public QTextEdit
{
public:
    QLiveStyleEditorEdit()
    {
        setAcceptRichText(false);
    }

protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (e->key() == Qt::Key_Tab &&
            e->modifiers() == Qt::NoModifier) {
            textCursor().insertText("    ");
            e->ignore();
            return;
        }

        QTextEdit::keyPressEvent(e);
    }
};

class QLiveStyleEditor : public QMainWindow
{
    Q_OBJECT

public:
    QLiveStyleEditor(QApplication *app, QString cssPath);

private slots:
    void onAbout();
    void onCutCopyAvailable(bool);
    void onRedoAvailable(bool);
    void onSave();
    void onTextChanged();
    void onUndoAvailable(bool);

private:
    // Css handling
    static bool _cssValid;
    static const char *_cssPrelude;
    static QString _cssError;
    static bool isValidCss(QString text);
    static void messageHandler(QtMsgType t,
                               const QMessageLogContext &context,
                               const QString &msg);

    // Event handling
    void closeEvent(QCloseEvent *e);

    // Menu building
    QAction *buildAction(QMenu *menu,
                         QString iconThemeName,
                         QString menuTip,
                         QKeySequence::StandardKey keySequence);
    void createMenuBar();
    void createFileMenu();
    void createEditMenu();
    void createHelpMenu();

    // Miscellaneous
    void loadStylesheetIntoEditor(QString cssPath);
    bool trySave();

    QAction *_copyAct;
    QAction *_cutAct;
    QAction *_redoAct;
    QAction *_undoAct;
    QApplication *_app;
    QLiveStyleEditorEdit *_styleEdit;
    QString _cssPath;
};

#endif
