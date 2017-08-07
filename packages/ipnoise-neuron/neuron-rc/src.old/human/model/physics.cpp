#include "human/viewer.hpp"
#include "ui/viewer.hpp"
#include "object/object.hpp"
#include "utils.hpp"

#include <QMetaObject>
#include <QSet>

#include "physics.hpp"

HumanModelPhysics::HumanModelPhysics(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    m_aligned = 0;

    // start timer
    m_timer = new QTimer();
    m_timer->setInterval(1000);
    connect(
        m_timer, SIGNAL(timeout()),
        this,    SLOT(timer())
    );

    m_objects = OBJECT_VECTOR();
}

HumanModelPhysics::~HumanModelPhysics()
{
}

ObjectSptr HumanModelPhysics::init()
{
    m_model_object = ThreadDb::get()->getObjectById(OBJECT_ID(
            MODEL_PHYSICS_OBJECT_NAME
    ));
    if (!m_model_object){
        m_model_object = ThreadMain::get()->createObject();
        m_model_object->setId(OBJECT_ID(
            MODEL_PHYSICS_OBJECT_NAME
        ));

        ObjectSptr new_object;

        // create new object
        //new_object = ThreadMain::get()->createObject();
        //new_object->setPos(
        //    0*UI_VIWER_OBJECT_SPHERE_SIZE,
        //    0*UI_VIWER_OBJECT_SPHERE_SIZE,
        //    0*UI_VIWER_OBJECT_SPHERE_SIZE
        //);
        //new_object->setHumanVal(0.5f);
        //LINK_AND_SAVE(m_model_object.get(), new_object.get());
        //m_objects.push_back(new_object);

        double x = 0.0f;
        double y = 0.0f;
        double z = 0.0f;

        for (x = -0.5f; x < 0.5f; x += 0.1f){
//            for (y = -0.5f; y < 0.5f; y += 0.1f){
//                for (z = -0.5f; z < 0.5f; z += 0.1f){
                    // create new object
                    new_object = ThreadMain::get()->createObject();
                    new_object->setPos(x, y, z);
                    LINK(m_model_object.get(), new_object.get());
                    m_objects->add(OBJECT(new_object));
//                }
//            }
        }
        getUiViewer()->setObjects(m_objects);
    }

    // start timer
    m_timer->start();

    return m_model_object;
}

void HumanModelPhysics::timer()
{
/*
    PhysicsObjectsIt  it;
    PhysicsObjects    cur_objects = m_objects;

    for (it = cur_objects.begin();
        it != cur_objects.end();
        it++)
    {
        ObjectSptr cur_object = *it;
        devide(cur_object);
    }

    PWARN("MORIK m_objects.size: '%ld'\n",
        m_objects.size()
    );

    align();
*/
    return;
}

