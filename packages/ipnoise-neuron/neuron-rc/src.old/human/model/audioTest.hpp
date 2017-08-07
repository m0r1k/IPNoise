#include "class.hpp"
DEFINE_CLASS(HumanModelAudioTest);

#ifndef HUMAN_MODEL_AUDIO_TEST_HPP
#define HUMAN_MODEL_AUDIO_TEST_HPP

#include <QObject>

#include "log.hpp"

using namespace std;

#include "utils.hpp"
#include "human/nerves/audio/input.hpp"
#include "human/model.hpp"
#include "core/object/object/main.hpp"

class HumanModelAudioTest
    :   public HumanModel
{
    Q_OBJECT

    public:
        HumanModelAudioTest(HumanViewer *);
        virtual ~HumanModelAudioTest();

        virtual ObjectSptr init();

    public slots:
        void newFrameSlot(HumanNervesAudioFrameSptr);

    private:
};

#endif

