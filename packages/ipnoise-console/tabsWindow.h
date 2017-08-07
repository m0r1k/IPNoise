#ifndef TABS_WINDOW_H
#define TABS_WINDOW_H

#include <assert.h>
#include <QWidget>
#include <QHash>

#include "eventsWindow.h"
#include "xmlEditor.h"
#include "ui_tabsWindow.h"

class TabInfo
{
    public:
        QWidget *parent;
        QWidget *child;
};

class TabsWindow : public QWidget, public Ui_TabsWindow
{
    Q_OBJECT

    public:
        TabsWindow(QWidget *parent = 0);
        ~TabsWindow();

    private:
        QHash <int, TabInfo *> tabs;

    public slots:
        void resizeEvent(QResizeEvent *event);
        void currentChangedSlot(int tab_id);

};

#endif

