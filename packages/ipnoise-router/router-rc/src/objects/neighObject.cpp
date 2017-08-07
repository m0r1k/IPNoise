/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/serverObject.h"

#include "objects/neighObject.h"

NeighObject * NeighObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NeighObject(a_node, a_doc, a_tagname);
}

NeighObject::NeighObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    Object::setAttributeSafe("state", "down");
};

NeighObject::~NeighObject()
{
};

void NeighObject::registrySignals()
{
    getDocument()->registrySignal(
        "neigh_up",
        "ipn_neigh"
    );
    getDocument()->registrySignal(
        "neigh_down",
        "ipn_neigh"
    );
}

void NeighObject::registrySlots()
{
}

void NeighObject::setAttribute(
    const string    &a_name,
    const string    &a_val)
{
    PERROR_OBJ(this, "Internal ERROR, you MUST"
        " use accessors instead 'setAttribute(\"%s\", \"%s\")'\n",
        a_name.c_str(),
        a_val.c_str()
    );
    // die
    assert(false);
}

string NeighObject::getAttribute(const string &name)
{
    PERROR_OBJ(this, "Internal ERROR, you MUST"
        " use accessors instead 'getAttribute(\"%s\")'\n",
        name.c_str()
    );
    // die
    assert(false);
}

int32_t NeighObject::autorun()
{
    int err = 0;

    int32_t recheck_time        = getRecheckTime();
    int32_t recheck_after       = getRecheckAfter();
    int32_t recheck_fail_max    = getRecheckFailMax();

    // setup recheck time if need
    if (recheck_time <= 0){
        recheck_time = NEIGH_RECHECK_TIME;
        setRecheckTime(recheck_time);
    }

    // setup recheck_after time if need
    if (recheck_after <= 0){
        recheck_after = recheck_time;
        setRecheckAfter(recheck_after);
    }

    // setup recheck_fail_max time if need
    if (recheck_fail_max <= 0){
        recheck_fail_max = NEIGH_RECHECK_FAIL_MAX;
        setRecheckFailMax(recheck_after);
    }

    setTimer("+1");
    return err;
}

void NeighObject::timer()
{
    int32_t recheck_fail_max    = getRecheckFailMax();
    int32_t recheck_after       = getRecheckAfter();
    int32_t recheck_time        = getRecheckTime();
    int32_t down_time           = getDownTime();

    recheck_after--;
    if (recheck_after <= 0){
        vector<DomElement *> res;

        getElementsByTagName("ipn_neigh_checkings", res);
        if (recheck_fail_max <= int32_t(res.size())){
            // reset src huid (going to slow path)
            setSrcHuid("");
            // max fail count reached
            deleteChildsSafe();
            // update recheck_time
            recheck_time *= 2;
            setRecheckTime(recheck_time);
            // update recheck_after
            recheck_after = recheck_time;
            // update down time
            down_time += recheck_time * res.size();
            setDownTime(down_time);
            // set neigh down
            setStateDown();
        } else {
            // reset recheck_after
            recheck_after = recheck_time;
            // check neigh
            doCheck();
        }
    }

    // update recheck after
    setRecheckAfter(recheck_after);

    // setup timer
    setTimer("+1");
}

void NeighObject::doCheck()
{
    vector<UserObject *>            users;
    vector<UserObject *>::iterator  users_it;
    map<const string, int>          src_huids;

    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    string          src_huid    = getSrcHuid();

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert ("ipn_server" == server->getTagName());

    // maybe fast path?
    if (src_huid.size()){
        // yea! we known working source huid
        doCheck(src_huid);
        goto out;
    }

    // we have not source huid :(
    // so send searchRoute commands from all local users names
    server->getCreateUsersObject()->getUsers(users);
    for (users_it = users.begin();
        users_it != users.end();
        users_it++)
    {
        // start checking
        UserObject  *user   = *users_it;
        string      huid    = user->getHuid();

        // skip multicast user
        if (MULTICAST_USER_HUID == huid){
            continue;
        }

        // store huid
        src_huids[huid] = 1;
    }

    // check
    doCheck(src_huids);


out:
    return;
}

