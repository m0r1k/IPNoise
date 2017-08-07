#include <linux/ipnoise.h>

#include "ipnoise_proc.h"

static struct proc_dir_entry *root_ipnoise_dir;
static struct proc_dir_entry *host_ipnoise_dir;

#define IPNOISE_DIR ".ipnoise"

static int proc_home_dir_read(char *page, char **start, off_t off,
    int count, int *eof, void *data)
{
    int err = 0;
    char *home_dir = os_getenv("HOME");
    if (home_dir != NULL){
        err = sprintf(page, "%s\n", home_dir);
    } else {
        err = sprintf(page, "");
    }
    return err;
}

static int proc_root_dir_read(char *page, char **start, off_t off,
    int count, int *eof, void *data)
{
    int err = 0;
    char *home_dir = os_getenv("HOME");
    if (home_dir != NULL){
        err = sprintf(page, "%s/"IPNOISE_DIR"\n", home_dir);
    } else {
        err = sprintf(page, "");
    }
    return err;
}

int unregister_procfs_files(void)
{
    if (root_ipnoise_dir != NULL){
        if (host_ipnoise_dir != NULL){
            remove_proc_entry("home_dir", host_ipnoise_dir);
            remove_proc_entry("root_dir", host_ipnoise_dir);
        }
        remove_proc_entry("host_os",  root_ipnoise_dir);
    }
}

int register_procfs_files(void)
{
    int size, err = -1;
    struct proc_dir_entry *entry;
    char *home_dir = os_getenv("HOME");
    char *tmp;

    if (home_dir == NULL){
        MERROR("Cannot read HOME env variable\n");
        goto fail;
    }

    // create profile dir
    size = strlen(home_dir) + strlen(IPNOISE_DIR) + 10;
    tmp  = kzalloc(size, 0);
    if (tmp == NULL){
        MERROR("kzalloc() failed, was needed: '%d' byte(s)\n", size);
        goto fail;
    }
    snprintf(tmp, size, "%s/"IPNOISE_DIR"", home_dir);
    os_mkdir(tmp, 0750);
    kfree(tmp);

    // create proc root dir
    root_ipnoise_dir = proc_mkdir("ipnoise", NULL);
    if (!root_ipnoise_dir){
        goto fail;
    }

    // create proc host_os dir
    host_ipnoise_dir = proc_mkdir("host_os", root_ipnoise_dir);
    if (!host_ipnoise_dir){
        goto fail;
    }

    // create proc file
    entry = create_proc_entry("home_dir", 0444, host_ipnoise_dir);
    if (entry){
        entry->data = NULL;
        entry->read_proc = proc_home_dir_read;
    }
    // create proc file
    entry = create_proc_entry("root_dir", 0444, host_ipnoise_dir);
    if (entry){
        entry->data = NULL;
        entry->read_proc = proc_root_dir_read;
    }

    err = 0;

ret:
    return err;
fail:
    goto ret;
};

