#include <QDir>
#include <QMenuBar>
#include <QMessageBox>
#include <QSaveFile>
#include <QStatusBar>
#include <QTextStream>

#include "qlivestyleeditor.h"

const char *QLiveStyleEditor::_cssPrelude =
R"(
/* This was made by passing a BreezeDark stylesheet through qstyleshaker. */

QLiveStyleEditor QWidget
{
    background-clip: border;
    background-color: #31363b;
    border-image: none;
    color: #eff0f1;
    selection-background-color: #3daee9;
    selection-color: #eff0f1;
}

QLiveStyleEditor QAbstractScrollArea
{
    background-color: transparent;
    border-radius: 0.09em;
    border: 0.09em solid #76797c;
}

QLiveStyleEditor QAbstractScrollArea::corner
{
    background: #31363b;
}

QLiveStyleEditor QScrollBar:horizontal
{
    background-color: #1d2023;
    border-radius: 0.17em;
    border: 0.04em transparent #1d2023;
    height: 0.65em;
    margin: 0.13em 0.65em 0.13em 0.65em;
}

QLiveStyleEditor QScrollBar:horizontal:hover
{
    background-color: #76797c;
}

QLiveStyleEditor QScrollBar::handle:horizontal
{
    background-color: #3daee9;
    border-radius: 0.17em;
    border: 0.04em solid #3daee9;
    min-width: 0.5em;
}

QLiveStyleEditor QScrollBar::handle:horizontal:hover
{
    background-color: #3daee9;
    border: 0.04em solid #3daee9;
}

QLiveStyleEditor QScrollBar:vertical
{
    background-color: #1d2023;
    border-radius: 0.17em;
    border: 0.04em transparent #1d2023;
    margin: 0.65em 0.13em 0.65em 0.13em;
    width: 0.65em;
}

QLiveStyleEditor QScrollBar:vertical:hover
{
    background-color: #76797c;
}

QLiveStyleEditor QScrollBar::handle:vertical
{
    background-color: #3daee9;
    border-radius: 0.17em;
    border: 0.04em solid #3daee9;
    min-height: 0.5em;
}

QLiveStyleEditor QScrollBar::handle:vertical:hover
{
    background-color: #3daee9;
    border: 0.04em solid #3daee9;
}

QLiveStyleEditor QScrollBar::add-line:horizontal,
QLiveStyleEditor QScrollBar::sub-line:horizontal,
QLiveStyleEditor QScrollBar::add-line:vertical,
QLiveStyleEditor QScrollBar::sub-line:vertical
{
    height: 0px;
    width: 0px;
}

QLiveStyleEditor QScrollBar:horizontal
{
    margin: 4px 0px 0px 0px;
}

QLiveStyleEditor QScrollBar:vertical
{
    margin: 0px 0px 0px 4px;
}

QLiveStyleEditor QTextEdit
{
    background-color: #1d2023;
    border: 0.04em solid #76797c;
    color: #eff0f1;
}

QLiveStyleEditor QMainWindow::separator
{
    background: transparent;
    border: 0.09em transparent #76797c;
}

QLiveStyleEditor QMenu::separator
{
    height: 0.09em;
    background-color: #76797c;
    padding-left: 0.2em;
    margin-top: 0.2em;
    margin-bottom: 0.2em;
    margin-left: 0.41em;
    margin-right: 0.41em;
}

QLiveStyleEditor QTextEdit:hover,
QLiveStyleEditor QTextEdit:focus
{
    border: 0.04em solid #3daee9;
    color: #eff0f1;
}

QLiveStyleEditor QTextEdit:hover:pressed
{
    background-color: #31363b;
}
)";

void QLiveStyleEditor::messageHandler(QtMsgType t,
                                      const QMessageLogContext &context,
                                      const QString &msg)
{
}

void QLiveStyleEditor::closeEvent(QCloseEvent *e)
{
    if (_styleEdit->document()->isModified()) {
        auto reply = QMessageBox::warning(
            this,
            tr("QLiveStyleEditor"),
            tr("Save changes to %1 before closing?").arg(_cssPath),
            QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
            QMessageBox::Save
        );

        if (reply == QMessageBox::Save && trySave())
            e->ignore();
        else if (reply == QMessageBox::Discard)
            e->accept();
        else if (reply == QMessageBox::Cancel)
            e->ignore();
    }
    else
        e->accept();
}

QAction *QLiveStyleEditor::buildAction(QMenu *menu,
                                       QString iconThemeName,
                                       QString menuTip,
                                       QKeySequence::StandardKey keySequence)
{
    const QIcon icon = QIcon::fromTheme(iconThemeName);
    QAction *act = new QAction(icon, menuTip, this);

    act->setShortcuts(keySequence);
    menu->addAction(act);
    return act;
}

void QLiveStyleEditor::createFileMenu(void)
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    connect(
        buildAction(
            fileMenu,
            "document-save",
            tr("&Save"),
            QKeySequence::Save
        ), &QAction::triggered, this, &QLiveStyleEditor::onSave
    );
}

