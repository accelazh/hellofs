#include "khellofs.h"

DEFINE_MUTEX(hellofs_sb_lock);

struct file_system_type hellofs_fs_type = {
    .owner = THIS_MODULE,
    .name = "hellofs",
    .mount = hellofs_mount,
    .kill_sb = hellofs_kill_superblock,
    .fs_flags = FS_REQUIRES_DEV,
};

const struct super_operations hellofs_sb_ops = {
    .destroy_inode = hellofs_destroy_inode,
    .put_super = hellofs_put_super,
};

const struct inode_operations hellofs_inode_ops = {
    .create = hellofs_create,
    .mkdir = hellofs_mkdir,
    .lookup = hellofs_lookup,
};

const struct file_operations hellofs_dir_operations = {
    .owner = THIS_MODULE,
    .readdir = hellofs_readdir,
};

const struct file_operations hellofs_file_operations = {
    .read = hellofs_read,
    .write = hellofs_write,
};

struct kmem_cache *hellofs_inode_cache = NULL;

static int __init hellofs_init(void)
{
    int ret;

    hellofs_inode_cache = kmem_cache_create("hellofs_inode_cache",
                                         sizeof(struct hellofs_inode),
                                         0,
                                         (SLAB_RECLAIM_ACCOUNT| SLAB_MEM_SPREAD),
                                         NULL);
    if (!hellofs_inode_cache) {
        return -ENOMEM;
    }

    ret = register_filesystem(&hellofs_fs_type);
    if (likely(0 == ret)) {
        printk(KERN_INFO "Sucessfully registered hellofs\n");
    } else {
        printk(KERN_ERR "Failed to register hellofs. Error code: %d\n", ret);
    }

    return ret;
}

static void __exit hellofs_exit(void)
{
    int ret;

    ret = unregister_filesystem(&hellofs_fs_type);
    kmem_cache_destroy(hellofs_inode_cache);

    if (likely(ret == 0)) {
        printk(KERN_INFO "Sucessfully unregistered hellofs\n");
    } else {
        printk(KERN_ERR "Failed to unregister hellofs. Error code: %d\n",
               ret);
    }
}

module_init(hellofs_init);
module_exit(hellofs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("accelazh");
