//
// Created by pjs on 2021/4/20.
//

#ifndef QUARKOS_FS_FAT_H
#define QUARKOS_FS_FAT_H

#include "types.h"
#include "fs/page_cache.h"
#include "fs/vfs.h"

#define ROOT_INUM    2 // 根目录的 inode 索引, inode 索引从 1 开始

#define BLOCK_SIZE 4096
#define INODE_SIZE 256
#define INODE_PER_BLOCK (BLOCK_SIZE/INODE_SIZE)
#define SUPER_BLOCK_NO 2
#define BLOCK_PER_GROUP (BLOCK_SIZE*8)
#define DESCRIPTOR_PER_BLOCK (BLOCK_SIZE/sizeof(groupDesc_t))
#define N_DIRECT_BLOCK 12                         // 直接指针数量
#define N_PTR          (BLOCK_SIZE/sizeof(u32_t)) // 块内指针数量
#define N_BLOCKS       1074791436                 // 直接遍历大的离谱, 不过一般目录用不到间接指针


typedef struct superBlock {
#define EXT2_SIGNATURE 0xef53
    u32_t inodeCnt;        // inode 数
    u32_t blockCnt;
    u32_t rBlockCnt;       // 为超级用户保留的块
    u32_t freeBlockCnt;    // 没有被分配的块数
    u32_t freeInodeCnt;
    u32_t firstDataBlock;  // 包含超级块的块数量

    u32_t logBlockSize;    // 值为: log2(block size) - 10
    u32_t logFragmentSize; // log2(fragment size) - 10

    u32_t blockPerGroup;
    u32_t fragmentsPerGroup;
    u32_t inodePerGroup;

    u32_t mountTime;
    u32_t writtenTime;

    u16_t mountCnt;
    u16_t maxMountCnt;

    u16_t magic;

    u16_t state;        // 文件系统状态
#define FS_STATE_ERROR 2
#define FS_STATE_CLEAN 1

    u16_t error;        // 处理错误的方法
#define FS_ERR_IGNORE            1
#define FS_ERR_REMOUNT_AS_READ_ONLY 2
#define FS_ERR_KERNEL_PANIC      3

    u16_t fsVersionMinor;

    u32_t fsckTime;         // 上次 fsck 检查(POSIX)时间
    u32_t fsckInterval;     // 强制 fsck 检查时间间隔

    u32_t creatorOS;
#define OS_LINUX    0
#define OS_GUN_HURD 1
#define OS_MASIX    2
#define OS_FREE_BSD 3
#define OS_OTHER    4


    u32_t fsVersionMajor;

    u16_t uid; //可以使用保留块的用户id
    u16_t gid; //可以使用保留块的用户组 id

//fs_version_major >= 1 时, 以下字段有效
    u32_t firstInode;  // 版本<1则 = 11, 第一个非保留 inode
    u16_t inodeSize;   // 版本<1则 = 128
    u16_t blockGroup;  // 当前超级块所属的块组

    u32_t optionalFeature;    // 可选功能
#define PREALLOCATE_FOR_DIR 1    // 创建目录时预分配一些数据块
#define AFS_SERVER_INODES   2
#define JOURNAL             4    // 支持日志(ext3)
#define INODE_EXT           8    // inode 可以有扩展属性
#define FS_RESIZE           0x10 // fs 可以调整大小
#define DIR_HASH_INDEX      0x20 // 目录使用哈希索引
    u32_t requiredFeature;
#define COMPRESS            1    // 压缩
#define DIR_TYPE_FIELD      2    // 目录有类型字段
#define REPLAY_JOURNAL      4
#define JOURNAL_DEV         8
    u32_t optionalFeatureRo; //如果不支持这些功能则只读挂载
#define SS_GROUP_DESC       1   // Sparse superblocks and group descriptor tables
#define FS_64BIT_SIZE       2
#define DIR_ARE_BIN_TREE    4   // 目录以二叉树的形式存储

    u8_t fsId[16];

    u8_t volumeName[16];      // 以 0 结束的字符串
    u8_t pathLastMount[64];

    u32_t algorithmsBitmap;   // 位图使用的压缩算法
    u8_t preallocateBlock;    // 为文件预分配的块数
    u8_t preallocateBlockDir; // 为目录预分配的块数

    u16_t unused;

    u8_t journalId[16];
    u32_t journalInode;
    u32_t journalDevice;

    u32_t inodeListHead;          // start of list of inodes to delete

    u32_t hashSeed[4];            // HTREE hash seed
    u8_t defHashVersion;          // Default hash version to use
    u8_t reservedCharPad;
    u16_t reservedWordPad;
    u32_t defaultMountOpts;
    u32_t firstMetaBg;            // First metablock block group

    u8_t unused2[760];
} superBlock_t;

