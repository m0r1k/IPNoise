#ifndef LINK_UDP_WIDGET_HPP
#define LINK_UDP_WIDGET_HPP

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include "linkWidget.hpp"

class LinkUdpWidget
    :   public  LinkWidget
{
    Q_OBJECT

    public:
        LinkUdpWidget(QWidget *a_parent = 0);
        virtual ~LinkUdpWidget();

        void setupUi(QWidget *a_parent);

    private:
        QVBoxLayout     *m_vbox;
        QFormLayout     *m_form;
        QLabel          *m_descr;
        QLabel          *m_ip_label;
        QLineEdit       *m_ip_edit;
        QLabel          *m_port_label;
        QLineEdit       *m_port_edit;
};

#endif

