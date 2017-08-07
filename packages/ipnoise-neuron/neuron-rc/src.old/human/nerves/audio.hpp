#include "class.hpp"
DEFINE_CLASS(HumanNervesAudio);

#ifndef HUMAN_NERVES_AUDIO_HPP
#define HUMAN_NERVES_AUDIO_HPP

#include "opencv2/core/core.hpp"

#include <QObject>

#include "log.hpp"

using namespace std;

DEFINE_CLASS(HumanViewer);

#include "nerves.hpp"

class HumanNervesAudio
    :   public HumanNerves
{
    Q_OBJECT

    public:
        HumanNervesAudio(
            HumanViewer *,
            QWidget     *a_parent = 0
        );
        virtual ~HumanNervesAudio();
};

#endif

