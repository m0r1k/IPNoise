#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <unistd.h>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv2;

int main( int argc, const char** argv )
{
    int32_t     preview_x   = 0;
    int32_t     preview_y   = 0;
    CvCapture   *capture    = 0;

    Mat frame, frameCopy, image;

    cvNamedWindow("result", CV_WINDOW_AUTOSIZE);

    // 0=default, -1=any camera, 1..99=your camera
    capture = cvCaptureFromCAM( 0 );
    if (!capture){
        cout << "No camera detected" << endl;
        goto out;
    }

    cout << "In capture ..." << endl;
    for (;;){
        IplImage *iplImg = cvQueryFrame( capture );
        frame = iplImg;

        if (frame.empty()){
            break;
        }

        preview_y = 20;
        for (int32_t j = 0; j < 480; j += 480/64){
            preview_x = 20;
            for (int32_t i = 0; i < 640; i += 640/64){
                Vec3b   bgrPixel = frame.at<Vec3b>(j, i);
                int32_t color    = (bgrPixel[0] + bgrPixel[1] + bgrPixel[2])/3;

                bgrPixel[0] = color;
                bgrPixel[1] = color;
                bgrPixel[2] = color;

                frame.at<Vec3b>(preview_y,      preview_x)      = bgrPixel;
                frame.at<Vec3b>(preview_y,      preview_x + 1)  = bgrPixel;
                frame.at<Vec3b>(preview_y,      preview_x - 1)  = bgrPixel;
                frame.at<Vec3b>(preview_y + 1,  preview_x)      = bgrPixel;
                frame.at<Vec3b>(preview_y + 1,  preview_x + 1)  = bgrPixel;
                frame.at<Vec3b>(preview_y + 1,  preview_x - 1)  = bgrPixel;
                frame.at<Vec3b>(preview_y - 1,  preview_x)      = bgrPixel;
                frame.at<Vec3b>(preview_y - 1,  preview_x + 1)  = bgrPixel;
                frame.at<Vec3b>(preview_y - 1,  preview_x - 1)  = bgrPixel;

/*
                if (color > 170){
                    fprintf(stderr, "color: %d rgb: %2.2x%2.2x%2.2x\n",
                        color,
                        bgrPixel[0],
                        bgrPixel[1],
                        bgrPixel[2]
                    );
                }
*/
                preview_x += 3;
            }
            preview_y += 3;
        }

        //if( iplImg->origin == IPL_ORIGIN_TL ){
        //    frame.copyTo( frameCopy );
        //} else {
        //    flip( frame, frameCopy, 0 );
        //}

        cvShowImage("result", iplImg);

        if ( waitKey( 10 ) >= 0 ){
            cvReleaseCapture(&capture);
            break;
        }

        // microseconds
        usleep (100 * 1000);
    }

    cvDestroyWindow("result");

out:
    return 0;
}

