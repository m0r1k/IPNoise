#include "udp.hpp"

LinkUdpWidget::LinkUdpWidget(QWidget *a_parent)
    :   LinkWidget(
            "udp_v4",   // name
            "udp v4.",  // display name
            QIcon(),    // icon
            a_parent
        )
{
    setupUi(this);
}

LinkUdpWidget::~LinkUdpWidget()
{
}

void LinkUdpWidget::setupUi(QWidget *a_parent)
{
}

