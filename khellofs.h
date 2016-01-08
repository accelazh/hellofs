#ifndef __KHELLOFS_H__
#define __KHELLOFS_H__

/* khellofs.h defines symbols to work in kernel space */

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/module.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>

#include "hellofs.h"

/* Declare operations to be hooked to VFS */

extern struct file_system_type hellofs_fs_type;
extern const struct super_operations hellofs_sb_ops;
extern const struct inode_operations hellofs_inode_ops;
extern const struct file_operations hellofs_dir_operations;
extern const struct file_operations hellofs_file_operations;

struct dentry *hellofs_mount(struct file_system_type *fs_type,
                              int flags, const char *dev_name,
                              void *data);
void hellofs_kill_superblock(struct super_block *sb);

void hellofs_destroy_inode(struct inode *inode);
void hellofs_put_super(struct super_block *sb);

int hellofs_create(struct inode *dir, struct dentry *dentry,
                    umode_t mode, bool excl);
struct dentry *hellofs_lookup(struct inode *parent_inode,
                               struct dentry *child_dentry,
                               unsigned int flags);
int hellofs_mkdir(struct inode *dir, struct dentry *dentry,
                   umode_t mode);

int hellofs_readdir(struct file *filp, void *dirent, filldir_t filldir);

ssize_t hellofs_read(struct file * filp, char __user * buf, size_t len,
                      loff_t * ppos);
ssize_t hellofs_write(struct file * filp, const char __user * buf, size_t len,
                       loff_t * ppos);

extern struct kmem_cache *hellofs_inode_cache;

/* Helper functions */

// To translate VFS superblock to hellofs superblock
static inline struct hellofs_superblock *HELLOFS_SB(struct super_block *sb) {
    return sb->s_fs_info;
}
static inline struct hellofs_inode *HELLOFS_INODE(struct inode *inode) {
    return inode->i_private;
}

static inline uint64_t HELLOFS_INODES_PER_BLOCK(struct super_block *sb) {
    struct hellofs_superblock *hellofs_sb;
    hellofs_sb = HELLOFS_SB(sb);
    return HELLOFS_INODES_PER_BLOCK_HSB(hellofs_sb);
}

// Given the inode_no, calcuate which block in inode table contains the corresponding inode
static inline uint64_t HELLOFS_INODE_BLOCK_OFFSET(struct super_block *sb, uint64_t inode_no) {
    struct hellofs_superblock *hellofs_sb;
    hellofs_sb = HELLOFS_SB(sb);
    return inode_no / HELLOFS_INODES_PER_BLOCK_HSB(hellofs_sb);
}
static inline uint64_t HELLOFS_INODE_BYTE_OFFSET(struct super_block *sb, uint64_t inode_no) {
    struct hellofs_superblock *hellofs_sb;
    hellofs_sb = HELLOFS_SB(sb);
    return (inode_no % HELLOFS_INODES_PER_BLOCK_HSB(hellofs_sb)) * sizeof(struct hellofs_inode);
}

static inline uint64_t HELLOFS_DIR_MAX_RECORD(struct super_block *sb) {
    struct hellofs_superblock *hellofs_sb;
    hellofs_sb = HELLOFS_SB(sb);
    return hellofs_sb->blocksize / sizeof(struct hellofs_dir_record);
}

// From which block does data blocks start
static inline uint64_t HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO(struct super_block *sb) {
    struct hellofs_superblock *hellofs_sb;
    hellofs_sb = HELLOFS_SB(sb);
    return HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(hellofs_sb);
}

void hellofs_save_sb(struct super_block *sb);

// functions to operate inode
void hellofs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hellofs_inode *hellofs_inode);
int hellofs_alloc_hellofs_inode(struct super_block *sb, uint64_t *out_inode_no);
struct hellofs_inode *hellofs_get_hellofs_inode(struct super_block *sb,
                                                uint64_t inode_no);
void hellofs_save_hellofs_inode(struct super_block *sb,
                                struct hellofs_inode *inode);
int hellofs_add_dir_record(struct super_block *sb, struct inode *dir,
                           struct dentry *dentry, struct inode *inode);
int hellofs_alloc_data_block(struct super_block *sb, uint64_t *out_data_block_no);
int hellofs_create_inode(struct inode *dir, struct dentry *dentry,
                         umode_t mode);

#endif /*__KHELLOFS_H__*/