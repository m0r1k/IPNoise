#include "class.hpp"
DEFINE_CLASS(HumanModel);

#ifndef HUMAN_MODEL_HPP
#define HUMAN_MODEL_HPP

#include <stdint.h>

#include <QObject>

#include "log.hpp"

using namespace std;

//DEFINE_CLASS(UiViewer);
DEFINE_CLASS(HumanViewer);
DEFINE_CLASS(DbThread);
DEFINE_CLASS(Object);

#include "ui/viewer.hpp"
//#include "core/object/object/main.hpp"

#define UI_VIWER_OBJECT_SPHERE_SIZE 0.01f

class HumanModel
    :   public QObject
{
    Q_OBJECT

    public:
        HumanModel(HumanViewer *);
        virtual ~HumanModel();

        virtual ObjectSptr init();

        HumanViewer * getHumanViewer();
        UiViewer    * getUiViewer();

        void getNeighObjects(
            ObjectSptr          a_object,
            vector<ObjectSptr>  &a_out,
            double              a_radius = UI_VIWER_OBJECT_SPHERE_SIZE
        );

    private:
        HumanViewer *m_human_viewer;
};

#endif

