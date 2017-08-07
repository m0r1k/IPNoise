/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "objects/usersObject.h"
#include "objects/userObject.h"

#include "objects/sessionLoObject.h"

SessionLoObject * SessionLoObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionLoObject(a_node, a_doc, a_tagname);
}

SessionLoObject::SessionLoObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   SessionObject(a_node, a_doc, a_tagname)
{
    setApi("ipnoise");
    setState("logged");
};

SessionLoObject::~SessionLoObject()
{
};

int32_t SessionLoObject::autorun()
{
    int err         = 0;
    int priveleges  = 0;

    priveleges = getDocument()->getPriveleges();

    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

ssize_t SessionLoObject::xmit(
    DomElement      *a_data,
    const int32_t   &a_delete_after_send)
{
    PERROR("drop xmit packet:\n'%s'\n",
        a_data->serialize(1).c_str()
    );
    if (a_delete_after_send){
        DomDocument::deleteElement(a_data);
        a_data = NULL;
    }
    return -1;
}

