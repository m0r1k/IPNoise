#include "class.hpp"
DEFINE_CLASS(HumanModelPhysics);

#ifndef HUMAN_MODEL_PHYSICS_HPP
#define HUMAN_MODEL_PHYSICS_HPP

#include <QObject>
#include <QTimer>
#include <vector>

#include "object/vector.hpp"

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/model.hpp"
#include "core/object/object/main.hpp"

#define MODEL_PHYSICS_OBJECT_NAME "model_physics_001"

class HumanModelPhysics
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelPhysics(HumanViewer *);
        virtual ~HumanModelPhysics();

        virtual ObjectSptr init();

        void        process();
        ObjectSptr  devide(ObjectSptr);
        void        align();

    public slots:
       void         timer();

    private:
        ObjectSptr      m_model_object;
        ObjectVectorSptr  m_objects;
        QTimer          *m_timer;

        int32_t         m_aligned;
};

#endif

