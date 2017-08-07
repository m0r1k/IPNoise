#include "main.h"
#include "tabsWindow.h"
#include "eventsWindow.h"

#include <QApplication>
#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
    int ret, err = 0;

    QApplication app(argc, argv);
    TabsWindow win;

    win.show();
    app.exec();

ret:
    return err;
fail:
    err = -1;
    goto ret;

}

