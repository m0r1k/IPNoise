#ifndef LINK_TCP_HPP
#define LINK_TCP_HPP

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include "linkWidget.hpp"

class LinkTcpWidget
    :   public  LinkWidget
{
    Q_OBJECT

    public:
        LinkTcpWidget(QWidget *a_parent = 0);
        virtual ~LinkTcpWidget();

        void setupUi(QWidget *a_parent);
};

#endif

