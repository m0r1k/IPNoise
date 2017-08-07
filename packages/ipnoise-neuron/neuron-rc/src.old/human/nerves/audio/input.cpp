#include "human/nerves/audio/engine.hpp"
#include "human/nerves/audio/levelmeter.hpp"
#include "human/nerves/audio/spectrograph.hpp"
#include "human/nerves/audio/audio_utils.hpp"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>

#include "input.hpp"

const int NullTimerId = -1;

HumanNervesAudioInput::HumanNervesAudioInput(
    HumanViewer *a_human_viewer,
    QWidget     *a_parent)
    :   HumanNervesAudio(
            a_human_viewer,
            a_parent
        ),
        m_engine(new Engine(this)),
        m_spectrograph(new Spectrograph(this)),
        m_levelMeter(new LevelMeter(this)),
        m_infoMessageTimerId(NullTimerId)
{
    m_spectrograph->setParams(
        SpectrumNumBands,
        SpectrumLowFreq,
        SpectrumHighFreq
    );

    createUi();
    connectUi();

    initializeRecord();
    m_engine->startRecording();
}

HumanNervesAudioInput::~HumanNervesAudioInput()
{
}

void HumanNervesAudioInput::spectrumChanged(
    qint64,                 // position
    qint64,                 // length
    const FrequencySpectrum &spectrum)
{
    HumanNervesAudioFrameSptr frame;

    frame = HumanNervesAudioFrameSptr(
        new HumanNervesAudioFrame(spectrum)
    );
    emit newFrame(frame);

    m_spectrograph->spectrumChanged(spectrum);
}

void HumanNervesAudioInput::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_infoMessageTimerId);
    Q_UNUSED(event) // suppress warnings in release builds
    killTimer(m_infoMessageTimerId);
    m_infoMessageTimerId = NullTimerId;
}

void HumanNervesAudioInput::initializeRecord()
{
    reset();
    m_engine->initializeRecord();
}

void HumanNervesAudioInput::createUi()
{
    setWindowTitle(tr("Spectrum Analyser"));

    QVBoxLayout *windowLayout = new QVBoxLayout(this);

    QScopedPointer<QHBoxLayout> analysisLayout(new QHBoxLayout);
    analysisLayout->addWidget(m_spectrograph);
    m_spectrograph->setMinimumSize(QSize(600, 400));
    analysisLayout->addWidget(m_levelMeter);
    windowLayout->addLayout(analysisLayout.data());
    analysisLayout.take();

    setLayout(windowLayout);
}

void HumanNervesAudioInput::connectUi()
{
    CHECKED_CONNECT(
        m_engine,       SIGNAL(levelChanged(qreal, qreal, int)),
        m_levelMeter,   SLOT(levelChanged(qreal, qreal, int))
    );

    CHECKED_CONNECT(
        m_engine,       SIGNAL(spectrumChanged(qint64, qint64, const FrequencySpectrum &)),
        this,           SLOT(spectrumChanged(qint64, qint64, const FrequencySpectrum &))
    );
}

void HumanNervesAudioInput::reset()
{
    m_engine->reset();
    m_levelMeter->reset();
    m_spectrograph->reset();
}

