#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "object_object.hpp"
#include "viewer.hpp"

#include "webcam.hpp"

using namespace cv2;

HumanNervesVideoWebCam::HumanNervesVideoWebCam(
    HumanViewer     *a_human_viewer,
    const int32_t   &a_cam)
    :   HumanNervesVideo(a_human_viewer)
{
    //  0       = default,
    //  -1      = any camera,
    //  1..99   = your camera
    m_capture = cvCaptureFromCAM(a_cam);
    if (m_capture){
        // create window
        cvNamedWindow("result", CV_WINDOW_AUTOSIZE);

        cout << "In capture ..." << endl;
        // start capture timer
        m_timer = new QTimer();
        m_timer->setInterval(100);
        connect(
            m_timer, SIGNAL(timeout()),
            this,    SLOT(timer())
        );
        m_timer->start();
    } else {
        PERROR("No camera detected\n");
    }
}

HumanNervesVideoWebCam::~HumanNervesVideoWebCam()
{
    if (m_capture){
        cvReleaseCapture(&m_capture);
        m_capture = NULL;
    }

    cvDestroyWindow("result");
}

void HumanNervesVideoWebCam::timer()
{
    process();
}

void * HumanNervesVideoWebCam::getFrame()
{
    IplImage *iplImg = NULL;

    if (m_capture){
        iplImg = cvQueryFrame(m_capture);
    }

    return (void *)iplImg;
}

