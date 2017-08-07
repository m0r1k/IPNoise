#include "human/viewer.hpp"

#include <QMetaObject>
#include <QSet>

#include "cube.hpp"

HumanModelCube::HumanModelCube(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    assert(MODEL_CUBE_OBJECTS_X > 0);
    assert(MODEL_CUBE_OBJECTS_Y > 0);
    assert(MODEL_CUBE_OBJECTS_Z > 0);

    // init pos
    m_x = 0;
    m_y = 0;
    m_z = 0;

    // init timer
    m_timer = new QTimer();
    m_timer->setInterval(100);
    connect(
        m_timer, SIGNAL(timeout()),
        this,    SLOT(timer())
    );
}

HumanModelCube::~HumanModelCube()
{
}

ObjectSptr HumanModelCube::init()
{
    char        buffer[512]     = { 0x00 };
    double      step            = .05f;
    double      ax              = .0f;
    double      ay              = .0f;
    double      az              = .0f;
    int32_t     x, y, z;

    m_model_object = ThreadDb::get()->getObjectById(ObjectIdSptr(
        new ObjectId(
            MODEL_CUBE_OBJECT_NAME
        )
    ));
    if (!m_model_object){
        m_model_object = ThreadMain::get()->createObject();
        m_model_object->setId(ObjectIdSptr(
            new ObjectId(
                MODEL_CUBE_OBJECT_NAME
            )
        ));

        ax = .0f;
        for (x = 0; x < MODEL_CUBE_OBJECTS_X; x++){
            ay = .0f;
            for (y = 0; y < MODEL_CUBE_OBJECTS_Y; y++){
                az = .0f;
                for (z = 0; z < MODEL_CUBE_OBJECTS_Z; z++){
                    ObjectSptr  new_object;
                    string      nerve_path;

                    new_object = ThreadMain::get()->createObject<Object>();

                    // set position
                    new_object->setPosX(ObjectDoubleSptr(
                        new ObjectDouble(
                            ax
                        )
                    ));
                    new_object->setPosY(ObjectDoubleSptr(
                        new ObjectDouble(
                            ay
                        )
                    ));
                    new_object->setPosZ(ObjectDoubleSptr(
                        new ObjectDouble(
                            az
                        )
                    ));

                    // create nerve path
                    snprintf(buffer, sizeof(buffer),
                       "%d_%d_%d",
                        x,
                        y,
                        z
                    );

                    nerve_path = buffer;
                    new_object->setHumanNervePath(ObjectStringSptr(
                        new ObjectString(nerve_path)
                    ));

                    LINK(m_model_object.get(), new_object.get());

                    // store to objects by columns
                    m_objects[nerve_path] = new_object;

                    az += step;
                }
                ay += step;
            }
            ax += step;
        }
    }

    // start timer
    m_timer->start();

    return m_model_object;
}

void HumanModelCube::timer()
{
    process();
}

void HumanModelCube::process()
{
    char            buffer[512] = { 0x00};
    CubeObjectsIt  it;
    ObjectSptr      cur_object;
    string          nerve_path;

    // dim cur pos
    snprintf(buffer, sizeof(buffer),
        "%d_%d_%d",
        m_x,
        m_y,
        m_z
    );
    nerve_path = buffer;
    it = m_objects.find(nerve_path);
    if (m_objects.end() != it){
        cur_object = it->second;
        cur_object->setHumanVal(ObjectDoubleSptr(
            new ObjectDouble(0.0f)
        ));
    }

    // calculate next pos
    m_x++;
    if (MODEL_CUBE_OBJECTS_X == m_x){
        m_x = 0;
        m_y++;
    }
    if (MODEL_CUBE_OBJECTS_Y == m_y){
        m_y = 0;
        m_z++;
    }
    if (MODEL_CUBE_OBJECTS_Z == m_z){
        m_z = 0;
    }

    // highlight new pos
    snprintf(buffer, sizeof(buffer),
        "%d_%d_%d",
        m_x,
        m_y,
        m_z
    );
    nerve_path = buffer;
    it = m_objects.find(nerve_path);
    if (m_objects.end() != it){
        cur_object = it->second;
        cur_object->setHumanVal(ObjectDoubleSptr(
            new ObjectDouble(1.0f)
        ));
    }
}

