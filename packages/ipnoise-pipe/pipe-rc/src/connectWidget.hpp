#ifndef CONNECT_WIDGET_HPP
#define CONNECT_WIDGET_HPP

#include <stdio.h>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

#include "loginWidget.hpp"
#include <ipnoise-common/log.h>
#include <ipnoise-common/path.hpp>
#include <ipnoise-common/qt.hpp>

class ConnectWidget
    :   public  QWidget
{
    Q_OBJECT

    signals:
        void doConnect();

    public:
        ConnectWidget(QWidget *a_parent = 0);
        virtual ~ConnectWidget();

        void        setupUi(QWidget *a_parent);
        void        rotate();
        QString     getLogin();
        QString     getPassword();
        bool        isNewLogin();
        void        setDisabled(const bool &);

    public slots:
        void loginButtonSlot(bool);

    private:
        QVBoxLayout         *m_vbox;
        QHBoxLayout         *m_status_icon_hbox;
        QSpacerItem         *m_vbox_spacer_1;
        QSpacerItem         *m_vbox_spacer_2;
        QLabel              *m_status_icon;
        LoginWidget         *m_login_widget;
        int                 m_status_icon_degrees;
};

#endif

