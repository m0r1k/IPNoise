#include "class.hpp"
DEFINE_CLASS(HumanNervesVideo);
DEFINE_CLASS(HumanNervesVideoFrame);

#ifndef HUMAN_NERVES_VIDEO_HPP
#define HUMAN_NERVES_VIDEO_HPP

#include "opencv2/core/core.hpp"

//using namespace cv2;

//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include <QObject>

#include "log.hpp"

#define YUE_WIDTH  120
#define YUE_HEIGHT 100

using namespace std;

DEFINE_CLASS(HumanViewer);

#include "nerves.hpp"

class HumanNervesVideoFrame
    :   public QObject
{
    Q_OBJECT

    public:
        HumanNervesVideoFrame(){}
        HumanNervesVideoFrame(
            const cv2::Mat &a_mat)
        {
            m_mat = a_mat;
        }
        virtual ~HumanNervesVideoFrame(){}

    private:
        cv2::Mat m_mat;
};

class HumanNervesVideo
    :   public HumanNerves
{
    Q_OBJECT

    public:
        HumanNervesVideo(HumanViewer *);
        virtual ~HumanNervesVideo();

//        virtual void        process();
//        virtual void    *   getFrame() = 0;

//    private:
//        float   m_bright_old;
};

#endif

