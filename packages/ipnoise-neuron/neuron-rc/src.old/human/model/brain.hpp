#include "class.hpp"
DEFINE_CLASS(HumanModelBrain);

#ifndef HUMAN_MODEL_BRAIN_HPP
#define HUMAN_MODEL_BRAIN_HPP

#include <QObject>
#include <QTimer>

#include <vector>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/object/object/main.hpp"

#define MODEL_BRAIN_OBJECT_NAME "model_brain_001"

typedef vector<ObjectSptr>      BrainObjects;
typedef BrainObjects::iterator  BrainObjectsIt;

class HumanModelBrain
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelBrain(HumanViewer *);
        virtual ~HumanModelBrain();

        virtual ObjectSptr init();

        void        process();
        ObjectSptr  devide(ObjectSptr);
        void        align();

    public slots:
       void         timer();

    private:
        ObjectSptr      m_model_object;
        BrainObjects    m_objects;
        QTimer          *m_timer;

        int32_t         m_aligned;
};

#endif

