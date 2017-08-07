/*
 * Roman E. Chechnev Jun, 2009
 */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_arp.h>
#include <ipnoise-common/ipnoise.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

#define MAX_ADDR_LEN 32 // from <linux/netdevice.h>  (MORIK)

extern struct hwtype ipnoise_hwtype;

/* Display an IPNoise UDP address in readable format. */
static char *pr_ipnoise(unsigned char *ptr)
{
    static char buff[MAX_ADDR_LEN];

    snprintf(buff, MAX_ADDR_LEN, "%s", ptr);
    return (buff);
}

/* Input an IPNoise address. */
static int in_ipnoise(char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    char c, *orig;
    int i;
    unsigned val;

    sap->sa_family = ipnoise_hwtype.type;
    ptr = sap->sa_data;

    printf("MORIK: %s\n", bufp);

    if (strlen(bufp) > IPNOISE_ALEN){
        errno = EINVAL;
        return (-1);
    }
    strcpy(ptr, bufp);

    return (0);
}

struct hwtype ipnoise_hwtype =
{
    "ipnoise", "IPNoise ll address", ARPHRD_IPNOISE, IPNOISE_ALEN,
    pr_ipnoise, in_ipnoise, NULL
};


