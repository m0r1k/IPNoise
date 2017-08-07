#include <math.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <QSet>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QDebug>
#include <QThread>
#include <QFile>

#include "human/nerves/audio/audio_utils.hpp"
#include "log.hpp"

#include "human/nerves/audio/engine.hpp"

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const qint64 BufferDurationUs       = 100 * 1000000;
const int    NotifyIntervalMs       = 100;

// Size of the level calculation window in microseconds
const int    LevelWindowUs          = 0.1 * 1000000;


//-----------------------------------------------------------------------------
// Helper functions
//-----------------------------------------------------------------------------

QDebug& operator<<(QDebug &debug, const QAudioFormat &format)
{
    debug << format.frequency() << "Hz"
          << format.channels() << "channels";
    return debug;
}

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------

Engine::Engine(QObject *parent)
    :   QObject(parent)
    ,   m_mode(QAudio::AudioInput)
    ,   m_state(QAudio::StoppedState)
    ,   m_availableAudioInputDevices
            (QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    ,   m_audioInputDevice(QAudioDeviceInfo::defaultInputDevice())
    ,   m_audioInput(0)
    ,   m_audioInputIODevice(0)
    ,   m_recordPosition(0)
    ,   m_availableAudioOutputDevices
            (QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    ,   m_audioOutputDevice(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_audioOutput(0)
    ,   m_playPosition(0)
    ,   m_bufferPosition(0)
    ,   m_bufferLength(0)
    ,   m_dataLength(0)
    ,   m_levelBufferLength(0)
    ,   m_rmsLevel(0.0)
    ,   m_peakLevel(0.0)
    ,   m_spectrumBufferLength(0)
    ,   m_spectrumAnalyser(0)
    ,   m_spectrumPosition(0)
    ,   m_count(0)
{
    qRegisterMetaType<FrequencySpectrum>("FrequencySpectrum");
    qRegisterMetaType<WindowFunction>("WindowFunction");

    CHECKED_CONNECT(
        &m_spectrumAnalyser, SIGNAL(spectrumChanged(FrequencySpectrum)),
        this,                SLOT(spectrumChanged(FrequencySpectrum))
    );

    initialize();
}

Engine::~Engine()
{

}

bool Engine::initializeRecord()
{
    reset();
    // ENGINE_DEBUG << "Engine::initializeRecord";
    return initialize();
}

qint64 Engine::bufferLength() const
{
    return m_bufferLength;
}

void Engine::setWindowFunction(WindowFunction type)
{
    m_spectrumAnalyser.setWindowFunction(type);
}

void Engine::startRecording()
{
    if (m_audioInput) {
        if (    QAudio::AudioInput      == m_mode
            &&  QAudio::SuspendedState  == m_state)
        {
            m_audioInput->resume();
        } else {
            m_spectrumAnalyser.cancelCalculation();
            spectrumChanged(0, 0, FrequencySpectrum());

            m_buffer.fill(0);
            setRecordPosition(0, true);

            stopPlayback();

            m_mode = QAudio::AudioInput;

            CHECKED_CONNECT(
                m_audioInput,   SIGNAL(stateChanged(QAudio::State)),
                this,           SLOT(audioStateChanged(QAudio::State))
            );
            CHECKED_CONNECT(
                m_audioInput,   SIGNAL(notify()),
                this,           SLOT(audioNotify())
            );

            m_count         = 0;
            m_dataLength    = 0;

            emit dataLengthChanged(0);

            m_audioInputIODevice = m_audioInput->start();

            CHECKED_CONNECT(
                m_audioInputIODevice,   SIGNAL(readyRead()),
                this,                   SLOT(audioDataReady())
            );
        }
    }
}

void Engine::startPlayback()
{
    if (m_audioOutput) {
        if (QAudio::AudioOutput == m_mode &&
            QAudio::SuspendedState == m_state) {
            m_audioOutput->resume();
        } else {
            m_spectrumAnalyser.cancelCalculation();
            spectrumChanged(0, 0, FrequencySpectrum());
            setPlayPosition(0, true);
            stopRecording();
            m_mode = QAudio::AudioOutput;

            CHECKED_CONNECT(
                m_audioOutput,  SIGNAL(stateChanged(QAudio::State)),
                this,           SLOT(audioStateChanged(QAudio::State))
            );

            CHECKED_CONNECT(
                m_audioOutput,  SIGNAL(notify()),
                this,           SLOT(audioNotify())
            );

            m_count = 0;
            m_audioOutputIODevice.close();
            m_audioOutputIODevice.setBuffer(&m_buffer);
            m_audioOutputIODevice.open(QIODevice::ReadOnly);
            m_audioOutput->start(&m_audioOutputIODevice);
        }
    }
}

void Engine::suspend()
{
    if (QAudio::ActiveState == m_state ||
        QAudio::IdleState == m_state) {
        switch (m_mode) {
        case QAudio::AudioInput:
            m_audioInput->suspend();
            break;
        case QAudio::AudioOutput:
            m_audioOutput->suspend();
            break;
        }
    }
}

void Engine::setAudioInputDevice(const QAudioDeviceInfo &device)
{
    if (device.deviceName() != m_audioInputDevice.deviceName()) {
        m_audioInputDevice = device;
        initialize();
    }
}

void Engine::setAudioOutputDevice(const QAudioDeviceInfo &device)
{
    if (device.deviceName() != m_audioOutputDevice.deviceName()) {
        m_audioOutputDevice = device;
        initialize();
    }
}

void Engine::audioNotify()
{
    switch (m_mode){
        case QAudio::AudioInput: {
            qint64 recordPosition = 0;
            qint64 levelPosition  = 0;

            recordPosition = qMin(
                m_bufferLength,
                audioLength(m_format, m_audioInput->processedUSecs())
            );

            setRecordPosition(recordPosition);

            levelPosition = m_dataLength - m_levelBufferLength;

            if (levelPosition >= 0){
                calculateLevel(levelPosition, m_levelBufferLength);
            }

            if (m_dataLength >= m_spectrumBufferLength){
                const qint64 spectrumPosition = m_dataLength - m_spectrumBufferLength;
                calculateSpectrum(spectrumPosition);
            }

//            if (m_dataLength >= (m_spectrumBufferLength*10)){
//                m_dataLength = 0;
//            }

            emit bufferChanged(0, m_dataLength, m_buffer);
        }
        break;
    case QAudio::AudioOutput: {
            const qint64 playPosition = audioLength(m_format, m_audioOutput->processedUSecs());
            setPlayPosition(qMin(bufferLength(), playPosition));
            const qint64 levelPosition = playPosition - m_levelBufferLength;
            const qint64 spectrumPosition = playPosition - m_spectrumBufferLength;
            if (playPosition >= m_dataLength)
                stopPlayback();
            if (levelPosition >= 0 && levelPosition + m_levelBufferLength < m_bufferPosition + m_dataLength)
                calculateLevel(levelPosition, m_levelBufferLength);
            if (spectrumPosition >= 0 && spectrumPosition + m_spectrumBufferLength < m_bufferPosition + m_dataLength)
                calculateSpectrum(spectrumPosition);
        }
        break;
    }
}

void Engine::audioStateChanged(QAudio::State state)
{
    // ENGINE_DEBUG << "Engine::audioStateChanged from" << m_state
    //              << "to" << state;

    if (QAudio::IdleState == state){
        stopPlayback();
    } else {
        if (QAudio::StoppedState == state) {
            // Check error
            QAudio::Error error = QAudio::NoError;
            switch (m_mode) {
            case QAudio::AudioInput:
                error = m_audioInput->error();
                break;
            case QAudio::AudioOutput:
                error = m_audioOutput->error();
                break;
            }
            if (QAudio::NoError != error) {
                reset();
                return;
            }
        }
        setState(state);
    }
}

void Engine::audioDataReady()
{
    Q_ASSERT(0 == m_bufferPosition);

    qint64 bytesReady     = m_audioInput->bytesReady();
    qint64 bytesSpace     = m_buffer.size() - m_dataLength;
    qint64 bytesToRead    = qMin(bytesReady, bytesSpace);
    qint64 bytesRead      = 0;

    bytesRead = m_audioInputIODevice->read(
        m_buffer.data() + m_dataLength,
        bytesToRead
    );

    if (bytesRead){
        m_dataLength += bytesRead;
    } else {
        m_dataLength = 0;
    }

    emit dataLengthChanged(dataLength());
}

void Engine::spectrumChanged(const FrequencySpectrum &spectrum)
{
    // ENGINE_DEBUG << "Engine::spectrumChanged" << "pos" << m_spectrumPosition;
    emit spectrumChanged(m_spectrumPosition, m_spectrumBufferLength, spectrum);
}

void Engine::resetAudioDevices()
{
    delete m_audioInput;
    m_audioInput = 0;
    m_audioInputIODevice = 0;
    setRecordPosition(0);
    delete m_audioOutput;
    m_audioOutput = 0;
    setPlayPosition(0);
    m_spectrumPosition = 0;
    setLevel(0.0, 0.0, 0);
}

void Engine::reset()
{
    stopRecording();
    stopPlayback();
    setState(QAudio::AudioInput, QAudio::StoppedState);
    setFormat(QAudioFormat());
    m_buffer.clear();
    m_bufferPosition    = 0;
    m_bufferLength      = 0;
    m_dataLength        = 0;
    emit dataLengthChanged(0);
    resetAudioDevices();
}

bool Engine::initialize()
{
    bool result = false;

    QAudioFormat format = m_format;

    if (selectFormat()) {
        if (m_format != format) {
            resetAudioDevices();
            m_bufferLength = audioLength(m_format, BufferDurationUs);
            m_buffer.resize(m_bufferLength);
            m_buffer.fill(0);
            emit bufferLengthChanged(bufferLength());
            emit bufferChanged(0, 0, m_buffer);
            m_audioInput = new QAudioInput(m_audioInputDevice, m_format, this);
            m_audioInput->setNotifyInterval(NotifyIntervalMs);
            result = true;
            m_audioOutput = new QAudioOutput(m_audioOutputDevice, m_format, this);
            m_audioOutput->setNotifyInterval(NotifyIntervalMs);
        }
    } else {
        emit errorMessage(tr("No common input / output format found"), "");
    }

    // ENGINE_DEBUG << "Engine::initialize" << "m_bufferLength" << m_bufferLength;
    // ENGINE_DEBUG << "Engine::initialize" << "m_dataLength" << m_dataLength;
    // ENGINE_DEBUG << "Engine::initialize" << "format" << m_format;

    return result;
}

bool Engine::selectFormat()
{
    bool foundSupportedFormat = false;

    if (QAudioFormat() != m_format) {
        QAudioFormat format = m_format;
        if (m_audioOutputDevice.isFormatSupported(format)) {
            setFormat(format);
            foundSupportedFormat = true;
        }
    } else {

        QList<int> frequenciesList;
        frequenciesList += m_audioInputDevice.supportedFrequencies();

        frequenciesList += m_audioOutputDevice.supportedFrequencies();
        frequenciesList = frequenciesList.toSet().toList(); // remove duplicates
        qSort(frequenciesList);
        // ENGINE_DEBUG << "Engine::initialize frequenciesList" << frequenciesList;

        QList<int> channelsList;
        channelsList += m_audioInputDevice.supportedChannels();
        channelsList += m_audioOutputDevice.supportedChannels();
        channelsList = channelsList.toSet().toList();
        qSort(channelsList);
        // ENGINE_DEBUG << "Engine::initialize channelsList" << channelsList;

        QAudioFormat format;
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setCodec("audio/pcm");
        format.setSampleSize(16);
        format.setSampleType(QAudioFormat::SignedInt);
        int frequency, channels;
        foreach (frequency, frequenciesList) {
            if (foundSupportedFormat)
                break;
            format.setFrequency(frequency);
            foreach (channels, channelsList) {
                format.setChannels(channels);
                const bool inputSupport = m_audioInputDevice.isFormatSupported(format);
                const bool outputSupport = m_audioOutputDevice.isFormatSupported(format);
                // ENGINE_DEBUG << "Engine::initialize checking " << format
                //              << "input" << inputSupport
                //              << "output" << outputSupport;
                if (inputSupport && outputSupport) {
                    foundSupportedFormat = true;
                    break;
                }
            }
        }

        if (!foundSupportedFormat)
            format = QAudioFormat();

        setFormat(format);
    }

    return foundSupportedFormat;
}

void Engine::stopRecording()
{
    if (m_audioInput) {
        m_audioInput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioInput->disconnect();
    }
    m_audioInputIODevice = 0;
}

void Engine::stopPlayback()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        QCoreApplication::instance()->processEvents();
        m_audioOutput->disconnect();
        setPlayPosition(0);
    }
}

void Engine::setState(QAudio::State state)
{
    const bool changed = (m_state != state);
    m_state = state;
    if (changed)
        emit stateChanged(m_mode, m_state);
}

void Engine::setState(QAudio::Mode mode, QAudio::State state)
{
    const bool changed = (m_mode != mode || m_state != state);
    m_mode = mode;
    m_state = state;
    if (changed)
        emit stateChanged(m_mode, m_state);
}

void Engine::setRecordPosition(qint64 position, bool forceEmit)
{
    const bool changed = (m_recordPosition != position);
    m_recordPosition = position;
    if (changed || forceEmit)
        emit recordPositionChanged(m_recordPosition);
}

void Engine::setPlayPosition(qint64 position, bool forceEmit)
{
    const bool changed = (m_playPosition != position);
    m_playPosition = position;
    if (changed || forceEmit)
        emit playPositionChanged(m_playPosition);
}

void Engine::calculateLevel(qint64 position, qint64 length)
{
    Q_ASSERT(position + length <= m_bufferPosition + m_dataLength);

    qreal peakLevel = 0.0;

    qreal sum = 0.0;
    const char *ptr = m_buffer.constData() + position - m_bufferPosition;
    const char *const end = ptr + length;
    while (ptr < end) {
        const qint16 value = *reinterpret_cast<const qint16*>(ptr);
        const qreal fracValue = pcmToReal(value);
        peakLevel = qMax(peakLevel, fracValue);
        sum += fracValue * fracValue;
        ptr += 2;
    }
    const int numSamples = length / 2;
    qreal rmsLevel = sqrt(sum / numSamples);

    rmsLevel = qMax(qreal(0.0), rmsLevel);
    rmsLevel = qMin(qreal(1.0), rmsLevel);
    setLevel(rmsLevel, peakLevel, numSamples);

    // ENGINE_DEBUG << "Engine::calculateLevel" << "pos" << position << "len" << length
    //             << "rms" << rmsLevel << "peak" << peakLevel;
}

void Engine::calculateSpectrum(qint64 position)
{
    Q_ASSERT(position + m_spectrumBufferLength <= m_bufferPosition + m_dataLength);
    Q_ASSERT(0 == m_spectrumBufferLength % 2); // constraint of FFT algorithm

    // QThread::currentThread is marked 'for internal use only', but
    // we're only using it for debug output here, so it's probably OK :)
    // ENGINE_DEBUG << "Engine::calculateSpectrum" << QThread::currentThread()
    //              << "count" << m_count << "pos" << position << "len" << m_spectrumBufferLength
    //              << "spectrumAnalyser.isReady" << m_spectrumAnalyser.isReady();

    if(m_spectrumAnalyser.isReady()) {
        m_spectrumBuffer = QByteArray::fromRawData(
            m_buffer.constData() + position - m_bufferPosition,
            m_spectrumBufferLength
        );
        m_spectrumPosition = position;
        m_spectrumAnalyser.calculate(m_spectrumBuffer, m_format);
    }
}

void Engine::setFormat(const QAudioFormat &format)
{
    const bool changed      = (format != m_format);

    m_format                = format;
    m_levelBufferLength     = audioLength(m_format, LevelWindowUs);
    m_spectrumBufferLength  = SpectrumLengthSamples
        * (m_format.sampleSize() / 8) * m_format.channels();

    if (changed){
        emit formatChanged(m_format);
    }
}

void Engine::setLevel(qreal rmsLevel, qreal peakLevel, int numSamples)
{
    m_rmsLevel = rmsLevel;
    m_peakLevel = peakLevel;
    emit levelChanged(m_rmsLevel, m_peakLevel, numSamples);
}

