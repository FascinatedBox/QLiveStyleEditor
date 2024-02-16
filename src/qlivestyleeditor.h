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
    QLiveStyleEditor(QApplication *, QString);

private slots:
    void onAbout(void);
    void onCutCopyAvailable(bool);
    void onRedoAvailable(bool);
    void onSave(void);
    void onTextChanged(void);
    void onUndoAvailable(bool);

private:
    static const char *_cssPrelude;
    static void messageHandler(QtMsgType,
                               const QMessageLogContext &,
                               const QString &);

    QAction *buildAction(QMenu *, QString, QString, QKeySequence::StandardKey);
    void closeEvent(QCloseEvent *);
    void createEditMenu(void);
    void createFileMenu(void);
    void createHelpMenu(void);
    void createMenuBar(void);
    void loadStylesheetIntoEditor(QString);
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
