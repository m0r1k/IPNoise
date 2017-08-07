#include "command.h"
#include "main.h"

extern Instance *inst;

Command::Command()
{
};

Command::~Command()
{

};

int Command::init()
{
    connect(
        this,   SIGNAL(searchRoute(sk_buff *)),
        this,   SLOT(searchRouteSlot(sk_buff *))
    );

    connect(
        this,   SIGNAL(searchRouteAnswer(sk_buff *)),
        this,   SLOT(searchRouteAnswerSlot(sk_buff *))
    );

    return 0;
}

void Command::recvCommandSlot(sk_buff *skb)
{
    PDEBUG(5, "new command was received\n");
    PDEBUG(5, "from:       '%s'\n", skb->from.toAscii().data());
    PDEBUG(5, "to:         '%s'\n", skb->to.toAscii().data());
    PDEBUG(5, "inbuffer:   '%s'\n", skb->inbuffer.toAscii().data());

    int i;

    QString     errorMsg;
    int         errorLine;
    int         errorColumn;
    bool        res;

    QDomNodeList commands;

    res = skb->indom.setContent(skb->inbuffer,
        &errorMsg, &errorLine, &errorColumn);
    if (!res){
        PERROR("Cannot parse xml: '%s', error: '%s',"
            " line: '%d', column: '%d'\n",
            skb->inbuffer.toAscii().data(),
            errorMsg.toAscii().data(),
            errorLine,
            errorColumn
        );
        goto fail;
    }

    skb->indomroot  = skb->indom.documentElement();
    commands        = skb->indom.elementsByTagName("command");

    if (skb->indomroot.tagName() != "ipnoise"){
        PERROR("Cannot parse xml: '%s', root element is not 'ipnoise'\n",
            skb->inbuffer.toAscii().data());
        goto fail;
    }

    for (i = 0; i < commands.size(); i++){
        QDomNode node = commands.at(i);
        if (!node.isElement()){
            continue;
        }
        QDomElement element = node.toElement();
        QString cmd_type    = element.attribute("type");

        if (cmd_type == "searchRoute"){
            skb->indomroot = element;
            emit searchRoute(skb);
        } else if (cmd_type == "searchRouteAnswer"){
            skb->indomroot = element;
            emit searchRouteAnswer(skb);
        } else {
            PERROR("Cannot parse xml: '%s', unknown command: '%s'\n",
                skb->inbuffer.toAscii().data(),
                cmd_type.toAscii().data());
            goto fail;
        }
    }

ret:
    delete skb;
    return;
fail:
    goto ret;
};

void Command::sendCommandSlot(sk_buff *skb)
{
    PDEBUG(5, "prepare '%s' command for send\n",
        skb->getOutCommandType().toAscii().data());
    PDEBUG(5, "from:    '%s'\n", skb->from.toAscii().data());
    PDEBUG(5, "to:      '%s'\n", skb->to.toAscii().data());
    PDEBUG(5, "to_ll:   '%s'\n", skb->to_ll.toAscii().data());

    QDomElement root = skb->outdom.createElement("ipnoise");
    skb->outdom.appendChild(root);
    root.appendChild(skb->outdomroot);

    skb->outbuffer  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    skb->outbuffer += skb->outdom.toString(0);

    inst->server->sendPacket(skb);
};

void Command::doSearchRouteSlot(sk_buff *skb)
{
    skb->outdomroot = skb->outdom.createElement("command");
    skb->outdomroot.setAttribute("type", "searchRoute");
    emit sendCommandSlot(skb);
}

void Command::searchRouteAnswerSlot(sk_buff *skb)
{
    QString tmp;
    PDEBUG(5, "searchRouteAnswer command was received\n");
    PDEBUG(5, "from:        '%s'\n", skb->from.toAscii().data());
    PDEBUG(5, "from_ll:     '%s'\n", skb->from_ll.toAscii().data());
    PDEBUG(5, "to:          '%s'\n", skb->to.toAscii().data());
    PDEBUG(5, "inbuffer:    '%s'\n", skb->inbuffer.toAscii().data());
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

