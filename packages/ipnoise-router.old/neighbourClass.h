#ifndef NEIGHBOUR_CLASS_H
#define NEIGHBOUR_CLASS_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>

#include "shttp.h"

using namespace std;

class NeighbourRecord
{
    public:
        int     id;         //<! ID записи
        string  huid;       //<! пункт назначения
        string  hw_addr;    //<! канальный адрес
        string  dev;        //<! имя устройства

        int cmp(const class NeighbourRecord &right) const
        {
            int res;
            res = strcmp(huid.c_str(), right.huid.c_str());
            if (res) return res;
            res = strcmp(hw_addr.c_str(), right.hw_addr.c_str());
            if (res) return res;
            res = strcmp(dev.c_str(), right.dev.c_str());
            if (res) return res;
            return 0;
        }
        bool operator<(const class NeighbourRecord &right) const
        {
            return (cmp(right) == -1) ? true : false;
        };
};

typedef vector<class NeighbourRecord> NEIGHBOUR_RECORDS;

class NeighbourClass
{
    public:
        NeighbourClass();
        ~NeighbourClass();

        int insertRecord(NeighbourRecord &record);
        int syncTable();


        NEIGHBOUR_RECORDS records;

    private:
        int max_id;
};

#endif

