#include "tabsWindow.h"

TabsWindow::TabsWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    
    TabInfo *tabInfo = NULL;

    QSizePolicy sizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Expanding
    );
 
    const QByteArray data("<?xml version='1.0'?><server>    <users>        <user huid='2210:0:7309:227d:3030:fb23:816a:cc5e'>            <neighs>                <neigh huid='2210:0:7309:227d:3030:fb23:816a:cc5d' lladdr='85.119.78.13:2210' dev='udp0' recheck_time='30' last_check='1266241488' last_recv='1265485737' uptime='34770' state='down' down_time='15660'/>                <neigh huid='2210:0:7309:227d:3030:fb23:816a:cc5f' lladdr='95.31.5.65:2210' dev='udp0' recheck_time='30' last_check='1266241488' down_time='46470' last_recv='1265323309' uptime='3060' state='down'/>            </neighs>            <groups>                <group id='1' name='all users'>                    <item huid='2210:0:7309:227d:3030:fb23:816a:0001'/>                </group>            </groups>            <items>                <item huid='2210:0:7309:227d:3030:fb23:816a:0001' status='offline'>                    <name>lexfer</name>                    <icon_self>/public/self.jpg</icon_self>                </item>            </items>        </user>    </users>    <transports>        <transport id='udp0' ifindex='4' last_check='1266241495'/>    </transports></server>");

    tabInfo        = new TabInfo();
    tabInfo->parent = eventsTab;
    tabInfo->child  = new EventsWindow(tabInfo->parent);
    tabs[0] = tabInfo;

    tabInfo        = new TabInfo();
    tabInfo->parent = internalViewTab;
    tabInfo->child  = new XmlEditor(tabInfo->parent, data);
    tabs[2] = tabInfo;

    EventsWindow *eventsWindow = (EventsWindow *)tabs[0]->child;

    eventsWindow->appendItem(
        "15:09:00",
        "command",
        "<?xml version=\"1.0\"?><ipnoise><command type=\"searchRoute\" id=\"4bc790e3554d0ba65ca891346fa8a134b62b467e\"/></ipnoise>"
    );

    eventsWindow->appendItem(
        "15:11:00",
        "debug",
        "1799"
    );

    eventsWindow->appendItem(
        "15:13:00",
        "command",
        "<?xml version=\"1.0\"?><ipnoise><events><event><name>test</name><type>bugaga</type></event></events></ipnoise>"
    );

    eventsWindow->setSizePolicy(sizePolicy);

    show();

    connect (
        tabWidget,  SIGNAL(currentChanged(int)),
        this,       SLOT(currentChangedSlot(int))
    );
}

TabsWindow::~TabsWindow()
{

}

void TabsWindow::currentChangedSlot(int cur_tab)
{
    TabInfo *tab_info = tabs[cur_tab];
    if (tab_info == NULL){
        return;
    }
    tab_info->child->resize(tab_info->parent->size());
}

void TabsWindow::resizeEvent(QResizeEvent *event)
{
    int cur_tab = tabWidget->currentIndex();
    TabInfo *tab_info = tabs[cur_tab];
    if (tab_info == NULL){
        return;
    }
    tab_info->child->resize(tab_info->parent->size());
}

