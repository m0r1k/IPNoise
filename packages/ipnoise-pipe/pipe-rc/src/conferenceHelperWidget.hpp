#ifndef CONFERENCE_HELPER_WIDGET
#define CONFERENCE_HELPER_WIDGET

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

class ConferenceWidget;
class ConferencesWidget;

#include "api.hpp"

class SelectConferenceButton
    :   public QPushButton
{
    Q_OBJECT

    signals:
        void clicked(ConferenceWidget *);

    public:
        SelectConferenceButton(
            ConferenceWidget    *a_conference,
            const QString       &a_text,
            QWidget             *a_parent
        );
        virtual ~SelectConferenceButton();

    protected slots:
        void clicked(bool);

    private:
        ConferenceWidget    *m_conference;
};

// --------------------------------------------

class NewConferenceButton
    :   public QPushButton
{
    Q_OBJECT

    signals:
        void clicked(QString);

    public:
        NewConferenceButton(
            const QString       &a_huid,
            const QString       &a_text,
            QWidget             *a_parent
        );
        virtual ~NewConferenceButton();

    protected slots:
        void clicked(bool);

    private:
        QString m_huid;
};

// --------------------------------------------

class ConferenceHelperWidget
    :   public  QWidget
{
    Q_OBJECT

    public:
        ConferenceHelperWidget(
            int32_t             a_contact_id,
            Api                 *a_api,
            ConferencesWidget   *a_conferences,
            QWidget             *a_parent = 0
        );
        virtual ~ConferenceHelperWidget();

        void setupUi(QWidget *a_parent);

    protected slots:
        void startNewConference(const QString &a_huid = "");
        void selectConference(ConferenceWidget *);
        void removeUs();

    private:
        ConferencesWidget   *m_conferences;
        int32_t             m_contact_id;
        Api                 *m_api;
        QVBoxLayout         *m_vbox;
        QSpacerItem         *m_top_spacer;
        QSpacerItem         *m_bottom_spacer;
};

#endif