void QLiveStyleEditor::createEditMenu(void)
{
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    _undoAct = buildAction(
        editMenu,
        "edit-undo",
        tr("&Undo"),
        QKeySequence::Undo
    );
    _undoAct->setEnabled(false);
    connect(_undoAct, &QAction::triggered, _styleEdit, &QTextEdit::undo);
    connect(_styleEdit, &QTextEdit::undoAvailable, this, &QLiveStyleEditor::onUndoAvailable);

    _redoAct = buildAction(
        editMenu,
        "edit-redo",
        tr("&Redo"),
        QKeySequence::Redo
    );
    _redoAct->setEnabled(false);
    connect(_redoAct, &QAction::triggered, _styleEdit, &QTextEdit::redo);
    connect(_styleEdit, &QTextEdit::redoAvailable, this, &QLiveStyleEditor::onRedoAvailable);

    editMenu->addSeparator();

    _cutAct = buildAction(
        editMenu,
        "edit-cut",
        tr("Cu&t"),
        QKeySequence::Cut
    );
    connect(_cutAct, &QAction::triggered, _styleEdit, &QTextEdit::cut);

    _copyAct = buildAction(
        editMenu,
        "edit-copy",
        tr("&Copy"),
        QKeySequence::Copy
    );
    connect(_copyAct, &QAction::triggered, _styleEdit, &QTextEdit::copy);
    _copyAct->setEnabled(false);
    connect(_styleEdit, &QTextEdit::copyAvailable, this, &QLiveStyleEditor::onCutCopyAvailable);

    connect(
        buildAction(
            editMenu,
            "edit-paste",
            tr("&Paste"),
            QKeySequence::Paste
        ), &QAction::triggered, _styleEdit, &QTextEdit::paste
    );

    editMenu->addSeparator();

    connect(
        buildAction(
            editMenu,
            "edit-select-all",
            tr("Select &All"),
            QKeySequence::SelectAll
        ), &QAction::triggered, _styleEdit, &QTextEdit::selectAll
    );
}

void QLiveStyleEditor::createHelpMenu(void)
{
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    connect(
        buildAction(
            helpMenu,
            "help-about",
            tr("&About QLiveStyleEditor"),
            QKeySequence::HelpContents
        ), &QAction::triggered, this, &QLiveStyleEditor::onAbout
    );
}

void QLiveStyleEditor::createMenuBar(void)
{
    createFileMenu();
    createEditMenu();
    createHelpMenu();
}

void QLiveStyleEditor::onAbout(void)
{
    (void) QMessageBox::information(
        this,
        tr("About QLiveStyleEditor"),
        tr(
            "QLiveStyleEditor is a tool for writing Qt stylesheets."
            "<br><br>"
            "QLiveStyleEditor applies styles immediately once they are valid."
            "<br><br>"
            "<a href=\"https://gitlab.com/FascinatedBox/QLiveStyleEditor\">https://gitlab.com/FascinatedBox/QLiveStyleEditor</a>"
        )
    );
}

void QLiveStyleEditor::onCutCopyAvailable(bool available)
{
    _cutAct->setEnabled(available);
    _copyAct->setEnabled(available);
}

void QLiveStyleEditor::onRedoAvailable(bool available)
{
    _redoAct->setEnabled(available);
}

void QLiveStyleEditor::onSave(void)
{
    if (trySave() == false)
        return;

    _styleEdit->document()->setModified(false);
    setWindowModified(false);
    statusBar()->showMessage(tr("Saved %1").arg(_cssPath), 2000);
}

void QLiveStyleEditor::onTextChanged(void)
{
    QString text = _styleEdit->toPlainText();

    setWindowModified(_styleEdit->document()->isModified());
    text.prepend(_cssPrelude);

    /* Prevent stylesheet error messages from spamming the console. */
    qInstallMessageHandler(QLiveStyleEditor::messageHandler);
    _app->setStyleSheet(text);

    /* Restore the message handler since qDebug uses it. */
    qInstallMessageHandler(0);
}

void QLiveStyleEditor::onUndoAvailable(bool available)
{
    _undoAct->setEnabled(available);
}

bool QLiveStyleEditor::trySave(void)
{
    QString errorMessage;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    QSaveFile f(_cssPath);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&f);
        out << _styleEdit->toPlainText();
        if (!f.commit())
            errorMessage = tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(_cssPath), f.errorString());
    }
    else
        errorMessage = tr("Cannot open file %1 for writing:\n%2.")
                       .arg(QDir::toNativeSeparators(_cssPath), f.errorString());

    QGuiApplication::restoreOverrideCursor();

    if (!errorMessage.isEmpty())
        QMessageBox::critical(this, tr("QLiveStyleEditor"), errorMessage);

    return errorMessage.isEmpty();
}

void QLiveStyleEditor::loadStylesheetIntoEditor(QString cssPath)
{
    if (QDir::isAbsolutePath(cssPath) == false) {
        cssPath = QCoreApplication::applicationDirPath() + "/" + cssPath;
    }

    QFile f(cssPath);

    if (f.open(QFile::ReadOnly | QFile::Text)) {
        QString sheet = f.readAll();

        _styleEdit->setPlainText(sheet);
        _styleEdit->document()->setModified(false);
        f.close();

        statusBar()->showMessage(
                tr("Opened %1").arg(cssPath), 2000);
    }
    else
        statusBar()->showMessage(
                tr("Failed to open %1: %2").arg(cssPath, f.errorString()));

    _cssPath = cssPath;
}

QLiveStyleEditor::QLiveStyleEditor(QApplication *app, QString cssPath)
    : QMainWindow(nullptr)
{
    _styleEdit = new QLiveStyleEditorEdit();
    createMenuBar();
    statusBar()->show();
    loadStylesheetIntoEditor(cssPath);

    setCentralWidget(_styleEdit);
    setObjectName("QLiveStyleEditor");
    setWindowFilePath(_cssPath + " — Style Editor");

    connect(_styleEdit, &QTextEdit::textChanged,
            this, &QLiveStyleEditor::onTextChanged);

    QString text = _styleEdit->toPlainText();

    text.prepend(_cssPrelude);
    _app->setStyleSheet(text);

    /* Assume new rules will be added at the bottom and start there. */
    QTextCursor cursor(_styleEdit->textCursor());

    cursor.movePosition(QTextCursor::End);
    _styleEdit->setTextCursor(cursor);
}
