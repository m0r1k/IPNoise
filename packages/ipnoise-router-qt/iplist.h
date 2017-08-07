#ifndef IPLIST_H
#define IPLIST_H

#include <QString>
#include <QHash>
#include <QTime>
#include <vector>
#include <algorithm>

#include "log.h"

using namespace std;

namespace IPLIST
{
    class Item
    {
        public:
            Item(){
                dev             = "";
                lladdr          = "";
                last_active     = QTime::currentTime();
            };

            QString     dev;
            QString     lladdr;
            QTime       last_active;

            bool operator<(const class Item &right) const
            {
                // note: must be reversed
                if (last_active < right.last_active) return false;
                return true;
            };
    };
    typedef vector<Item>           ITEMS;
    typedef QHash<QString, ITEMS > IPS;
};

using namespace IPLIST;

class IPList
{
    public:
        IPList();
        ~IPList();

        void dump();
        void updateRecord(
            QString     &ip,
            QString     &lladdr,
            QString     &dev
        );

    private:
        IPS     ips;
};

#endif

