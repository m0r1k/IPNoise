#include "class.hpp"
DEFINE_CLASS(HumanNervesVideoTest);

#ifndef HUMAN_NERVES_VIDEO_TEST_HPP
#define HUMAN_NERVES_VIDEO_TEST_HPP

#include <QObject>

#include "log.hpp"
#include "human/nerves/video.hpp"

using namespace std;

DEFINE_CLASS(HumanViewer);

class HumanNervesVideoTest
    :   public  HumanNervesVideo
{
    Q_OBJECT

    public:
        HumanNervesVideoTest(HumanViewer *);
        virtual ~HumanNervesVideoTest();

        void    process();
        void *  getFrame();
};

#endif

