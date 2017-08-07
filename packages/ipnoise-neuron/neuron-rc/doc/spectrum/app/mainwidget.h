#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QtMultimedia/qaudio.h>

class Engine;
class FrequencySpectrum;
class Spectrograph;
class LevelMeter;

//QT_FORWARD_DECLARE_CLASS(QAudioFormat)
//QT_FORWARD_DECLARE_CLASS(QLabel)
//QT_FORWARD_DECLARE_CLASS(QPushButton)
//QT_FORWARD_DECLARE_CLASS(QMenu)
//QT_FORWARD_DECLARE_CLASS(QAction)

class MainWidget
    :   public QWidget
{
    Q_OBJECT

    public:
        MainWidget(QWidget *parent = 0);
        ~MainWidget();

        // QObject
        void timerEvent(QTimerEvent *event);

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

