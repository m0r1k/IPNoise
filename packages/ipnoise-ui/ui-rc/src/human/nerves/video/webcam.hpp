#include "class.hpp"
DEFINE_CLASS(HumanNervesVideoWebCam);

#ifndef HUMAN_NERVES_VIDEO_WEBCAM_HPP
#define HUMAN_NERVES_VIDEO_WEBCAM_HPP

#include <QObject>
#include <QTimer>

#include "log.hpp"

struct CvCapture;

#include "human/nerves/video.hpp"

using namespace std;

DEFINE_CLASS(HumanViewer);

class HumanNervesVideoWebCam
    :   public  HumanNervesVideo
{
    Q_OBJECT

    public:
        HumanNervesVideoWebCam(
            HumanViewer *,
            const int32_t &a_cam = 0
        );
        virtual ~HumanNervesVideoWebCam();

        void    process();
        void *  getFrame();

    signals:
        void newFrame(HumanNervesVideoFrameSptr);

    public slots:
        void    timer();

    private:
        CvCapture   *m_capture;
        QTimer      *m_timer;
};

#endif