//Block Group Descriptor Table, 紧接超级块后
typedef struct blockGroupDescriptor {
    u32_t blockBitmapAddr;
    u32_t inodeBitmapAddr;
    u32_t inodeTableAddr;
    u16_t freeBlockCnt;     // 组中没有分配的块数
    u16_t freeInodesCnt;
    u16_t dirNum;           // 组中目录数
    u8_t unused[14];
} groupDesc_t;

typedef struct inode {
    u16_t mode;             // 权限与类型
// 类型
#define MOD_FIFO            0x1000
#define MOD_CHARACTER_DEV   0x2000
#define MOD_DIR             0x4000
#define MOD_BLOCK           0x6000
#define MOD_REGULAR_FILE    0x8000
#define MOD_SYMBOLIC_LINK   0xA000
#define MOD_UNIX_SOCKET     0xC000
// 权限
#define MOD_X               0x001
#define MOD_W               0x002
#define MOD_R               0x004

#define MOD_OTHER           0
#define MOD_GROUP           3
#define MOD_USER            6
#define GET_MODE(who, permit)  ((permit) << (who))

#define USER_MODE           GET_MODE(MOD_USER,MOD_X|MOD_W|MOD_R)

    u16_t uid;
/*
 * In revision 1 and later revisions, and only for regular files,
 * this represents the lower 32-bit of the file size;
 * the upper 32-bit is located in the i_dir_acl.
 */
    u32_t size;

    u32_t accessTime;
    u32_t createTime;
    u32_t modTime;   // 修改时间
    u32_t delTime;

    u16_t groupId;

    u16_t linkCnt;    // 链接数量

    u32_t cntSectors; // 磁盘扇区数(不包括inode与链接)

    u32_t flags;
#define SYNC_UPDATE         0x8   //新数据立即写入磁盘
#define IMM_FILE            0x10  // 文件内容不可变
#define APPEND_ONLY         0x20
#define HASH_INDEX_DIR      0x10000
#define AFS_DIR             0x20000
#define JOURNAL_FILE_DATA   0x40000
    u32_t osSpec;

    u32_t blocks[N_DIRECT_BLOCK + 3]; // 0-12 为直接指针

    u32_t generationNum;
    u32_t filAcl;         // 版本>=1 有效
    u32_t dirAcl;         // 版本>=1 有效
    u32_t fragmentAddr;
    u32_t osSpec2[3];

    u8_t padding[INODE_SIZE - 128];
} inode_t;


typedef struct directory_entry {
    u32_t inode;
    u16_t entrySize; // 目录项总大小
    u8_t nameLen;
    u8_t type;
    //目录项指向的文件类型
#define DIR_UNKNOWN            0
#define DIR_REGULAR_FILE       1
#define DIR_DIR                2
#define DIR_CHAR               3
#define DIR_BLOCK              4
#define DIR_FIFO               5
#define DIR_SOCKET             6
#define DIR_SOFT_LINK          7
    u8_t name[0];
} PACKED dir_t;

void ext2_close();

void ext2_mount();

void ext2_write(fd_t *file, uint32_t _offset, uint32_t size, char *_buf);

void ext2_read(fd_t *file, uint32_t _offset, uint32_t size, char *_buf);

void ext2_open(fd_t *file);

void ext2_ls(fd_t *parent);

void ext2_link(fd_t *parent);

void ext2_rmDir(fd_t *parent, char *name);

fd_t *ext2_findDir(fd_t *parent, char *name);

void ext2_unlink(fd_t *parent, char *name);

#endif //QUARKOS_FS_FAT_H
