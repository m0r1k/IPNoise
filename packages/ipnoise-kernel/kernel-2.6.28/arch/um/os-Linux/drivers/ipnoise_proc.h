#ifndef IPNOISE_PROC
#define IPNOISE_PROC

#include <linux/ipnoise.h>
#include <linux/proc_fs.h>
#include "os.h"

#define MODULE_NAME         "ipnoise_proc"

#define MDEBUG(fmt, args...) \
        PDEBUG(MODULE_NAME, fmt, ## args);

#define MERROR(fmt, args...) \
        PERROR(MODULE_NAME, fmt, ## args);

#define MWARN(fmt, args...) \
        PWARN(MODULE_NAME, fmt, ## args);

#define MINFO(fmt, args...) \
        PINFO(MODULE_NAME, fmt, ## args);

static int proc_home_dir_read(char *page, char **start, off_t off,
    int count, int *eof, void *data);

static int proc_root_dir_read(char *page, char **start, off_t off,
    int count, int *eof, void *data);

int unregister_procfs_files(void);
int register_procfs_files(void);

#endif

