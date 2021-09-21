#include "qlivestyleeditor.h"

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    QLiveStyleEditor *l = new QLiveStyleEditor(&app, "resources/style.qss");

    l->show();
    return app.exec();
}
