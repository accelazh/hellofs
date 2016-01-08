#include "khellofs.h"

void hellofs_destroy_inode(struct inode *inode) {
    struct hellofs_inode *hellofs_inode = HELLOFS_INODE(inode);

    printk(KERN_INFO "Freeing private data of inode %p (%lu)\n",
           hellofs_inode, inode->i_ino);
    kmem_cache_free(hellofs_inode_cache, hellofs_inode);
}

void hellofs_fill_inode(struct super_block *sb, struct inode *inode,
                        struct hellofs_inode *hellofs_inode) {
    inode->i_mode = hellofs_inode->mode;
    inode->i_sb = sb;
    inode->i_ino = hellofs_inode->inode_no;
    inode->i_op = &hellofs_inode_ops;
    // TODO hope we can use hellofs_inode to store timespec
    inode->i_atime = inode->i_mtime 
                   = inode->i_ctime
                   = CURRENT_TIME;
    inode->i_private = hellofs_inode;    
    
    if (S_ISDIR(hellofs_inode->mode)) {
        inode->i_fop = &hellofs_dir_operations;
    } else if (S_ISREG(hellofs_inode->mode)) {
        inode->i_fop = &hellofs_file_operations;
    } else {
        printk(KERN_WARNING
               "Inode %lu is neither a directory nor a regular file",
               inode->i_ino);
        inode->i_fop = NULL;
    }

    /* TODO hellofs_inode->file_size seems not reflected in inode */
}

/* TODO I didn't implement any function to dealloc hellofs_inode */
int hellofs_alloc_hellofs_inode(struct super_block *sb, uint64_t *out_inode_no) {
    struct hellofs_superblock *hellofs_sb;
    struct buffer_head *bh;
    uint64_t i;
    int ret;
    char *bitmap;
    char *slot;
    char needle;

    hellofs_sb = HELLOFS_SB(sb);

    mutex_lock(&hellofs_sb_lock);

    bh = sb_bread(sb, HELLOFS_INODE_BITMAP_BLOCK_NO);
    BUG_ON(!bh);

    bitmap = bh->b_data;
    ret = -ENOSPC;
    for (i = 0; i < hellofs_sb->inode_table_size; i++) {
        slot = bitmap + i / BITS_IN_BYTE;
        needle = 1 << (i % BITS_IN_BYTE);
        if (0 == (*slot & needle)) {
            *out_inode_no = i;
            *slot |= needle;
            hellofs_sb->inode_count += 1;
            ret = 0;
            break;
        }
    }

    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
    hellofs_save_sb(sb);

    mutex_unlock(&hellofs_sb_lock);
    return ret;
}

struct hellofs_inode *hellofs_get_hellofs_inode(struct super_block *sb,
                                                uint64_t inode_no) {
    struct buffer_head *bh;
    struct hellofs_inode *inode;
    struct hellofs_inode *inode_buf;

    bh = sb_bread(sb, HELLOFS_INODE_TABLE_START_BLOCK_NO + HELLOFS_INODE_BLOCK_OFFSET(sb, inode_no));
    BUG_ON(!bh);
    
    inode = (struct hellofs_inode *)(bh->b_data + HELLOFS_INODE_BYTE_OFFSET(sb, inode_no));
    inode_buf = kmem_cache_alloc(hellofs_inode_cache, GFP_KERNEL);
    memcpy(inode_buf, inode, sizeof(*inode_buf));

    brelse(bh);
    return inode_buf;
}

void hellofs_save_hellofs_inode(struct super_block *sb,
                                struct hellofs_inode *inode_buf) {
    struct buffer_head *bh;
    struct hellofs_inode *inode;
    uint64_t inode_no;

    inode_no = inode_buf->inode_no;
    bh = sb_bread(sb, HELLOFS_INODE_TABLE_START_BLOCK_NO + HELLOFS_INODE_BLOCK_OFFSET(sb, inode_no));
    BUG_ON(!bh);

    inode = (struct hellofs_inode *)(bh->b_data + HELLOFS_INODE_BYTE_OFFSET(sb, inode_no));
    memcpy(inode, inode_buf, sizeof(*inode));

    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
}

int hellofs_add_dir_record(struct super_block *sb, struct inode *dir,
                           struct dentry *dentry, struct inode *inode) {
    struct buffer_head *bh;
    struct hellofs_inode *parent_hellofs_inode;
    struct hellofs_dir_record *dir_record;

    parent_hellofs_inode = HELLOFS_INODE(dir);
    if (unlikely(parent_hellofs_inode->dir_children_count
            >= HELLOFS_DIR_MAX_RECORD(sb))) {
        return -ENOSPC;
    }

    bh = sb_bread(sb, parent_hellofs_inode->data_block_no);
    BUG_ON(!bh);

    dir_record = (struct hellofs_dir_record *)bh->b_data;
    dir_record += parent_hellofs_inode->dir_children_count;
    dir_record->inode_no = inode->i_ino;
    strcpy(dir_record->filename, dentry->d_name.name);

    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);

    parent_hellofs_inode->dir_children_count += 1;
    hellofs_save_hellofs_inode(sb, parent_hellofs_inode);

    return 0;
}

