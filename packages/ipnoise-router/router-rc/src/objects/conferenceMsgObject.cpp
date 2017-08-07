/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/conferenceMsgObject.h"

ConferenceMsgObject * ConferenceMsgObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ConferenceMsgObject(a_node, a_doc, a_tagname);
}

ConferenceMsgObject::ConferenceMsgObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

ConferenceMsgObject::~ConferenceMsgObject()
{
};

int32_t ConferenceMsgObject::autorun()
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

void ConferenceMsgObject::setId(const string &a_id)
{
    setAttribute("id", a_id);
}

string ConferenceMsgObject::getId()
{
    return getAttribute("id");
}

ConferenceMsgsObject * ConferenceMsgObject::getConferenceMsgsObject()
{
    ConferenceMsgsObject *conference_msgs = NULL;

    conference_msgs = (ConferenceMsgsObject *)getParentNode();
    assert ("ipn_conference_msgs" == conference_msgs->getTagName());

    return conference_msgs;
}

