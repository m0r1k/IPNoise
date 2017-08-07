#ifndef EVENTS_WINDOW_H
#define EVENTS_WINDOW_H

#include <assert.h>
#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QHBoxLayout>
#include <QDebug>

#include "ui_eventsWindow.h"

class EventsWindow : public QWidget, public Ui_EventsWindow
{
    Q_OBJECT

    public:
        EventsWindow(QWidget *parent = 0);
        ~EventsWindow();

        void appendItem(
            const char *_date,
            const char *_type,
            const char *_data
        );

    private:
        QHBoxLayout     *horizontalLayout;

    public slots:
        void itemClicked(QTableWidgetItem *item);

};
#endif

