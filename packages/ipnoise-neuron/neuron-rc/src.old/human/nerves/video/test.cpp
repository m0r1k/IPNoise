#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "object_object.hpp"
#include "viewer.hpp"

#include "test.hpp"

using namespace cv2;

int32_t g_pos = 0;

HumanNervesVideoTest::HumanNervesVideoTest(
    HumanViewer *a_human_viewer)
    :   HumanNervesVideo(a_human_viewer)
{
    cout << "In capture ..." << endl;
}

HumanNervesVideoTest::~HumanNervesVideoTest()
{
}

void HumanNervesVideoTest::process()
{
}

void * HumanNervesVideoTest::getFrame()
{
    int32_t     i, j, pos;
    IplImage    *iplImg = NULL;
    Mat         frame;
    int32_t     test = 4;

    iplImg = cvCreateImage(
        cvSize(YUE_WIDTH, YUE_HEIGHT),  // size
        IPL_DEPTH_8U,                   // depth
        3                               // channels
    );

    frame = iplImg;
    pos   = 0;

    for (j = 0; j < iplImg->height; j += iplImg->height/YUE_HEIGHT){
        for (i = 0; i < iplImg->width; i += iplImg->width/YUE_WIDTH){
            Vec3b bgrPixel;

            switch (test){
                case 4:
                    if (    pos == g_pos
                        ||  pos == (YUE_HEIGHT * YUE_WIDTH - g_pos))
                    {
                        bgrPixel[0] = 0xff;
                        bgrPixel[1] = 0xff;
                        bgrPixel[2] = 0xff;
                    }
                    break;

                case 3:
                    if (    pos == g_pos
                        ||  pos == (g_pos + YUE_WIDTH/3))
                    {
                        bgrPixel[0] = 0xff;
                        bgrPixel[1] = 0xff;
                        bgrPixel[2] = 0xff;
                    }
                    break;

                case 2:
                    if (    pos == g_pos
                        ||  pos == (g_pos + YUE_WIDTH/2))
                    {
                        bgrPixel[0] = 0xff;
                        bgrPixel[1] = 0xff;
                        bgrPixel[2] = 0xff;
                    }
                    break;

                case 1:
                    if (pos == g_pos){
                        bgrPixel[0] = 0xff;
                        bgrPixel[1] = 0xff;
                        bgrPixel[2] = 0xff;
                    }
                    break;

                default:
                    PERROR("no such test: '%d'\n", test);
                    bgrPixel[0] = 0x00;
                    bgrPixel[1] = 0x00;
                    bgrPixel[2] = 0x00;
                    break;
             }

            frame.at<Vec3b>(j, i) = bgrPixel;

            pos++;
        }
    }

    g_pos++;
    if (g_pos >= (iplImg->height * iplImg->width)){
        g_pos = 0;
    }

    return (void *)iplImg;
}

