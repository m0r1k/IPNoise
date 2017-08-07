/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#ifndef SERVER_OBJECT_ABSTRACT_H
#define SERVER_OBJECT_ABSTRACT_H

#include <ipnoise-common/log.h>

// Don't do any include here!
// use libxml2/domElement.h instead

virtual int startServer()
{
    int err = -1;
    PERROR_OBJ(this, "Method is not implemented\n");
    return err;
}

/*
virtual Command *getCmd()
{
    PERROR_OBJ(this, "Method is not implemented\n");
    return NULL;
}
*/

virtual bool isRunning()
{
    PERROR_OBJ(this, "Method is not implemented\n");
    return false;
}

#endif

