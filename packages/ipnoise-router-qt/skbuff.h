#ifndef SKBUFF_H
#define SKBUFF_H

#include <QDomDocument>
#include <QDomElement>
#include <QString>

class sk_buff
{
    public:
        sk_buff();
        ~sk_buff();

        // in
        QString         inbuffer;
        QString         indev_name;     // input device name
        int             indev_index;    // input device index
        QString         from_ll;        // link level src addr
        QString         from;           // net  level src addr

        QDomDocument    indom;          // input command dom model
        QDomElement     indomroot;      // input command root element

        // out
        QString         outbuffer;      // output buffer
        QString         outdev_name;    // output device name
        int             outdev_index;   // output device index
        QString         to_ll;          // link level dst addr 
        QString         to;             // net  level dst addr

        QDomDocument    outdom;         // output command dom model
        QDomElement     outdomroot;     // output command root element

        // methods
        QString getInCommandType();     // return input command type
        QString getOutCommandType();    // return output command type


};

#endif

