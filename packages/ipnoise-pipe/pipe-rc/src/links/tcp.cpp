#include "tcp.hpp"

LinkTcpWidget::LinkTcpWidget(QWidget *a_parent)
    :   LinkWidget(
            "tcp_v4",   // name
            "tcp v4.",  // display name
            QIcon(),    // icon
            a_parent
        )
{
    setupUi(this);
}

LinkTcpWidget::~LinkTcpWidget()
{
}

void LinkTcpWidget::setupUi(QWidget *a_parent)
{
}

