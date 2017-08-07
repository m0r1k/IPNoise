#include "objects/userObject.h"
#include "libxml2/domDocument.h"
#include "libxml2/domElement.h"

#include "contactAndItem.h"

ContactAndItem::ContactAndItem(
    xmlNodePtr      _node,
    DomDocument     *_doc,
    const string    &_tagname)
    :   Object(_node, _doc, _tagname)
{
};

ContactAndItem::~ContactAndItem()
{
};

