#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

//#include "object_neuron.hpp"
//#include "viewer.hpp"

#include "video.hpp"

using namespace cv2;

//Mat g_old_frame;

// 2560x1920
// 1280x960
// 640x480
// 320x240
// 160x120
// 80x60
// 40x30

HumanNervesVideo::HumanNervesVideo(
    HumanViewer *a_human_viewer)
    :   HumanNerves(a_human_viewer)
{
//    m_bright_old = 0.0f;
}

HumanNervesVideo::~HumanNervesVideo()
{
}

