#include <QApplication>

#include "qlivestyleeditor.h"
#include "demomainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QLiveStyleEditor *l = new QLiveStyleEditor(&app, "resources/demo.qss");
    DemoMainWindow *mw = new DemoMainWindow();
    mw->show();
    l->show();

    return app.exec();
}
