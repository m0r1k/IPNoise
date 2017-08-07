#include "class.hpp"
DEFINE_CLASS(HumanNervesAudioInput);
DEFINE_CLASS(HumanNervesAudioFrame);

#ifndef HUMAN_NERVES_AUDIO_INPUT_HPP
#define HUMAN_NERVES_AUDIO_INPUT_HPP

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <QtMultimedia/qaudio.h>

class Engine;
class FrequencySpectrum;
class Spectrograph;
class LevelMeter;

#include "frequencySpectrum.hpp"
#include "log.hpp"

using namespace std;

DEFINE_CLASS(HumanViewer);

#include "human/nerves/audio.hpp"

class HumanNervesAudioFrame
    :   public QObject
{
    Q_OBJECT

    public:
        HumanNervesAudioFrame(){}
        HumanNervesAudioFrame(
            const FrequencySpectrum &a_spectrum)
        {
            m_spectrum = a_spectrum;
        }
        virtual ~HumanNervesAudioFrame(){}

        void setSprectrum(
            const FrequencySpectrum &a_spectrum)
        {
            m_spectrum = a_spectrum;
        }
        FrequencySpectrum getSpectrum(){
            return m_spectrum;
        }

    private:
        FrequencySpectrum   m_spectrum;
};

class HumanNervesAudioInput
    :   public HumanNervesAudio
{
    Q_OBJECT

    public:
        HumanNervesAudioInput(
            HumanViewer *,
            QWidget     *a_parent = 0
        );
        virtual ~HumanNervesAudioInput();

        // QObject
        void timerEvent(QTimerEvent *event);

    signals:
        void newFrame(HumanNervesAudioFrameSptr);

    public slots:
        void spectrumChanged(
            qint64                  position,
            qint64                  length,
            const FrequencySpectrum &spectrum
        );

    private slots:
        void initializeRecord();

    private:
        void createUi();
        void createMenus();
        void connectUi();
        void reset();

        enum Mode {
            NoMode,
            RecordMode,
            GenerateToneMode,
            LoadFileMode
        };

        void setMode(Mode mode);

        Engine*                 m_engine;
        Spectrograph*           m_spectrograph;
        LevelMeter*             m_levelMeter;
        int                     m_infoMessageTimerId;

};

#endif

