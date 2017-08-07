#include "human/viewer.hpp"
#include "human/nerves/video.hpp"
#include "human/nerves/video/webcam.hpp"

#include <QMetaObject>
#include <QSet>

#include "audioTest.hpp"

HumanModelAudioTest::HumanModelAudioTest(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
}

HumanModelAudioTest::~HumanModelAudioTest()
{
}

NeuronSptr HumanModelAudioTest::init()
{
    NeuronSptr neuron;

    PWARN("MORIK Human model audio test init\n");

    HumanViewer         *viewer     = getHumanViewer();
    HumanNervesAudio    *audio      = viewer->getHumanNervesAudio();

    CHECKED_CONNECT(
        audio,  SIGNAL(newFrame(HumanNervesAudioFrameSptr)),
        this,   SLOT(newFrameSlot(HumanNervesAudioFrameSptr))
    );

    return neuron;
}

void HumanModelAudioTest::newFrameSlot(
    HumanNervesAudioFrameSptr)
{
    PWARN("HumanModelAudioTest::newFrameSlot\n");
}

