#include "routeClass.h"
#include "net/icmp.h"
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

void routeClass::icmpReply(__u8 *buf, uint32_t len)
{
    Routes::iterator routes_i;
    struct icmp6_hdr    *icmph;
    struct icmp_payload *payload = NULL;
    icmph = (struct icmp6_hdr *) buf;

    char buffer[1024];

    if (icmph->icmp6_id != IPNOISE_ICMP6_REPLY_ID){
        // invalid icmp reply
        goto ret;
    }
    if (len-sizeof(struct icmp6_hdr) < sizeof(struct icmp_payload)){
        // don't have valid payload size
        goto ret;
    }

    // seek to payload
    payload = (struct icmp_payload*)(buf + sizeof(struct icmp6_hdr));

    for (routes_i = routes.begin();
        routes_i != routes.end();
        routes_i++)
    {
        routeRecordClass *record = &(*routes_i);

        if ((uint32_t)record == payload->id){
            struct timeval tv;
            gettimeofday(&tv, NULL);
            record->addPingRes(&tv);
            //record->dump_header();
            //record->dump();

            // add route
            snprintf(buffer, sizeof(buffer) - 1,
                "/sbin/ip route replace %s/128 dev %s",
                record->getDstHuid(),
                record->getDev()
            );
            // PDEBUG(5, "MORIK SYSTEM: '%s'\n", buffer);
            my_system(buffer);

            // add neigh
            snprintf(buffer, sizeof(buffer) - 1,
                "/sbin/ip neigh replace %s lladdr %s dev %s",
                record->getDstHuid(),
                record->getLladdr(),
                record->getDev()
            );
            // PDEBUG(5, "MORIK SYSTEM: '%s'\n", buffer);
            my_system(buffer);
        }
    }
ret:
    return;
}

void routeClass::checkRoutes()
{
    Routes::iterator routes_i;

    for (routes_i = routes.begin();
        routes_i != routes.end();
        routes_i++)
    {
        routeRecordClass &record = *routes_i;
        record.ping();
//        record.dump_header();
//        record.dump();
    }
}

void routeClass::dump()
{
    Routes::iterator routes_i;

    PDEBUG(5, "--- Dump route table --- {\n");
    routeRecordClass::dump_header();
    for (routes_i = routes.begin();
        routes_i != routes.end();
        routes_i++)
    {
        routeRecordClass &record = *routes_i;
        record.dump();
    }
    PDEBUG(5, "--- Dump route table --- }\n");
}

void routeClass::addRoute(routeRecordClass &route)
{
    char buffer[1024];

    // add addr to device
    snprintf(buffer, sizeof(buffer) - 1,
        "/sbin/ip addr replace %s/128 dev %s",
        route.getSrcHuid(),
        route.getDev()
    );
    // PDEBUG(5, "MORIK SYSTEM: '%s'\n", buffer);
    my_system(buffer);

    routes.push_back(route);
}

routeClass::routeClass()
{
};

routeClass::~routeClass()
{
};

