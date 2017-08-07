#include "engine.h"
#include "levelmeter.h"
#include "mainwidget.h"
#include "spectrograph.h"
#include "utils.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QMenu>
#include <QFileDialog>
#include <QTimerEvent>
#include <QMessageBox>

const int NullTimerId = -1;

MainWidget::MainWidget(QWidget *parent)
    :   QWidget(parent),
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

MainWidget::~MainWidget()
{

}

void MainWidget::spectrumChanged(
    qint64,                 // position
    qint64,                 // length
    const FrequencySpectrum &spectrum)
{
    FrequencySpectrum::const_iterator it;

/*
    int dirty = 0;

    for (it = spectrum.begin();
        it != spectrum.end();
        it++)
    {
        qreal frequency = it->frequency;
        qreal amplitude = it->amplitude;

        if (0.25 > amplitude){
            continue;
        }

        fprintf(stderr, "MORIK"
            " frequency: '%.0f',"
            " amplitude: '%f'"
            "\n",
            frequency,
            amplitude
        );

        dirty = 1;
    }

    if (dirty){
        fprintf(stderr,
            "MORIK ---------------------------------------\n"
        );
    }
*/

    m_spectrograph->spectrumChanged(spectrum);
}

void MainWidget::timerEvent(QTimerEvent *event)
{
    Q_ASSERT(event->timerId() == m_infoMessageTimerId);
    Q_UNUSED(event) // suppress warnings in release builds
    killTimer(m_infoMessageTimerId);
    m_infoMessageTimerId = NullTimerId;
}

void MainWidget::initializeRecord()
{
    reset();
    m_engine->initializeRecord();
}

void MainWidget::createUi()
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

void MainWidget::connectUi()
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

void MainWidget::reset()
{
    m_engine->reset();
    m_levelMeter->reset();
    m_spectrograph->reset();
}

