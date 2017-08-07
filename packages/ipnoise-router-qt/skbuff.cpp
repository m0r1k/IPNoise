#include "skbuff.h"

sk_buff::sk_buff()
{
    inbuffer        = "";
    indev_name      = "";
    indev_index     = -1;
    from_ll         = "";
    from            = "";

    outbuffer       = "";
    outdev_name     = "";
    outdev_index    = -1;
    to_ll           = "";
    to              = "";
};

sk_buff::~sk_buff()
{

};

QString sk_buff::getInCommandType()
{
    return indomroot.attribute("type");
};

QString sk_buff::getOutCommandType()
{
    return outdomroot.attribute("type");
};