void NeighObject::doCheck(
    const string &a_src_huid)
{
    map<const string, int> src_huids;
    src_huids[a_src_huid] = 1;
    doCheck(src_huids);
}

void NeighObject::doCheck(
    map<const string, int> &a_src_huids)
{
    map<const string, int>::iterator src_huids_it;

    string dst_huid = getDstHuid();
    string lladdr   = getLLAddr();
    string dev      = getDev();

    DomElement *ipn_neigh_checkings = NULL;

    if (not a_src_huids.size()){
        goto out;
    }

    ipn_neigh_checkings = getDocument()->createElement(
        "ipn_neigh_checkings"
    );
    appendChildSafe(ipn_neigh_checkings);

    for (src_huids_it = a_src_huids.begin();
        src_huids_it != a_src_huids.end();
        src_huids_it++)
    {
        string          src_huid    = src_huids_it->first;
        SkBuffObject    *skb        = NULL;
        DomElement      *ipnoise    = NULL;
        DomElement      *command    = NULL;
        DomElement      *commands   = NULL;
        DomElement      *out_dom    = NULL;

        skb = getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        skb->setFrom(src_huid);
        skb->setTo(dst_huid);
        skb->setToLL(lladdr);
        skb->setOutDevName(dev);

        // create command
        out_dom = skb->getOutDom(1);

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        command = skb->getDocument()->createElement("command");
        command->setAttributeSafe("type", "searchRoute");
        commands = skb->getDocument()->createElement("commands");
        commands->appendChildSafe(command);
        ipnoise->appendChildSafe(commands);

        // append "up" neighs
        NetCommandObject::appendNeighsUp(command);

        // create "ipn_neigh_checking" item
        {
            string      checking_id  = getRand(20);
            DomElement  *checking    = NULL;

            checking = getDocument()->createElement(
                "ipn_neigh_checking"
            );

            // delete element after exit
            checking->setExpired("0");

            // set checking ID
            checking->setAttributeSafe("id",        checking_id);
            checking->setAttributeSafe("src_huid",  src_huid);
            command->setAttributeSafe("id",         checking_id);

            // add checking item
            ipn_neigh_checkings->appendChildSafe(checking);
        }

        // send packet,
        // NOTE: packet will be deleted after sendPacket call
        // tx packet (emitSignal will create clone of element)
        skb->prepare();
        getDocument()->emitSignalAndDelObj(
            "packet_outcoming",
            skb
        );
    }

out:
    return;
}

NeighsObject * NeighObject::getNeighsObject()
{
    NeighsObject    *neighs_obj = NULL;
    DomElement      *parent     = NULL;

    parent = getParentNode();
    if (parent
        && "ipn_neighs" == parent->getTagName())
    {
        neighs_obj = (NeighsObject *)parent;
    }
    return neighs_obj;
}

void NeighObject::getNeighsChecking(
    vector<NeighCheckingObject *>   &a_out)
{
    vector<DomElement *>            elements;
    vector<DomElement *>::iterator  elements_it;

    getElementsByTagName("ipn_neigh_checking", elements);
    for (elements_it = elements.begin();
        elements_it != elements.end();
        elements_it++)
    {
        NeighCheckingObject *neigh_checking = NULL;
        neigh_checking = (NeighCheckingObject *)(*elements_it);
        a_out.push_back(neigh_checking);
    }
}


void NeighObject::setSrcHuid(const string &a_src_huid)
{
    Object::setAttribute("src_huid", a_src_huid);
}

string NeighObject::getSrcHuid()
{
    return Object::getAttribute("src_huid");
}

