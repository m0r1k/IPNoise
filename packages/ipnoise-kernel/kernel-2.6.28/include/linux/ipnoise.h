#ifndef IPNOISE_H
#define IPNOISE_H

// ------------------ logging ------------------

#include "um_malloc.h"
#include <shared/os.h>
#include <linux/slab.h>

#define IPNOISE_KLOG_FILE       "/tmp/ipnoise.log.%d"
#define IPNOISE_KLOG_FILE_MODE  0644
#define IPNOISE_KLOG_BUF_SIZE   65535
#define IPNOISE_FNAME_BUF_SIZE  256

#define PLOG(fmt, args...)                                                  \
    do {                                                                    \
        int ___fd;                                                          \
        struct openflags ___flags;                                          \
        char *___buffer1 = kzalloc(IPNOISE_FNAME_BUF_SIZE, UM_GFP_KERNEL);  \
        char *___buffer2 = kzalloc(IPNOISE_KLOG_BUF_SIZE,  UM_GFP_KERNEL);  \
        if (!___buffer1 || !___buffer2){                                    \
            break;                                                          \
        }                                                                   \
        memset(&___flags, 0x00, sizeof(___flags));                          \
        ___flags.c = 1;                                                     \
        ___flags.a = 1;                                                     \
        ___flags.w = 1;                                                     \
        snprintf(___buffer1, IPNOISE_FNAME_BUF_SIZE,                        \
            IPNOISE_KLOG_FILE, os_getpid());                                \
        ___fd = os_open_file(___buffer1, ___flags,                          \
            IPNOISE_KLOG_FILE_MODE);                                        \
        if (___fd >= 0){                                                    \
            snprintf(___buffer2, IPNOISE_KLOG_BUF_SIZE, fmt, ## args);      \
            os_write_file(___fd, ___buffer2, strlen(___buffer2));           \
            os_close_file(___fd);                                           \
        }                                                                   \
        kfree(___buffer2);                                                  \
        kfree(___buffer1);                                                  \
    } while (0);

#define PINFO(who, fmt, args...) \
    PLOG(KERN_INFO "[ " who " ] %s: " fmt, MODULE_NAME, ## args)

#define PERROR(who, fmt, args...) \
	PLOG(KERN_ERR "[ " who " ] -----------[ start cut here ]-----------\n");\
	PLOG(KERN_ERR "[ " who " ] ERROR: at %s:%d %s()\n", __FILE__,           \
		__LINE__, __FUNCTION__);                                            \
    PLOG(KERN_ERR "[ " who " ] DESCR: " fmt, ## args);                      \
	dump_stack();                                                           \
	PLOG(KERN_ERR "[ " who " ] -----------[ stop cut here ]------------\n");


#define PDEBUG(who, fmt, args...) \
	PLOG(KERN_ERR "[ " who " ] DEBUG: at %s:%d %s()\n", __FILE__,           \
		__LINE__, __FUNCTION__);                                            \
    PLOG(KERN_ERR "[ " who " ] DESCR:   " fmt, ## args);

#define PWARN(who, fmt, args...) \
    PLOG(KERN_WARNING "[ " who " ] %s: " fmt, MODULE_NAME, ## args)

/*
 * Roman E. Chechnev Jan, 2009
 */
static inline void ipnoise_hexdump(
    const char      *who,
    unsigned char   *data,
    int             len)
{
    int i, m = 0;
    unsigned char *buffer = NULL;
    unsigned char line[20];

    if (len <= 0){
        goto fail;
    }

    // alloc buffer
    buffer = kzalloc(len*10, 0);

    for (i = 0, m = 0; i < len + (16 - (len % 16)); i++, m++){
        if (i < len){
            sprintf(buffer + strlen(buffer), "%2.2x ", data[i]);
            if (data[i] > 32 && data[i] < 127){
                line[m] = data[i];
            } else {
                line[m] = '.';
            }
        } else {
            sprintf(buffer + strlen(buffer), "   ");
            line[m] = ' ';
        }
        if (i && ((i + 1) % 8 == 0)){
            sprintf(buffer + strlen(buffer), " ");
        }
        if (i && ((i + 1) % 16 == 0)){
            line[m + 1] = '\00';
            m = -1;
            sprintf(buffer + strlen(buffer), " %s\n", line);
        }
    }
    sprintf(buffer + strlen(buffer), "\n");
//    PDEBUG(who, "%s\n", buffer);
    PLOG(KERN_ERR "[ %s ] DESCR:   %s", who, buffer);
    kfree(buffer);

out:
    return;
fail:
    goto out;
};

#endif

