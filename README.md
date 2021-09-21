QLiveStyleEditor
================

QLiveStyleEditor is a tool for writing Qt stylesheets that is easy to embed in
an existing project. Stylesheet rules written in QLiveStyleEditor are
immediately checked and applied if valid. The editor supports basic text editing
operations (cut, copy, paste, undo, redo, and save).

![Demo image](/resources/demo.gif)

# Usage

QLiveStyleEditor is composed of two files: `src/qlivestyleeditor.h` and
`src/qlivestyleeditor.cpp`. Copy them into your project and include them in the
build (can be qmake or cmake).

Once you've done that, all that's left is to create an instance of
QLiveStyleEditor.

```C++
#include "qlivestyleeditor.h"

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    QLiveStyleEditor *l = new QLiveStyleEditor(&app, "path-to-your-style.qss");

    l->show();
    return app.exec();
}
```