void NeighObject::setStateDown()
{
    string cur_state;

    //cur_state = getState();
    //if ("down" == cur_state){
    //    // already down
    //    return;
    //}

    // setup state
    Object::setAttributeSafe("state", "down");

    // delete from neigh and route
    // unsetInNeighTable();
    // unsetInRouteTable();

    // emit signal about neigh up state
    getDocument()->emitSignal(
        "neigh_down",
        (DomElement *)this
    );
}

int NeighObject::unsetInNeighTable()
{
    int res = -1;
    string std_out, std_err;
    string huid;
    string lladdr;
    string dev;
    string cur_state;

    huid    = getDstHuid();
    lladdr  = getLLAddr();
    dev     = getDev();

    // neigh
    if (    not huid.empty()
        &&  not lladdr.empty()
        &&  not dev.empty())
    {
        // ok, trying to remove this neigh
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-neigh",
            "del",
            huid.c_str(),
            lladdr.c_str(),
            dev.c_str(),
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    return res;
}

int NeighObject::unsetInRouteTable()
{
    int res = -1;
    string std_out, std_err;
    string huid;
    string dev;
    string src_huid;

    huid        = getDstHuid();
    dev         = getDev();
    src_huid    = getSrcHuid();

    // ok, trying to delete route from this neigh
    {
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-route",
            "del",
            huid.c_str(),
            dev.c_str(),
            src_huid.c_str(),
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    return res;
}

int NeighObject::setInNeighTable()
{
    int res = -1;
    string std_out, std_err;
    string huid;
    string lladdr;
    string dev;

    huid    = getDstHuid();
    lladdr  = getLLAddr();
    dev     = getDev();

    // neigh
    if (    not huid.empty()
        &&  not lladdr.empty()
        &&  not dev.empty())
    {
        // ok, trying to setup this neigh
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-neigh",
            "add",
            huid.c_str(),
            lladdr.c_str(),
            dev.c_str(),
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    return res;
}

int NeighObject::setInRouteTable()
{
    int res = -1;
    string std_out, std_err;
    string huid;
    string dev;
    string src_huid;

    huid        = getDstHuid();
    dev         = getDev();
    src_huid    = getSrcHuid();

    // ok, trying to setup route via this neigh
    {
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-route",
            "add",
            huid.c_str(),
            dev.c_str(),
            src_huid.c_str(),
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    return res;
}

void NeighObject::setStateUp()
{
    //if ("up" == getState()){
    //   // already up
    //   return;
    //}

    // setup state
    Object::setAttributeSafe("state", "up");

    // delete fail counts
    deleteChildsSafe();
    setRecheckAfter(NEIGH_RECHECK_TIME);
    setRecheckTime(NEIGH_RECHECK_TIME);

    // setup neigh and route
    //setInNeighTable();
    //setInRouteTable();

    // emit signal about neigh up state
    getDocument()->emitSignal(
        "neigh_up",
        (DomElement *)this
    );
}

int32_t NeighObject::getRecheckTime()
{
    int32_t res = 0;
    res = atoi(Object::getAttribute("recheck_time").c_str());
    return res;
}

int32_t NeighObject::getRecheckAfter()
{
    int32_t res = 0;
    res = atoi(Object::getAttribute("recheck_after").c_str());
    return res;
}

int32_t NeighObject::getRecheckFailMax()
{
    int32_t res = 0;
    res = atoi(Object::getAttribute("recheck_fail_max").c_str());
    return res;
}

void NeighObject::setRecheckTime(
    const string &a_recheck_time)
{
    Object::setAttribute("recheck_time", a_recheck_time);
}

void NeighObject::setRecheckTime(
    const int &a_recheck_time)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_recheck_time);
    setRecheckTime(buffer);
}

void NeighObject::setRecheckAfter(
    const string &a_recheck_after)
{
    Object::setAttribute("recheck_after", a_recheck_after);
}

void NeighObject::setRecheckAfter(
    const int &a_recheck_after)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_recheck_after);
    setRecheckAfter(buffer);
}

