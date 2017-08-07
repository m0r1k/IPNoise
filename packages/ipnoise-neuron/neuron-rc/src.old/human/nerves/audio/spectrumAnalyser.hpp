#ifndef HUMAN_NERVES_AUDIO_SPECTRUM_ANALYSER_HPP
#define HUMAN_NERVES_AUDIO_SPECTRUM_ANALYSER_HPP

#include <QByteArray>
#include <QObject>
#include <QVector>

#include "frequencySpectrum.hpp"
#include "spectrum.hpp"

#include "FFTRealFixLenParam.h"

QT_FORWARD_DECLARE_CLASS(QAudioFormat)
QT_FORWARD_DECLARE_CLASS(QThread)

class FFTRealWrapper;

class SpectrumAnalyserThreadPrivate;

/**
 * Implementation of the spectrum analysis which can be run in a
 * separate thread.
 */
class SpectrumAnalyserThread
    :   public QObject
{
    Q_OBJECT

    public:
        SpectrumAnalyserThread(QObject *parent);
        virtual ~SpectrumAnalyserThread();

    public slots:
        void setWindowFunction(WindowFunction type);
        void calculateSpectrum(
            const QByteArray &buffer,
            int inputFrequency,
            int bytesPerSample
        );

    signals:
        void calculationComplete(
            const FrequencySpectrum &spectrum
        );

    private:
        void calculateWindow();

    private:
        FFTRealWrapper*                             m_fft;

        const int                                   m_numSamples;

        WindowFunction                              m_windowFunction;

        typedef FFTRealFixLenParam::DataType        DataType;
        QVector<DataType>                           m_window;

        QVector<DataType>                           m_input;
        QVector<DataType>                           m_output;

        FrequencySpectrum                           m_spectrum;

        QThread*                                    m_thread;
};

/**
 * Class which performs frequency spectrum analysis on a window of
 * audio samples, provided to it by the Engine.
 */
class SpectrumAnalyser
    :   public QObject
{
    Q_OBJECT

    public:
        SpectrumAnalyser(QObject *parent = 0);
        ~SpectrumAnalyser();

    public:
        /*
         * Set the windowing function which is applied before calculating the FFT
         */
        void setWindowFunction(WindowFunction type);

        /*
         * Calculate a frequency spectrum
         *
         * \param buffer       Audio data
         * \param format       Format of audio data
         *
         * Frequency spectrum is calculated asynchronously.  The result is returned
         * via the spectrumChanged signal.
         *
         * An ongoing calculation can be cancelled by calling cancelCalculation().
         *
         */
        void calculate(const QByteArray &buffer, const QAudioFormat &format);

        /*
         * Check whether the object is ready to perform another calculation
         */
        bool isReady() const;

        /*
         * Cancel an ongoing calculation
         *
         * Note that cancelling is asynchronous.
         */
        void cancelCalculation();

    signals:
        void spectrumChanged(const FrequencySpectrum &spectrum);

    private slots:
        void calculationComplete(const FrequencySpectrum &spectrum);

    private:
        void calculateWindow();

        SpectrumAnalyserThread*    m_thread;

        enum State {
            Idle,
            Busy,
            Cancelled
        };

        State              m_state;
};

#endif

