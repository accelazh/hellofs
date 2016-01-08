#include "khellofs.h"

int hellofs_readdir(struct file *filp, void *dirent, filldir_t filldir) {
    loff_t pos;
    struct inode *inode;
    struct super_block *sb;
    struct buffer_head *bh;
    struct hellofs_inode *hellofs_inode;
    struct hellofs_dir_record *dir_record;
    uint64_t i;

    pos = filp->f_pos;
    inode = filp->f_dentry->d_inode;
    sb = inode->i_sb;
    hellofs_inode = HELLOFS_INODE(inode);

    if (pos) {
        // TODO @Sankar: we use a hack of reading pos to figure if we have filled in data.
        return 0;
    }

    printk(KERN_INFO "readdir: hellofs_inode->inode_no=%llu", hellofs_inode->inode_no);

    if (unlikely(!S_ISDIR(hellofs_inode->mode))) {
        printk(KERN_ERR
               "Inode %llu of dentry %s is not a directory\n",
               hellofs_inode->inode_no,
               filp->f_dentry->d_name.name);
        return -ENOTDIR;
    }

    bh = sb_bread(sb, hellofs_inode->data_block_no);
    BUG_ON(!bh);

    dir_record = (struct hellofs_dir_record *)bh->b_data;
    for (i = 0; i < hellofs_inode->dir_children_count; i++) {
        filldir(dirent, dir_record->filename, HELLOFS_FILENAME_MAXLEN, pos,
                dir_record->inode_no, DT_UNKNOWN);
        filp->f_pos += sizeof(struct hellofs_dir_record);
        pos += sizeof(struct hellofs_dir_record);
        dir_record++;
    }
    brelse(bh);

    return 0;
}