void HumanNervesVideoWebCam::process()
{
//    int32_t     j, k, row, col;
//    float       total_red       = 0.0f;
//    float       total_green     = 0.0f;
//    float       total_blue      = 0.0f;
//    float       avg_red         = 0.0f;
//    float       avg_green       = 0.0f;
//    float       avg_blue        = 0.0f;
//    float       red_in_frame    = 0.0f;
//    float       green_in_frame  = 0.0f;
//    float       blue_in_frame   = 0.0f;
//    float       bright          = 0.0f;
//    float       bright_delta    = 0.0f;
    IplImage    *img_in         = NULL;
//    IplImage    *img_out        = NULL;
    Mat         frame_in, frame_out, image;
//    UiObjectsIt it;

    // get image
    img_in = (IplImage *)getFrame();
    if (!img_in){
        PERROR("Cannot get frame\n");
        goto out;
    }

    frame_in = img_in;
    if (frame_in.empty()){
        goto out;
    }

    cvShowImage("result", img_in);

    emit newFrame(
        HumanNervesVideoFrameSptr(
            new HumanNervesVideoFrame(frame_in)
        )
    );

//    frame_in = img_in;
//    if (frame_in.empty()){
//        goto out;
//    }

    // prepare output
//    img_out = cvCloneImage(img_in);

//    frame_out = img_out;
//    if (frame_out.empty()){
//        goto out;
//    }

    // goto first object
/*
    if (0){
        UiObjectObjectSptr  object;
        float               xx, yy, zz;
        object = getViewer()->m_objects_objects[0];
        object->m_frame.getPosition(xx, yy, zz);
        m_viewer->m_object_object->m_frame.setPosition(
            xx, yy, zz
        );
    }

    // goto center
    if (1){
        m_viewer->m_object_object->m_frame.setPosition(
            0, 0, 0
        );
    }

    // calculate total charge
    m_viewer->m_total_charge = 0.0f;
    for (j = 0; j < img_in->height; j += img_in->height/YUE_HEIGHT){
        for (k = 0; k < img_in->width; k += img_in->width/YUE_WIDTH){
            Vec3b   bgrPixel    = frame_in.at<Vec3b>(j, k);
            float   color       = (
                bgrPixel[0]/255.0f + bgrPixel[1]/255.0f + bgrPixel[2]/255.0f
            )/3;
            m_viewer->m_total_charge += color;
        }
    }
*/

    // preview
/*
    if (1){
        int32_t preview_x           = 0;
        int32_t preview_y           = 0;
        float   preview_step_by_x   = 0;
        float   preview_step_by_y   = 0;

        preview_step_by_x = img_in->width  / YUE_WIDTH;
        preview_step_by_y = img_in->height / YUE_HEIGHT;

        preview_y = 20;
        for (j = 0, row = 0;
            row < YUE_HEIGHT;
            j += preview_step_by_y, row++)
        {
            preview_x = 20;
            for (k = 0, col = 0;
                col < YUE_WIDTH;
                k += preview_step_by_x, col++)
            {
                Vec3b       bgrPixel    = frame_in.at<Vec3b>(j, k);
                float       red         = bgrPixel[2]/255.0f;
                float       green       = bgrPixel[1]/255.0f;
                float       blue        = bgrPixel[0]/255.0f;
                float       color       = (red + green + blue)/3;
//                float       charge      = color;
//                uint32_t    index       = (row << 16) + col;

                UiObjectObjectSptr object;

                total_red   += red;
                total_green += green;
                total_blue  += blue;

                if (0){
                    // 70 70 60
                    if (red > 0xf0){
                        PWARN("red: '%f'\n", red);
                    }
                    if (green > 0xf0){
                        PWARN("green: '%f'\n", green);
                    }
                    if (blue > 0xf0){
                        PWARN("blue: '%f'\n", blue);
                    }

                    //            PWARN("red: 0x%2.2x, green: 0x%2.2x, blue: 0x%2.2x\n",
                    //                red, green, blue
                    //            );
                }

                if (0 && !g_old_frame.empty()){
                    Vec3b   old_bgrPixel = g_old_frame.at<Vec3b>(j, k);
                    float   old_color    = (
                        old_bgrPixel[0]/255.0f + old_bgrPixel[1]/255.0f + old_bgrPixel[2]/255.0f
                    )/3;

                    if (old_color == color){
                        // not changed, next
                        continue;
                    }

//                   if (!morik && 0){
//                       morik = 1;
//                       float xx, yy, zz;
//                       object = getViewer()->m_objects_objects[index];
//                       object->m_frame.getPosition(xx, yy, zz);
//                       object->m_charge = charge;
//                       m_viewer->m_object_object->m_frame.setPosition(
//                           xx, yy, zz
//                       );
//                       continue;
//                   }

                }

                bgrPixel[0] = color*255;
                bgrPixel[1] = color*255;
                bgrPixel[2] = color*255;

                if (1){
                    frame_out.at<Vec3b>(preview_y,      preview_x)      = bgrPixel;
                    frame_out.at<Vec3b>(preview_y,      preview_x + 1)  = bgrPixel;
                    frame_out.at<Vec3b>(preview_y,      preview_x - 1)  = bgrPixel;
                    frame_out.at<Vec3b>(preview_y + 1,  preview_x)      = bgrPixel;
                    frame_out.at<Vec3b>(preview_y + 1,  preview_x + 1)  = bgrPixel;
                    frame_out.at<Vec3b>(preview_y + 1,  preview_x - 1)  = bgrPixel;
                    frame_out.at<Vec3b>(preview_y - 1,  preview_x)      = bgrPixel;
                    frame_out.at<Vec3b>(preview_y - 1,  preview_x + 1)  = bgrPixel;
                    frame_out.at<Vec3b>(preview_y - 1,  preview_x - 1)  = bgrPixel;
                }

//                object = getViewer()->m_objects_objects[index];
//                object->setCharge(charge);

                preview_x += 3;
            }
            preview_y += 3;
        }
    }

    avg_red     = total_red/float(YUE_WIDTH * YUE_HEIGHT);
    avg_green   = total_green/float(YUE_WIDTH * YUE_HEIGHT);
    avg_blue    = total_blue/float(YUE_WIDTH * YUE_HEIGHT);

    red_in_frame    = float(avg_red)/(float(avg_red) + float(avg_green) + float(avg_blue));
    green_in_frame  = float(avg_green)/(float(avg_red) + float(avg_green) + float(avg_blue));
    blue_in_frame   = float(avg_blue)/(float(avg_red) + float(avg_green) + float(avg_blue));

//    bright          = (avg_red + avg_green + avg_blue)/3;
//    bright_delta    = (m_bright_old > bright)
//        ?   (m_bright_old - bright)
//        :   (bright - m_bright_old);

    PDEBUG(20, "avg_red: '%f', avg_green: '%f', avg_blue: '%f'\n",
        avg_red,
        avg_green,
        avg_blue
    );

    PDEBUG(20, "red_in_frame: '%f', green_in_frame: '%f', blue_in_frame: '%f'\n",
        red_in_frame,
        green_in_frame,
        blue_in_frame
    );
*/

/*
    if (bright_delta > 0){
        float               ax, ay, az;
        UiObjectObjectSptr  memory;
        getViewer()->m_object_object->m_frame.getPosition(
            ax, ay, az
        );
        memory = UiObjectObjectSptr(new UiObjectObject(
            getViewer()
        ));
        memory->m_frame.setPosition(
            qglviewer::Vec(ax, ay, az)
        );

        {
            ColorSptr color = ColorSptr(new Color);
            color->push_back(bright);
            color->push_back(bright);
            color->push_back(bright);
            memory->m_color = color;
        }

        getViewer()->m_objects.push_back(memory);
    }
*/

/*
    // viewback
    if (0){
        UiObjectsObjectsIt  it;
        UiObjectObjectSptr  object;
        float               ax, ay, az;

        object = getViewer()->m_object_object;
        object->m_frame.getPosition(
            ax, ay, az
        );

        for (it = getViewer()->m_objects_objects.begin();
            it != getViewer()->m_objects_objects.end();
            it++)
        {
            float               bx, by, bz;
            UiObjectObjectSptr  cur_object = it->second;

            row = cur_object->m_object->morik_row;
            col = cur_object->m_object->morik_col;

            cur_object->m_frame.getPosition(
                bx, by, bz
            );

            float len = sqrtf((bx-ax)*(bx-ax) + (by-ay)*(by-ay)
                + (bz-az)*(bz-az)
            );

            Vec3b       bgrPixel;
            int32_t     color = 255 - len*200;
            if (color < 0){
                color = 0;
            }
            ObjectSptr  object = cur_object->m_object;
            int32_t     preview_y = 220 + object->morik_row*3;
            int32_t     preview_x = 20  + object->morik_col*3;

            bgrPixel[0] = color*255;
            bgrPixel[1] = color*255;
            bgrPixel[2] = color*255;

            if (1){
                frame_out.at<Vec3b>(preview_y,      preview_x)      = bgrPixel;
                frame_out.at<Vec3b>(preview_y,      preview_x + 1)  = bgrPixel;
                frame_out.at<Vec3b>(preview_y,      preview_x - 1)  = bgrPixel;
                frame_out.at<Vec3b>(preview_y + 1,  preview_x)      = bgrPixel;
                frame_out.at<Vec3b>(preview_y + 1,  preview_x + 1)  = bgrPixel;
                frame_out.at<Vec3b>(preview_y + 1,  preview_x - 1)  = bgrPixel;
                frame_out.at<Vec3b>(preview_y - 1,  preview_x)      = bgrPixel;
                frame_out.at<Vec3b>(preview_y - 1,  preview_x + 1)  = bgrPixel;
                frame_out.at<Vec3b>(preview_y - 1,  preview_x - 1)  = bgrPixel;
            }
        }
    }
*/

//    frame_in.copyTo(g_old_frame);

//    cvReleaseImage(&img_in);
//    cvReleaseImage(&img_out);

out:
    return;
}

