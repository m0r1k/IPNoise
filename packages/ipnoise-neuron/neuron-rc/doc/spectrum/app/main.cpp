#include <QtGui/QApplication>
#include "mainwidget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("QtMultimedia spectrum analyzer");
    MainWidget w;

    w.show();

    return app.exec();
}

