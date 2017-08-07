#include "class.hpp"
DEFINE_CLASS(HumanModelAudioFlame);

#ifndef HUMAN_MODEL_AUDIO_FLAME_HPP
#define HUMAN_MODEL_AUDIO_FLAME_HPP

#include <QObject>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/object/object/main.hpp"

#define MODEL_AUDIO_FLAME_OBJECT_NAME       "model_audio_flame_002"
#define MODEL_AUDIO_FLAME_HZ_PER_OBJECT     50
#define MODEL_AUDIO_FLAME_UP_HZ             4000
#define MODEL_AUDIO_FLAME_ROWS_COUNT        10

typedef map<string, ObjectSptr>     AudioFlameObjects;
typedef AudioFlameObjects::iterator AudioFlameObjectsIt;

class HumanModelAudioFlame
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelAudioFlame(HumanViewer *);
        virtual ~HumanModelAudioFlame();

        virtual ObjectSptr init();

    public slots:
        void newFrameSlot(HumanNervesAudioFrameSptr);

    private:
        AudioFlameObjects m_objects;
};

#endif

