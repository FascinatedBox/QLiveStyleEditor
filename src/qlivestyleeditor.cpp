#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSaveFile>
#include <QScreen>
#include <QStatusBar>
#include <QTextStream>
#include <QtGui/private/qcssparser_p.h>
#include <QtMessageHandler>

#include "qlivestyleeditor.h"


/* Css handling */


bool QLiveStyleEditor::_cssValid = true;
QString QLiveStyleEditor::_cssError = "";
const char *QLiveStyleEditor::_cssPrelude =
R"(
    QLiveStyleEditor QMenu {
        color: white;
        background-color: #353535;
        selection-background-color: palette(highlight);
    }
    QLiveStyleEditor QMenu {
        color: white;
    }
    QLiveStyleEditor QMenu:disabled {
        color: #777777;
    }


    QLiveStyleEditor QMenuBar {
        background-color: #353535;
        color: darkgray;
    }
    QLiveStyleEditor QMenuBar::item:selected {
        background-color: #353535;
        color: palette(highlight);
        border-bottom: 2px solid palette(highlight);
    }



    QLiveStyleEditor QScrollBar:vertical {
        background: #2c2c30;
        width: 12px;
        margin: 0px 4px 4px 0px;
        border-bottom: 1px solid #2c2c30;
        border-top: 1px solid #2c2c30;
        border-radius: 3px;
    }
    QLiveStyleEditor QScrollBar::handle:vertical {
        background: #565656;
        min-height: 20px;
        border: 1px solid #565656;
        border-radius: 3px;
    }
    QLiveStyleEditor QScrollBar::add-line:vertical,
    QLiveStyleEditor QScrollBar::sub-line:vertical
    {
        height: 0px;
    }



    QLiveStyleEditor QStatusBar {
        background-color: #c7cbd1;
        color: black;
    }



    QLiveStyleEditor QTextEdit {
        background-color: #0d0d0d;
        color: lightgray;
        font-family: Monospace;
        font-size: 11px;
        selection-background-color: #444444;
        selection-color: lightgray;
    }
)";

bool QLiveStyleEditor::isValidCss(QString text)
{
    QCss::Parser cssParser(text, /* is_file: */ false);
    QCss::StyleSheet styleSheet;

    _cssValid = true;

    bool ok = cssParser.parse(&styleSheet) && _cssValid;

    return ok;
}

void QLiveStyleEditor::messageHandler(QtMsgType t, const QMessageLogContext &context, const QString &msg)
{
    _cssError = msg;
    _cssValid = false;
}


/* Event handling */ 


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


/* Menu building */


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

void QLiveStyleEditor::createFileMenu()
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

void QLiveStyleEditor::createEditMenu()
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

void QLiveStyleEditor::createHelpMenu()
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

void QLiveStyleEditor::createMenuBar()
{
    createFileMenu();
    createEditMenu();
    createHelpMenu();
}


/* Slots */


void QLiveStyleEditor::onAbout()
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

void QLiveStyleEditor::onSave()
{
    if (trySave() == false)
        return;

    _styleEdit->document()->setModified(false);
    setWindowModified(false);
    statusBar()->showMessage(tr("Saved %1").arg(_cssPath), 2000);
}

void QLiveStyleEditor::onTextChanged()
{
    QString text = _styleEdit->toPlainText();

    setWindowModified(_styleEdit->document()->isModified());

    if (isValidCss(text) == false)
        return;

    text.prepend(_cssPrelude);
    _app->setStyleSheet(text);
}

void QLiveStyleEditor::onUndoAvailable(bool available)
{
    _undoAct->setEnabled(available);
}


/* Miscellaneous */


bool QLiveStyleEditor::trySave()
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
    resize(QGuiApplication::primaryScreen()->availableSize() * .3);

    qInstallMessageHandler(QLiveStyleEditor::messageHandler);

    connect(_styleEdit, &QTextEdit::textChanged,
            this, &QLiveStyleEditor::onTextChanged);

    QString text = _styleEdit->toPlainText();

    text.prepend(_cssPrelude);
    _app->setStyleSheet(text);
}