void NeighObject::setRecheckFailMax(
    const string &a_recheck_fail)
{
    Object::setAttribute("recheck_fail_max", a_recheck_fail);
}

void NeighObject::setRecheckFailMax(
    const int &a_recheck_fail)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_recheck_fail);
    setRecheckFailMax(buffer);
}

int32_t NeighObject::getUpTime()
{
    int32_t res = 0;
    res = atoi(Object::getAttribute("uptime").c_str());
    return res;
}

void NeighObject::setUpTime(const string &a_uptime)
{
    Object::setAttribute("uptime", a_uptime);
}

void NeighObject::setUpTime(const int32_t &a_uptime)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_uptime);
    setUpTime(buffer);
}

string NeighObject::getLastCheck()
{
    return Object::getAttribute("last_check");
}

string NeighObject::getLastRecv()
{
    return Object::getAttribute("last_recv");
}

void NeighObject::setLastRecv(const string &last_recv)
{
    Object::setAttribute("last_recv", last_recv);
}

int32_t NeighObject::getDownTime()
{
    int32_t res = 0;
    res = atoi(Object::getAttribute("down_time").c_str());
    return res;
}

void NeighObject::setDownTime(const string &down_time)
{
    Object::setAttribute("down_time", down_time);
}

void NeighObject::setDownTime(
    const int &a_down_time)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_down_time);
    setDownTime(buffer);
}

string NeighObject::getDstHuid()
{
    string huid;
    huid = Object::getAttribute("dst_huid");
    return huid;
}

void NeighObject::setDstHuid(const string &a_dst_huid)
{
    Object::setAttribute("dst_huid", a_dst_huid);
}

string NeighObject::getState()
{
    return Object::getAttribute("state");
}

bool NeighObject::isUp()
{
    int ret = 0;
    string state;

    state = getState();
    if ("up" == state){
        ret = 1;
    }
    return ret;
}

string NeighObject::getLLAddr()
{
    return Object::getAttribute("lladdr");
}

void NeighObject::setLLAddr(const string &lladdr)
{
    Object::setAttribute("lladdr", lladdr);
}

string NeighObject::getDev()
{
    return Object::getAttribute("dev");
}

void NeighObject::setDev(const string &dev)
{
    Object::setAttribute("dev", dev);
}

string NeighObject::getLinkAddress()
{
    return getLLAddr();
}

string NeighObject::getLinkName()
{
    string dev = getDev();
    string ret = dev;

    if ("udp_v4_" == dev.substr(0, 7)){
        ret = "udp_v4";
    } else if ("tcp_v4_" == dev.substr(0, 7)){
        ret = "tcp_v4";
    } else if ("icmp_v4_" == dev.substr(0, 8)){
        ret = "icmp_v4";
    } else {
        ret = "internal_error";
    }

    return ret;
}

string NeighObject::getMetric()
{
    return Object::getAttribute("metric");
}

void NeighObject::setMetric(const string &metric)
{
    Object::setAttribute("metric", metric);
}

void NeighObject::slot(
    const string    &, // signal_name
    DomElement      *) // object
{
}

/*
 *  Check what neigh have local (not-internet) address
 *  (it used for select near neigh)
 */
