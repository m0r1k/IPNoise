#include "nerves.hpp"

HumanNerves::HumanNerves(
    HumanViewer *a_human_viewer,
    QWidget     *a_parent)
    :   QWidget(a_parent)
{
    m_human_viewer = a_human_viewer;
}

HumanNerves::~HumanNerves()
{
}

