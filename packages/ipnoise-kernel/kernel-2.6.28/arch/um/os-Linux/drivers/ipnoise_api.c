/*
 *
 * Apr 15, 2009
 * Roman E. Chechnev <kernel|d|o|g|chechnev.ru>
 *
 */

#include <linux/module.h>
#include <net/sock.h>

#include "ipnoise_api.h"

#include <linux/mm.h>
#include <linux/types.h>
#include <linux/errno.h>

#include "um_malloc.h"
#include "os.h"

#include "ipnoise_proc.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(MODULE_AUTH);
MODULE_DESCRIPTION(MODULE_DESCR);
MODULE_VERSION(MODULE_VER);

static int __init ipnoise_api_init(void)
{
    // create proc files
    register_procfs_files();
    return 0;
}

static void __exit ipnoise_api_exit(void)
{
    // delete procfs files
    unregister_procfs_files();
    MINFO("module unloaded\n");
}

module_init(ipnoise_api_init);
module_exit(ipnoise_api_exit);