bool NeighObject::is_local()
{
    struct in_addr addr;

    int res, ret                = 0;
    string::size_type  pos      = string::npos;
    string             ip       = "";
    string             lladdr   = "";
    string             dev      = getDev();

    if (dev.empty()){
        goto out;
    }

    if (    ("udp"     != dev.substr(0, 3))
        &&  ("tcp"     != dev.substr(0, 3))
        &&  ("icmp"    != dev.substr(0, 4)))
    {
        goto out;
    }

    memset(&addr, 0x00, sizeof(addr));
    lladdr = getLLAddr();

    pos = lladdr.find(":");
    if (pos != string::npos){
        ip = lladdr.substr(0, pos);
    } else {
        ip = lladdr;
    }

    res = inet_aton(ip.c_str(), &addr);
    if (!res){
        goto fail;
    }

    // convert in host format
    addr.s_addr = ntohl(addr.s_addr);

    // IANA-reserved private IPv4 network ranges
    //                                      Start       End
    // 24-bit  Block (/8 prefix, 1 × A)     10.0.0.0    10.255.255.255
    // 20-bit Block (/12 prefix, 16 × B)    172.16.0.0  172.31.255.255
    // 16-bit Block (/16 prefix, 256 × C)   192.168.0.0 192.168.255.255
    if (((addr.s_addr & 0xff000000) >> 24) == 10){
        ret = 1;
        goto out;
    } else if (((addr.s_addr & 0xff000000) >> 24) == 172
        &&    (((addr.s_addr & 0x00ff0000) >> 16) == 16))
    {
        ret = 1;
        goto out;
    } else if (((addr.s_addr & 0xff000000) >> 24) == 192
        &&    (((addr.s_addr & 0x00ff0000) >> 16) == 168))
    {
        ret = 1;
        goto out;
    }

out:
    return ret;
fail:
    goto out;
}

int NeighObject::cmp(NeighObject &right)
{
    int ret = 0;
    string  right_dev       = right.getDev();
    string  right_state     = right.getState();
    int32_t right_uptime    = right.getUpTime();
    int32_t right_downtime  = right.getDownTime();
    double  right_upindex   = 0;

    string  dev         = getDev();
    string  state       = getState();
    int32_t uptime      = getUpTime();
    int32_t downtime    = getDownTime();
    double  upindex     = 0;

    do {
        // sort by state
        if (!isUp() && right.isUp()){
            ret = -1;
            break;
        }
        if (isUp() && !right.isUp()){
            ret = +1;
            break;
        }

        // sort by "dev" exist
        if (dev.empty() && not right_dev.empty()){
            ret = -1;
            break;
        }
        if (not dev.empty() && right_dev.empty()){
            ret = +1;
            break;
        }

        // check prio by address
        if (!is_local() && right.is_local()){
            ret = -1;
            break;
        }

        if (is_local() && !right.is_local()){
            ret = +1;
            break;
        }

        // sort by uptime/downtime
        {
            // left
            if (uptime < 0){
                uptime = 0;
            }
            if (downtime < 0){
                downtime = 0;
            }
            if (downtime > 0){
                upindex = uptime/downtime;
            }

            // right
            if (right_uptime < 0){
                right_uptime = 0;
            }
            if (right_downtime < 0){
                right_downtime = 0;
            }
            if (right_downtime){
                right_upindex = right_uptime/right_downtime;
            }

            if (upindex < right_upindex){
                ret = -1;
                break;
            } else if (upindex > right_upindex){
                ret = +1;
                break;
            }
        }

        // sort by devices
        if (not dev.empty() && not right_dev.empty()){
            if (     "udp" != dev.substr(0, 3)
                &&   "udp" == right_dev.substr(0, 3))
            {
                ret = -1;
                break;
            }
            if (    "udp" == dev.substr(0, 3)
                &&  "udp" != right_dev.substr(0, 3))
            {
                ret = +1;
                break;
            }
        }

    } while (0);

    /*
    PWARN("dev:       '%s' => '%s'\n",
        dev.c_str(),           right_dev.c_str());
    PWARN("state:     '%s' => '%s'\n",
        state.c_str(),         right_state.c_str());
    PWARN("uptime:    '%s' => '%s'\n",
        uptime.c_str(),        right_uptime.c_str());
    PWARN("lladdr:    '%s' => '%s'\n",
        getLLAddr().c_str(),   right->getLLAddr().c_str());
    PWARN("isLocal:   '%d' => '%d'\n",
        is_local(this),        is_local(right));
    PWARN("RES: '%d'\n", ret);
    */

    return ret;
}

bool NeighObject::operator > (NeighObject &right)
{
    return (cmp(right) > 0);
}

