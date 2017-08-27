class LinkWidget;

#ifndef LINK_WIDGET_HPP
#define LINK_WIDGET_HPP

#include <QtCore/QMap>
#include <QtWidgets/QWidget>
#include <QtWidgets/QInputDialog>
#include <QtGui/QIcon>

#include <ipnoise-common/log_common.h>

typedef QMap<QString, LinkWidget *>  Links;
typedef Links::iterator              LinksIt;
typedef Links::const_iterator        LinksConstIt;

#define REGISTER_LINK(name)                                 \
    do {                                                    \
        LinksIt         links_it;                           \
        LinkWidget      *link = NULL;                       \
        QString         link_name;                          \
                                                            \
        link = new Link##name##Widget(this);                \
        link_name = link->getName();                        \
                                                            \
        links_it = m_links.find(link_name);                 \
        if (m_links.end() != links_it){                     \
            PERROR("Attempt to double register"             \
                " link: '%s'",                              \
                link_name.toStdString().c_str()             \
            );                                              \
            delete link;                                    \
            break;                                          \
        };                                                  \
        m_links[link_name] = link;                          \
    } while(0);

class LinkWidget
    :   public  QDialog
{
    Q_OBJECT

    public:
        LinkWidget(
            const QString   &a_name,
            const QString   &a_display_name,
            const QIcon     &a_icon,
            QWidget         *a_parent = 0
        );
        virtual ~LinkWidget();

        QString     getName();
        QString     getDisplayName();
        QIcon       getIcon();

        void        setAddr(const QString &);
        QString     getAddr();

    private:
        QString     m_addr;
        QString     m_name;
        QString     m_display_name;
        QIcon       m_icon;
};

#endif

