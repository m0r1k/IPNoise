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
    assert(MODEL_AUDIO_FLAME_HZ_PER_NEURON > 0);
}

HumanModelAudioFlame::~HumanModelAudioFlame()
{
}

NeuronSptr HumanModelAudioFlame::init()
{
    HumanViewer      *human_viewer  = getHumanViewer();
    HumanNervesAudio *audio         = NULL;
    DbThread         *db            = getDbThread();
    int32_t          col            = 0;
    char             buffer[512]    = { 0x00 };
    NeuronSptr       neuron;

    double  step    = 1.0f/(MODEL_AUDIO_FLAME_UP_HZ/MODEL_AUDIO_FLAME_HZ_PER_NEURON);
    double  ax      = .0f;
    double  ay      = .0f;
    double  az      = .0f;
    int32_t row     = 0;

    audio = human_viewer->getHumanNervesAudio();

    if (!audio){
        PERROR("Cannot get audio nerves\n");
        goto fail;
    }

    neuron = db->getNeuronById(PropNeuronIdSptr(
        new PropNeuronId(
            MODEL_AUDIO_FLAME_NEURON_NAME
        )
    ));
    if (!neuron){
        neuron = getNetwork()->createNeuron();
        neuron->setId(PropNeuronIdSptr(
            new PropNeuronId(
                MODEL_AUDIO_FLAME_NEURON_NAME
            )
        ));

        for (row = 0; row < MODEL_AUDIO_FLAME_ROWS_COUNT; row++){
            ax = -step*(MODEL_AUDIO_FLAME_UP_HZ/MODEL_AUDIO_FLAME_HZ_PER_NEURON)/2;
            for (col = 0;
                col < MODEL_AUDIO_FLAME_UP_HZ;
                col += MODEL_AUDIO_FLAME_HZ_PER_NEURON)
            {
                NeuronSptr  new_neuron;
                string      col_name;
                string      nerve_path;

                int32_t low  = col;
                int32_t high = col + MODEL_AUDIO_FLAME_HZ_PER_NEURON - 1;

                new_neuron = getNetwork()->createNeuron<Neuron>();

                new_neuron->setPosX(PropDoubleSptr(
                    new PropDouble(
                        ax
                    )
                ));
                new_neuron->setPosY(PropDoubleSptr(
                    new PropDouble(
                        ay
                    )
                ));
                new_neuron->setPosZ(PropDoubleSptr(
                    new PropDouble(
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
                new_neuron->setHumanNervePath(PropStringSptr(
                    new PropString(nerve_path)
                ));

                LINK(neuron.get(), new_neuron.get());

                // store to neurons by columns
                m_neurons[col_name] = new_neuron;
            }
        }
    }

    CHECKED_CONNECT(
        audio,  SIGNAL(newFrame(HumanNervesAudioFrameSptr)),
        this,   SLOT(newFrameSlot(HumanNervesAudioFrameSptr))
    );

out:
    return neuron;
fail:
    goto out;
}

void HumanModelAudioFlame::newFrameSlot(
    HumanNervesAudioFrameSptr a_frame)
{
    FrequencySpectrum::const_iterator spectrum_it;

    FrequencySpectrum   spectrum    = a_frame->getSpectrum();
    char                buffer[512] = { 0x00};

    PropMapSptr         vals_by_cols(new PropMap);
    PropMapSptr         results(new PropMap);
    PropMapIterator     map_it;
    AudioFlameNeuronsIt neurons_it;
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

        PropVectorSptr  vector;
        string          col_name;

        int32_t col     = frequency/MODEL_AUDIO_FLAME_HZ_PER_NEURON;
        int32_t low     = MODEL_AUDIO_FLAME_HZ_PER_NEURON * col;
        int32_t high    = MODEL_AUDIO_FLAME_HZ_PER_NEURON * (col + 1) - 1;

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
            vector = dynamic_pointer_cast<PropVector>(
                vals_by_cols->get(col_name)
            );
        } else {
            // create array
            vector = PROP_VECTOR();
            vals_by_cols->add(col_name, vector);
        }

        // add value to vector
        PropDoubleSptr val(new PropDouble(amplitude));
        vector->add(val);
    }

    // clear current vals
    for (neurons_it = m_neurons.begin();
        neurons_it != m_neurons.end();
        neurons_it++)
    {
        NeuronSptr neuron = neurons_it->second;
        neuron->setHumanVal(PropDoubleSptr(
            new PropDouble
        ));
    }

    // calculate average amplitude
    for (map_it = vals_by_cols->begin();
        map_it != vals_by_cols->end();
        map_it++)
    {
        PropSptr    key         = map_it->first;
        PropSptr    val         = map_it->second;
        string      col_name    = key->toString();
        double      amplitude   = 0.f;

        PropVectorSptr      vector;
        PropVectorIterator  vector_it;

        vector = dynamic_pointer_cast<PropVector>(val);
        for (vector_it = vector->begin();
            vector_it != vector->end();
            vector_it++)
        {
            PropDoubleSptr value;
            value = dynamic_pointer_cast<PropDouble>(
                *vector_it
            );
            amplitude += *value.get();
        }

        amplitude /= vector->size();

        // update neurons vals
        for (row = 0; row < MODEL_AUDIO_FLAME_ROWS_COUNT; row++){
            snprintf(buffer, sizeof(buffer),
                "%d_%s",
                row,
                col_name.c_str()
            );
            neurons_it = m_neurons.find(buffer);
            if (m_neurons.end() != neurons_it){
                NeuronSptr neuron = neurons_it->second;
                neuron->setHumanVal(PropDoubleSptr(
                    new PropDouble(
                        row ? amplitude/(row*row) : amplitude
                    )
                ));
            }
        }

        {
            PropDoubleSptr val(new PropDouble(amplitude));
            results->add(col_name, val);
        }
    }

    //PWARN("MORIK -----------------------------\n%s\n",
    //    results->serialize().c_str()
    //);
}

