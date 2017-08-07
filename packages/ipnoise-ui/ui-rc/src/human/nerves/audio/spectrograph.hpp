#ifndef HUMAN_NERVES_AUDIO_SPECTROGRAPH_HPP
#define HUMAN_NERVES_AUDIO_SPECTROGRAPH_HPP

#include <QWidget>
#include "frequencySpectrum.hpp"

QT_FORWARD_DECLARE_CLASS(QMouseEvent)

/**
 * Widget which displays a spectrograph showing the frequency spectrum
 * of the window of audio samples most recently analyzed by the Engine.
 */
class Spectrograph
    :   public QWidget
{
    Q_OBJECT

    public:
        Spectrograph(QWidget *parent = 0);
        ~Spectrograph();

        void setParams(int numBars, qreal lowFreq, qreal highFreq);

        // QObject
        void timerEvent(QTimerEvent *event);

        // QWidget
        void paintEvent(QPaintEvent *event);
        void mousePressEvent(QMouseEvent *event);

    signals:
        void infoMessage(const QString &message, int intervalMs);

    public slots:
        void reset();
        void spectrumChanged(const FrequencySpectrum &spectrum);

    private:
        int barIndex(qreal frequency) const;
        QPair<qreal, qreal> barRange(int barIndex) const;
        void updateBars();

        void selectBar(int index);

    private:
        struct Bar {
            Bar() : value(0.0), clipped(false) { }
            qreal   value;
            bool    clipped;
        };

        QVector<Bar>        m_bars;
        int                 m_barSelected;
        int                 m_timerId;
        qreal               m_lowFreq;
        qreal               m_highFreq;
        FrequencySpectrum   m_spectrum;

};

#endif

