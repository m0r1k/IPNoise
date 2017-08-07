#ifndef API_CONTACT_INFO
#define API_CONTACT_INFO

#include <stdio.h>
#include <QObject>
#include <QString>
#include <QMap>
#include <stdint.h>

typedef QMap<QString, int32_t>          ContactItems;
typedef ContactItems::iterator          ContactItemsIt;
typedef ContactItems::const_iterator    ContactItemsConstIt;

class ApiContactInfo
    :   public QObject
{
    Q_OBJECT

    signals:
        void changed(ApiContactInfo *);

    public:
        ApiContactInfo(
            int32_t a_contact_id
        );
        virtual ~ApiContactInfo();

        int32_t     getContactId();
        QString     getContactIdStr();
        void        setNickName(const QString &);
        QString     getNickName() const;
        void        setOnline(bool);
        bool        isOnline() const;

        void        getContactItems(ContactItems &a_out);
        void        delContactItems();
        void        addContactItem(const QString &a_huid);

        ApiContactInfo & operator=(
            const ApiContactInfo &a_right);

    private:
        int32_t         m_contact_id;
        QString         m_nickname;
        bool            m_is_online;
        ContactItems    m_contact_items;
};

#endif

