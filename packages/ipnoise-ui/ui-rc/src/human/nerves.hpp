#include "class.hpp"
DEFINE_CLASS(HumanNerves);

#ifndef HUMAN_NERVES_HPP
#define HUMAN_NERVES_HPP

#include <QObject>
#include <QWidget>

#include "log.hpp"

using namespace std;

DEFINE_CLASS(HumanViewer);

class HumanNerves
    :   public QWidget
{
    Q_OBJECT

    public:
        HumanNerves(
            HumanViewer *,
            QWidget     *a_parent = 0
        );
        virtual ~HumanNerves();

    private:
        HumanViewer *m_human_viewer;
};

#endif

