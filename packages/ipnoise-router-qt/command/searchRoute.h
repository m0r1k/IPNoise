#ifndef SEARCH_ROUTE_H
#define SEARCH_ROUTE_H

void Command::doSearchRouteSlot(sk_buff *skb)
{
    skb->outdomroot = skb->outdom.createElement("command");
    skb->outdomroot.setAttribute("type", "searchRoute");
    emit sendCommandSlot(skb);
}

void Command::searchRouteAnswerSlot(sk_buff *skb)
{
    QString tmp;
    PDEBUG(15, "searchRouteAnswer command was received\n");
    PDEBUG(15, "from:        '%s'\n", skb->from.toAscii().data());
    PDEBUG(15, "from_ll:     '%s'\n", skb->from_ll.toAscii().data());
    PDEBUG(15, "to:          '%s'\n", skb->to.toAscii().data());
    PDEBUG(15, "inbuffer:    '%s'\n", skb->inbuffer.toAscii().data());
}

void Command::searchRouteSlot(sk_buff *skb)
{
    QString tmp;
    PDEBUG(15, "searchRoute command was received\n");
    PDEBUG(15, "from:        '%s'\n", skb->from.toAscii().data());
    PDEBUG(15, "from_ll:     '%s'\n", skb->from_ll.toAscii().data());
    PDEBUG(15, "to:          '%s'\n", skb->to.toAscii().data());
    PDEBUG(15, "inbuffer:    '%s'\n", skb->inbuffer.toAscii().data());

    // prepare answer, create new skb
    sk_buff *new_skb = new sk_buff(*skb);

    // prepare fields
    tmp             = skb->to;
    new_skb->to     = skb->from;
    new_skb->from   = tmp;
    new_skb->to_ll  = skb->from_ll;

    // clear interface information (for send to all interfaces)
    new_skb->outdev_name    = "";
    new_skb->outdev_index   = -1;

    new_skb->outdomroot = skb->outdom.createElement("command");
    new_skb->outdomroot.setAttribute("type", "searchRouteAnswer");

    emit sendCommandSlot(new_skb);
}

#endif

