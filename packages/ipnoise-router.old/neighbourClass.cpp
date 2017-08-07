#include "neighbourClass.h"

int NeighbourClass::syncTable()
{
    char buffer[1024];

    NEIGHBOUR_RECORDS::iterator records_i;

    PDEBUG(5, "sync neighbour records:\n");
    PDEBUG(5, "%-4.4s %-35.35s %-20.20s %-20.20s\n",
            "ID",
            "HUID",
            "HW_ADDR",
            "DEV"
    );

    for (records_i  = records.begin();
         records_i != records.end();
         records_i++)
    {
        NeighbourRecord &v = *records_i;
        PDEBUG(5, "%-4.4d %-35.35s %-20.20s %-20.20s\n",
            v.id,
            v.huid.c_str(),
            v.hw_addr.c_str(),
            v.dev.c_str()
        );
        snprintf(buffer, sizeof(buffer) - 1,
            "ip neigh replace %s lladdr %s dev %s",
            v.huid.c_str(),
            v.hw_addr.c_str(),
            v.dev.c_str()
        );
        PDEBUG(6, "Execute: '%s'\n", buffer);
    }
    PDEBUG(5, "\n");
};

int NeighbourClass::insertRecord(NeighbourRecord &record)
{
    int record_id = -1;
    NEIGHBOUR_RECORDS::iterator records_i;
    records_i = lower_bound(
        records.begin(),
        records.end(),
        record
    );

    if ((records_i != records.end()
        && !record.cmp(*records_i)))
    {
        // exist
        record_id = records_i->id;
    } else {
        record.id = max_id++;
        record_id = record.id;
        records.insert(records_i, record);
        syncTable();
    }
    return record_id;
}

NeighbourClass::NeighbourClass()
{
    // load
    max_id = 0;
};

NeighbourClass::~NeighbourClass()
{
};

