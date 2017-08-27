#ifndef API_ITEM_INFO
#define API_ITEM_INFO

#include <QtCore/QObject>
#include <QtCore/QString>

class ApiItemInfo
    :   public QObject
{
    Q_OBJECT

    signals:
        void changed(ApiItemInfo *);

    public:
        ApiItemInfo(
            const QString &a_huid
        );
        virtual ~ApiItemInfo();

        QString     getHuid();
        void        setNickName(const QString &);
        QString     getNickName() const;
        void        setOnline(bool);
        bool        isOnline() const;

        ApiItemInfo & operator=(
            const ApiItemInfo &a_right);

    private:
        QString     m_huid;
        QString     m_nickname;
        bool        m_is_online;
};

#endif

