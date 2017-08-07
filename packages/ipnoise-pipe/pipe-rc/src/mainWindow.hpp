class MainWindow;

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <unistd.h>

#include <QApplication>
#include <QLabel>
#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QDomDocument>
#include <QDomElement>
#include <QVBoxLayout>

#include "api.hpp"
#include "contactListWidget.hpp"
#include "config.hpp"
#include "main.hpp"

#include "connectWidget.hpp"

#define MAX_STATUS_DOTS     10
#define MAX_START_TIME_SEC  30

class MainWindow
    :   public QWidget
{
    Q_OBJECT

    public:
        MainWindow();
        virtual ~MainWindow();

        void    setupUi(QWidget *a_parent);
        int     init(QApplication *, Config *);
        int     load();
        void    statusMsg(const QString &a_msg);
        void    statusDot();
        void    removeAll();
        void    showConnectWidget();
        void    showContactListWidget();

    public slots:
        void    doConnectSlot();

    protected:
        virtual void    closeEvent(QCloseEvent *);
        void            _statusMsg(const QString &a_msg);

    private slots:
        void    initTimerSlot();
        void    apiConnectedSlot();
        void    apiDisconnectedSlot();
        void    apiEvent(ApiEventInit *);
        void    apiEvent(ApiEventLoginSuccess *);
        void    apiEvent(ApiEventLoginFailed *);

    private:
        Config              *m_config;
        int                 m_start_time;
        int                 m_started;
        QApplication        *m_app;
        QProcess            *m_ipnoise;
        QTimer              *m_init_timer;
        QString             m_last_status_msg;
        QString             m_last_status_dot_msg;
        Api                 *m_api;
        ContactListWidget   *m_contact_list_widget;
        QVBoxLayout         *m_vbox;
        ConnectWidget       *m_connect_widget;
        QLabel              *m_status_label;
};

#endif

