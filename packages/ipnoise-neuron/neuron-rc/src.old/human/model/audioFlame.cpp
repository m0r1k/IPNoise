#include "human/viewer.hpp"
//#include "human/nerves/video.hpp"
//#include "human/nerves/video/webcam.hpp"

#include <QMetaObject>
#include <QSet>

#include "audioFlame.hpp"

HumanModelAudioFlame::HumanModelAudioFlame(
    HumanViewer *a_human_viewer)
    :   HumanModel(a_human_viewer)
{
    assert(MODEL_AUDIO_FLAME_HZ_PER_OBJECT > 0);
}

HumanModelAudioFlame::~HumanModelAudioFlame()
{
}

ObjectSptr HumanModelAudioFlame::init()
{
    HumanViewer      *human_viewer  = getHumanViewer();
    HumanNervesAudio *audio         = NULL;
    int32_t          col            = 0;
    char             buffer[512]    = { 0x00 };
    ObjectSptr       object;

    double  step    = 1.0f/(MODEL_AUDIO_FLAME_UP_HZ/MODEL_AUDIO_FLAME_HZ_PER_OBJECT);
    double  ax      = .0f;
    double  ay      = .0f;
    double  az      = .0f;
    int32_t row     = 0;

    audio = human_viewer->getHumanNervesAudio();

    if (!audio){
        PERROR("Cannot get audio nerves\n");
        goto fail;
    }

    object = ThreadDb::get()->getObjectById(ObjectIdSptr(
        new ObjectId(
            MODEL_AUDIO_FLAME_OBJECT_NAME
        )
    ));
    if (!object){
        object = ThreadMain::get()->createObject();
        object->setId(ObjectIdSptr(
            new ObjectId(
                MODEL_AUDIO_FLAME_OBJECT_NAME
            )
        ));

        for (row = 0; row < MODEL_AUDIO_FLAME_ROWS_COUNT; row++){
            ax = -step*(MODEL_AUDIO_FLAME_UP_HZ/MODEL_AUDIO_FLAME_HZ_PER_OBJECT)/2;
            for (col = 0;
                col < MODEL_AUDIO_FLAME_UP_HZ;
                col += MODEL_AUDIO_FLAME_HZ_PER_OBJECT)
            {
                ObjectSptr  new_object;
                string      col_name;
                string      nerve_path;

                int32_t low  = col;
                int32_t high = col + MODEL_AUDIO_FLAME_HZ_PER_OBJECT - 1;

                new_object = ThreadMain::get()->createObject<Object>();

                new_object->setPosX(ObjectDoubleSptr(
                    new ObjectDouble(
                        ax
                    )
                ));
                new_object->setPosY(ObjectDoubleSptr(
                    new ObjectDouble(
                        ay
                    )
                ));
                new_object->setPosZ(ObjectDoubleSptr(
                    new ObjectDouble(
                        az
                    )
                ));
                ax += step;
                az = row * 0.05f;

                // create col name
                snprintf(buffer, sizeof(buffer),
                   "%d_%d_%d",
                    row,
                    low,
                    high
                );
                col_name = buffer;

                // create nerve path
                snprintf(buffer, sizeof(buffer),
                   "ear_%d_%d_%d",
                    row,
                    low,
                    high
                );
                nerve_path = buffer;
                new_object->setHumanNervePath(ObjectStringSptr(
                    new ObjectString(nerve_path)
                ));

                LINK(object.get(), new_object.get());

                // store to objects by columns
                m_objects[col_name] = new_object;
            }
        }
    }

    CHECKED_CONNECT(
        audio,  SIGNAL(newFrame(HumanNervesAudioFrameSptr)),
        this,   SLOT(newFrameSlot(HumanNervesAudioFrameSptr))
    );

out:
    return object;
fail:
    goto out;
}

void HumanModelAudioFlame::newFrameSlot(
    HumanNervesAudioFrameSptr a_frame)
{
    FrequencySpectrum::const_iterator spectrum_it;

    FrequencySpectrum   spectrum    = a_frame->getSpectrum();
    char                buffer[512] = { 0x00};

    ObjectMapSptr         vals_by_cols(new ObjectMap);
    ObjectMapSptr         results(new ObjectMap);
    ObjectMapIterator     map_it;
    AudioFlameObjectsIt objects_it;
    int32_t             row = 0;

    for (spectrum_it = spectrum.begin();
        spectrum_it != spectrum.end();
        spectrum_it++)
    {
        qreal frequency = spectrum_it->frequency;
        qreal amplitude = spectrum_it->amplitude;

        if (    !frequency
            ||  !amplitude)
        {
            continue;
        }

 //       PWARN("MORIK"
 //           " frequency: '%.0f',"
 //           " amplitude: '%f'"
 //           "\n",
 //           frequency,
 //           amplitude
 //       );

        ObjectVectorSptr  vector;
        string          col_name;

        int32_t col     = frequency/MODEL_AUDIO_FLAME_HZ_PER_OBJECT;
        int32_t low     = MODEL_AUDIO_FLAME_HZ_PER_OBJECT * col;
        int32_t high    = MODEL_AUDIO_FLAME_HZ_PER_OBJECT * (col + 1) - 1;

        // calculate colname
        snprintf(buffer, sizeof(buffer),
            "%d_%d",
            low,
            high
        );
        col_name = buffer;

        // get values vector by col name
        if (vals_by_cols->has(col_name)){
            // get array
            vector = dynamic_pointer_cast<ObjectVector>(
                vals_by_cols->get(col_name)
            );
        } else {
            // create array
            vector = OBJECT_VECTOR();
            vals_by_cols->add(col_name, vector);
        }

        // add value to vector
        ObjectDoubleSptr val(new ObjectDouble(amplitude));
        vector->add(val);
    }

    // clear current vals
    for (objects_it = m_objects.begin();
        objects_it != m_objects.end();
        objects_it++)
    {
        ObjectSptr object = objects_it->second;
        object->setHumanVal(ObjectDoubleSptr(
            new ObjectDouble
        ));
    }

    // calculate average amplitude
    for (map_it = vals_by_cols->begin();
        map_it != vals_by_cols->end();
        map_it++)
    {
        ObjectSptr    key         = map_it->first;
        ObjectSptr    val         = map_it->second;
        string      col_name    = key->toString();
        double      amplitude   = 0.f;

        ObjectVectorSptr      vector;
        ObjectVectorIterator  vector_it;

        vector = dynamic_pointer_cast<ObjectVector>(val);
        for (vector_it = vector->begin();
            vector_it != vector->end();
            vector_it++)
        {
            ObjectDoubleSptr value;
            value = dynamic_pointer_cast<ObjectDouble>(
                *vector_it
            );
            amplitude += *value.get();
        }

        amplitude /= vector->size();

        // update objects vals
        for (row = 0; row < MODEL_AUDIO_FLAME_ROWS_COUNT; row++){
            snprintf(buffer, sizeof(buffer),
                "%d_%s",
                row,
                col_name.c_str()
            );
            objects_it = m_objects.find(buffer);
            if (m_objects.end() != objects_it){
                ObjectSptr object = objects_it->second;
                object->setHumanVal(ObjectDoubleSptr(
                    new ObjectDouble(
                        row ? amplitude/(row*row) : amplitude
                    )
                ));
            }
        }

        {
            ObjectDoubleSptr val(new ObjectDouble(amplitude));
            results->add(col_name, val);
        }
    }

    //PWARN("MORIK -----------------------------\n%s\n",
    //    results->serialize().c_str()
    //);
}

