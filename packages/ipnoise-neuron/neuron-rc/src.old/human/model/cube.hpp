#include "class.hpp"
DEFINE_CLASS(HumanModelCube);

#ifndef HUMAN_MODEL_CUBE_HPP
#define HUMAN_MODEL_CUBE_HPP

#include <QObject>
#include <QTimer>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/object/object/main.hpp"

#define MODEL_CUBE_OBJECT_NAME       "model_cube_001"
#define MODEL_CUBE_OBJECTS_X         10
#define MODEL_CUBE_OBJECTS_Y         10
#define MODEL_CUBE_OBJECTS_Z         10

typedef map<string, ObjectSptr> CubeObjects;
typedef CubeObjects::iterator   CubeObjectsIt;

class HumanModelCube
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelCube(HumanViewer *);
        virtual ~HumanModelCube();

        virtual ObjectSptr init();

        void    process();

    public slots:
       void     timer();

    private:
        ObjectSptr      m_model_object;
        CubeObjects     m_objects;
        QTimer          *m_timer;

        int32_t         m_x;
        int32_t         m_y;
        int32_t         m_z;
};

#endif

