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

NeuronSptr HumanModelSphere::init()
{
    char        buffer[1024]    = { 0x00 };
    int32_t     row             = 0;
    int32_t     col             = 0;
    int32_t     max             = YUE_HEIGHT * YUE_WIDTH;
    DbThread    *db             = getDbThread();
    NeuronSptr  neuron;

    HumanViewer         *viewer     = getHumanViewer();
    HumanNervesVideo    *video      = viewer->getHumanNervesVideo();

    if (!video){
        PERROR("Cannot get video nerves\n");
        goto fail;
    }

    neuron = db->getNeuronById(PropNeuronIdSptr(
        new PropNeuronId(
            "sphere2"
        )
    ));
    if (!neuron){
        max++;
        neuron = getNetwork()->createNeuron();
        neuron->setId(PropNeuronIdSptr(
            new PropNeuronId(
                "sphere2"
            )
        ));

        PWARN("will created: '%d' neurons\n", max);

        for (row = 0; row < YUE_HEIGHT; row++){
            for (col = 0; col < YUE_WIDTH; col++){
                NeuronSptr new_neuron;
                new_neuron = getNetwork()->createNeuron<Neuron>();
                snprintf(buffer, sizeof(buffer),
                    "eye.%d.%d",
                    col,
                    row
                );
                new_neuron->setHumanNervePath(PropStringSptr(
                    new PropString(buffer)
                ));
                LINK_AND_SAVE(neuron.get(), new_neuron.get());
            }
        }
    } else if (0){
        // get neighs
        vector<NeuronSptr>              neighs;
        vector<NeuronSptr>::iterator    neighs_it;

        neuron->getNeighs<Neuron>(neighs);
        for (neighs_it = neighs.begin();
            neighs_it != neighs.end();
            neighs_it++)
        {
            NeuronSptr      cur_neuron = *neighs_it;
            PropStringSptr  nerve_path;

            nerve_path = cur_neuron->getHumanNervePath();
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
    return neuron;
fail:
    goto out;
}

void HumanModelSphere::newFrameSlot(
    HumanNervesVideoFrameSptr)
{
    PWARN("HumanModelSphere::newFrameSlot\n");
}

