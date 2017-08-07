/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class IPNoiseObject;

#ifndef IPNOISE_OBJECT_H
#define IPNOISE_OBJECT_H

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/serverObject.h"
#include "objects/signalsObject.h"
#include "objects/slotsObject.h"
#include "objects/tmpObject.h"

class IPNoiseObject
    :   public Object
{
    public:
        IPNoiseObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_ipnoise"
        );
        virtual ~IPNoiseObject();

        // internal
        virtual IPNoiseObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_ipnoise"
        );
        virtual int32_t autorun();

        // internal
        ServerObject        * getServerObject();
        SignalsObject       * getSignalsObject();
        SlotsObject         * getSlotsObject();
        TmpObject           * getTmpObject();
};

#endif