int hellofs_alloc_data_block(struct super_block *sb, uint64_t *out_data_block_no) {
    struct hellofs_superblock *hellofs_sb;
    struct buffer_head *bh;
    uint64_t i;
    int ret;
    char *bitmap;
    char *slot;
    char needle;

    hellofs_sb = HELLOFS_SB(sb);

    mutex_lock(&hellofs_sb_lock);

    bh = sb_bread(sb, HELLOFS_DATA_BLOCK_BITMAP_BLOCK_NO);
    BUG_ON(!bh);

    bitmap = bh->b_data;
    ret = -ENOSPC;
    for (i = 0; i < hellofs_sb->data_block_table_size; i++) {
        slot = bitmap + i / BITS_IN_BYTE;
        needle = 1 << (i % BITS_IN_BYTE);
        if (0 == (*slot & needle)) {
            *out_data_block_no
                = HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO(sb) + i;
            *slot |= needle;
            hellofs_sb->data_block_count += 1;
            ret = 0;
            break;
        }
    }

    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
    hellofs_save_sb(sb);

    mutex_unlock(&hellofs_sb_lock);
    return ret;
}

int hellofs_create_inode(struct inode *dir, struct dentry *dentry,
                         umode_t mode) {
    struct super_block *sb;
    struct hellofs_superblock *hellofs_sb;
    uint64_t inode_no;
    struct hellofs_inode *hellofs_inode;
    struct inode *inode;
    int ret;

    sb = dir->i_sb;
    hellofs_sb = HELLOFS_SB(sb);

    /* Create hellofs_inode */
    ret = hellofs_alloc_hellofs_inode(sb, &inode_no);
    if (0 != ret) {
        printk(KERN_ERR "Unable to allocate on-disk inode. "
                        "Is inode table full? "
                        "Inode count: %llu\n",
                        hellofs_sb->inode_count);
        return -ENOSPC;
    }
    hellofs_inode = kmem_cache_alloc(hellofs_inode_cache, GFP_KERNEL);
    hellofs_inode->inode_no = inode_no;
    hellofs_inode->mode = mode;
    if (S_ISDIR(mode)) {
        hellofs_inode->dir_children_count = 0;
    } else if (S_ISREG(mode)) {
        hellofs_inode->file_size = 0;
    } else {
        printk(KERN_WARNING
               "Inode %llu is neither a directory nor a regular file",
               inode_no);
    }

    /* Allocate data block for the new hellofs_inode */
    ret = hellofs_alloc_data_block(sb, &hellofs_inode->data_block_no);
    if (0 != ret) {
        printk(KERN_ERR "Unable to allocate on-disk data block. "
                        "Is data block table full? "
                        "Data block count: %llu\n",
                        hellofs_sb->data_block_count);
        return -ENOSPC;
    }

    /* Create VFS inode */
    inode = new_inode(sb);
    if (!inode) {
        return -ENOMEM;
    }
    hellofs_fill_inode(sb, inode, hellofs_inode);

    /* Add new inode to parent dir */
    ret = hellofs_add_dir_record(sb, dir, dentry, inode);
    if (0 != ret) {
        printk(KERN_ERR "Failed to add inode %lu to parent dir %lu\n",
               inode->i_ino, dir->i_ino);
        return -ENOSPC;
    }

    inode_init_owner(inode, dir, mode);
    d_add(dentry, inode);

    /* TODO we should free newly allocated inodes when error occurs */

    return 0;
}

int hellofs_create(struct inode *dir, struct dentry *dentry,
                   umode_t mode, bool excl) {
    return hellofs_create_inode(dir, dentry, mode);
}

int hellofs_mkdir(struct inode *dir, struct dentry *dentry,
                  umode_t mode) {
    /* @Sankar: The mkdir callback does not have S_IFDIR set.
       Even ext2 sets it explicitly. Perhaps this is a bug */
    mode |= S_IFDIR;
    return hellofs_create_inode(dir, dentry, mode);
}

struct dentry *hellofs_lookup(struct inode *dir,
                              struct dentry *child_dentry,
                              unsigned int flags) {
    struct hellofs_inode *parent_hellofs_inode = HELLOFS_INODE(dir);
    struct super_block *sb = dir->i_sb;
    struct buffer_head *bh;
    struct hellofs_dir_record *dir_record;
    struct hellofs_inode *hellofs_child_inode;
    struct inode *child_inode;
    uint64_t i;

    bh = sb_bread(sb, parent_hellofs_inode->data_block_no);
    BUG_ON(!bh);

    dir_record = (struct hellofs_dir_record *)bh->b_data;

    for (i = 0; i < parent_hellofs_inode->dir_children_count; i++) {
        printk(KERN_INFO "hellofs_lookup: i=%llu, dir_record->filename=%s, child_dentry->d_name.name=%s", i, dir_record->filename, child_dentry->d_name.name);    // TODO
        if (0 == strcmp(dir_record->filename, child_dentry->d_name.name)) {
            hellofs_child_inode = hellofs_get_hellofs_inode(sb, dir_record->inode_no);
            child_inode = new_inode(sb);
            if (!child_inode) {
                printk(KERN_ERR "Cannot create new inode. No memory.\n");
                return NULL; 
            }
            hellofs_fill_inode(sb, child_inode, hellofs_child_inode);
            inode_init_owner(child_inode, dir, hellofs_child_inode->mode);
            d_add(child_dentry, child_inode);
            return NULL;    
        }
        dir_record++;
    }

    printk(KERN_ERR
           "No inode found for the filename: %s\n",
           child_dentry->d_name.name);
    return NULL;
}