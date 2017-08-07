#include <QMetaObject>
#include <QSet>

#include "human/viewer.hpp"
#include "human/nerves/video.hpp"
#include "human/nerves/video/webcam.hpp"
#include "log.hpp"

#include "sphere.hpp"

HumanModelSphere::HumanModelSphere(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
}

HumanModelSphere::~HumanModelSphere()
{
}

ObjectSptr HumanModelSphere::init()
{
    char        buffer[1024]    = { 0x00 };
    int32_t     row             = 0;
    int32_t     col             = 0;
    int32_t     max             = YUE_HEIGHT * YUE_WIDTH;
    ObjectSptr  object;

    HumanViewer         *viewer     = getHumanViewer();
    HumanNervesVideo    *video      = viewer->getHumanNervesVideo();

    if (!video){
        PERROR("Cannot get video nerves\n");
        goto fail;
    }

    object = ThreadDb::get()->getObjectById(ObjectIdSptr(
        new ObjectId(
            "sphere2"
        )
    ));
    if (!object){
        max++;
        object = ThreadMain::get()->createObject();
        object->setId(ObjectIdSptr(
            new ObjectId(
                "sphere2"
            )
        ));

        PWARN("will created: '%d' objects\n", max);

        for (row = 0; row < YUE_HEIGHT; row++){
            for (col = 0; col < YUE_WIDTH; col++){
                ObjectSptr new_object;
                new_object = ThreadMain::get()->createObject<Object>();
                snprintf(buffer, sizeof(buffer),
                    "eye.%d.%d",
                    col,
                    row
                );
                new_object->setHumanNervePath(ObjectStringSptr(
                    new ObjectString(buffer)
                ));
                LINK_AND_SAVE(object.get(), new_object.get());
            }
        }
    } else if (0){
        // get neighs
        vector<ObjectSptr>              neighs;
        vector<ObjectSptr>::iterator    neighs_it;

        object->getNeighs<Object>(neighs);
        for (neighs_it = neighs.begin();
            neighs_it != neighs.end();
            neighs_it++)
        {
            ObjectSptr      cur_object = *neighs_it;
            ObjectStringSptr  nerve_path;

            nerve_path = cur_object->getHumanNervePath();
            if (!nerve_path->toString().size()){
                continue;
            }
        }
    }

    CHECKED_CONNECT(
        video,  SIGNAL(imageRaw(HumanNervesVideoFrameSptr)),
        this,   SLOT(imageRawSlot(HumanNervesVideoFrameSptr))
    );

out:
    return object;
fail:
    goto out;
}

void HumanModelSphere::newFrameSlot(
    HumanNervesVideoFrameSptr)
{
    PWARN("HumanModelSphere::newFrameSlot\n");
}

