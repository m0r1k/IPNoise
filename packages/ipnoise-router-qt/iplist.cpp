#include "iplist.h"

void IPList::dump()
{
    IPS::iterator   ips_i;
    ITEMS::iterator items_i;

    for (ips_i = ips.begin();
        ips_i != ips.end();
        ips_i++)
    {
        ITEMS &items = ips_i.value();
        PDEBUG(5, "---------- dump iplist ---------- { \n");
        PDEBUG(5, "IP: %s\n", ips_i.key().toAscii().data());
        PDEBUG(5, "%-28.28s\t%-10.10s\n", "LLADDR", "DEV");
        for (items_i = items.begin();
            items_i != items.end();
            items_i++)
        {
            Item *cur_item = &(*items_i);
            PDEBUG(5, "%-28.28s\t%-10.10s\n",
                cur_item->lladdr.toAscii().data(),
                cur_item->dev.toAscii().data()
            );
        }
        PDEBUG(5, "---------- dump iplist ---------- }\n");
    }
}

void IPList::updateRecord(
    QString &ip,
    QString &lladdr,
    QString &dev)
{
    QTime cur_time = QTime::currentTime();

    IPS::iterator   ips_i;
    ITEMS::iterator items_i;

    Item    new_item;
    Item    *item = NULL;
    ITEMS   *items;

    ips_i = ips.find(ip);
    if (ips_i == ips.end()){
        PDEBUG(15, "MORIK create new items for ip: '%s'\n", ip.toAscii().data());
        // create items list
        ITEMS new_items;
        ips[ip] = new_items;
        // get our items list
        items   = &ips[ip];
    } else {
        // get our items list
        PDEBUG(15, "MORIK items was found for ip: '%s'\n",
            ip.toAscii().data());
        items = &ips_i.value();
    }

again:
    for (items_i  = items->begin();
         items_i != items->end();
         items_i++)
    {
        Item *cur_item = &(*items_i);
        PDEBUG(15, "cur_item->dev: '%s' (mustbe: '%s'),"
            " cur_item->lladdr: '%s' (mustbe: '%s')\n",
            cur_item->dev.toAscii().data(),
            dev.toAscii().data(),
            cur_item->lladdr.toAscii().data(),
            lladdr.toAscii().data()
        );

        if (    cur_item->dev    == dev
            &&  cur_item->lladdr == lladdr)
        {
            item = cur_item;
            break;
        }
    }

    if (item == NULL){
        // we have not items for this ip
        new_item.dev    = dev;
        new_item.lladdr = lladdr;
        items->push_back(new_item);
        goto again;
    }

    // update item active time
    item->last_active = cur_time;

    // sort items
    std::sort(items->begin(), items->end());

    // get first item
    item = &(*items->begin());

    // setup one record
    char buf[65536];
    snprintf(buf, sizeof(buf),
        "ip route replace %s/128 dev %s",
        ip.toAscii().data(),
        item->dev.toAscii().data()
    );
    PDEBUG(5, "exec: '%s'\n", buf);
    system(buf);

    return;
};

IPList::IPList()
{

};

IPList::~IPList()
{

};


