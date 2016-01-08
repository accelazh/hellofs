#ifndef __HELLOFS_H__
#define __HELLOFS_H__

#define BITS_IN_BYTE 8
#define HELLOFS_MAGIC 0x20160105
#define HELLOFS_DEFAULT_BLOCKSIZE 4096
#define HELLOFS_DEFAULT_INODE_TABLE_SIZE 1024
#define HELLOFS_DEFAULT_DATA_BLOCK_TABLE_SIZE 1024
#define HELLOFS_FILENAME_MAXLEN 255

/* Define filesystem structures */

extern struct mutex hellofs_sb_lock;

struct hellofs_dir_record {
    char filename[HELLOFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

struct hellofs_inode {
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_no;

    // TODO struct timespec is defined kenrel space,
    // but mkfs-hellofs.c is compiled in user space
    /*struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;*/

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

struct hellofs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t inode_table_size;
    uint64_t inode_count;

    uint64_t data_block_table_size;
    uint64_t data_block_count;
};

static const uint64_t HELLOFS_SUPERBLOCK_BLOCK_NO = 0;
static const uint64_t HELLOFS_INODE_BITMAP_BLOCK_NO = 1;
static const uint64_t HELLOFS_DATA_BLOCK_BITMAP_BLOCK_NO = 2;
static const uint64_t HELLOFS_INODE_TABLE_START_BLOCK_NO = 3;

static const uint64_t HELLOFS_ROOTDIR_INODE_NO = 0;
// data block no is the absolute block number from start of device
// data block no offset is the relative block offset from start of data block table
static const uint64_t HELLOFS_ROOTDIR_DATA_BLOCK_NO_OFFSET = 0;

/* Helper functions */

static inline uint64_t HELLOFS_INODES_PER_BLOCK_HSB(
        struct hellofs_superblock *hellofs_sb) {
    return hellofs_sb->blocksize / sizeof(struct hellofs_inode);
}

static inline uint64_t HELLOFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(
        struct hellofs_superblock *hellofs_sb) {
    return HELLOFS_INODE_TABLE_START_BLOCK_NO
           + hellofs_sb->inode_table_size / HELLOFS_INODES_PER_BLOCK_HSB(hellofs_sb)
           + 1;
}

#endif /*__HELLOFS_H__*/