#ifndef _TYPES_H_
#define _TYPES_H_
  
/******************************************************************************
* SECTION: Type def
*******************************************************************************/
typedef int          boolean;
typedef uint16_t     flag16;


/******************************************************************************
* SECTION: Macro
*******************************************************************************/
#define TRUE                    1
#define FALSE                   0
#define UINT32_BITS             32
#define UINT8_BITS              8

#define NEWFS_MAGIC_NUM           0x52415453  
#define NEWFS_SUPER_OFS           0
#define NEWFS_ROOT_INO            0



#define NEWFS_ERROR_NONE          0
#define NEWFS_ERROR_ACCESS        EACCES
#define NEWFS_ERROR_SEEK          ESPIPE     
#define NEWFS_ERROR_ISDIR         EISDIR
#define NEWFS_ERROR_NOSPACE       ENOSPC
#define NEWFS_ERROR_EXISTS        EEXIST
#define NEWFS_ERROR_NOTFOUND      ENOENT
#define NEWFS_ERROR_UNSUPPORTED   ENXIO
#define NEWFS_ERROR_IO            EIO     /* Error Input/Output */
#define NEWFS_ERROR_INVAL         EINVAL  /* Invalid Args */

#define NEWFS_MAX_FILE_NAME       128//最长文件名
#define NEWFS_INODE_PER_FILE      1//每个文件的索引节点数
#define NEWFS_DATA_PER_FILE       6//每个文件的数据块数
#define NEWFS_DEFAULT_PERM        0777

#define NEWFS_IOC_MAGIC           'S'
#define NEWFS_IOC_SEEK            _IO(NEWFS_IOC_MAGIC, 0)

#define NEWFS_FLAG_BUF_DIRTY      0x1
#define NEWFS_FLAG_BUF_OCCUPY     0x2
/******************************************************************************
* SECTION: Macro Function
*******************************************************************************/
#define NEWFS_IO_SZ()                     (newfs_super.sz_io)
#define NEWFS_DISK_SZ()                   (newfs_super.sz_disk)
#define NEWFS_DRIVER()                    (newfs_super.driver_fd)

#define NEWFS_ROUND_DOWN(value, round)    (value % round == 0 ? value : (value / round) * round)
#define NEWFS_ROUND_UP(value, round)      (value % round == 0 ? value : (value / round + 1) * round)

#define NEWFS_BLKS_SZ(blks)               (blks * NEWFS_IO_SZ())
#define NEWFS_ASSIGN_FNAME(psfs_dentry,_fname)\ 
                                        memcpy(psfs_dentry->fname, _fname, strlen(_fname))
#define NEWFS_INO_OFS(ino)                (newfs_super.inode_offset + ino * NEWFS_BLKS_SZ(NEWFS_INODE_PER_FILE ))//inode offset
#define NEWFS_DATA_OFS(ino)               (newfs_super.data_offset + ino * NEWFS_BLKS_SZ(NEWFS_DATA_PER_FILE))//data offset

#define NEWFS_IS_DIR(pinode)              (pinode->dentry->ftype == NEWFS_DIR)
#define NEWFS_IS_REG(pinode)              (pinode->dentry->ftype == NEWFS_REG_FILE)
/******************************************************************************
* SECTION: FS Specific Structure - In memory structure
*******************************************************************************/
struct newfs_dentry;
struct newfs_inode;
struct newfs_super;

struct custom_options {
	const char*        device;
};

typedef enum newfs_file_type {
    NEWFS_REG_FILE,
    NEWFS_DIR
} NEWFS_FILE_TYPE;

struct newfs_inode {     //文件系统的索引项
    uint32_t             ino;                           // inode位图里的下标
    int                  size;                          /* 文件已占用空间 */
    int                  dir_cnt;
    struct newfs_dentry* dentry;                        /* 指向该inode的dentry */
    struct newfs_dentry* dentrys;                       /* 所有目录项 */
    uint8_t*             data; 
};

struct newfs_dentry {    //文件系统的目录项
    char                 fname[NEWFS_MAX_FILE_NAME];
    uint32_t             ino;                           // 其所指向的ino号
    struct newfs_inode*  inode;                         /* 指向inode */
    NEWFS_FILE_TYPE      ftype;
    struct newfs_dentry* brother;                       /* 兄弟 */
    struct newfs_dentry* parent;
};

struct newfs_super {   //文件系统的超级快
    uint32_t             magic;
    int                  driver_fd;
    int                  sz_io;                   //size
    int                  sz_disk;
    int                  sz_usage;

    int                  max_ino;
    uint8_t*             map_inode;               //inode位图
    int                  map_inode_blks;          //inode位图占用的块数
    int                  map_inode_offset;        //inode位图在磁盘上的偏移
    uint8_t*             map_data;                //data位图
    int                  map_data_blks;           //data位图占用的块数
    int                  map_data_offset;         //data位图在磁盘上的偏移

    int                  inode_offset;
    int                  data_offset;

    boolean              is_mounted;

    struct newfs_dentry* root_dentry;             //根目录root
};
static inline struct newfs_dentry* new_dentry(char * fname, NEWFS_FILE_TYPE ftype) {
    struct newfs_dentry * dentry = (struct newfs_dentry *)malloc(sizeof(struct newfs_dentry));
    memset(dentry, 0, sizeof(struct newfs_dentry));
    NEWFS_ASSIGN_FNAME(dentry, fname);
    dentry->ftype   = ftype;
    dentry->ino     = -1;
    dentry->inode   = NULL;
    dentry->parent  = NULL;
    dentry->brother = NULL;                                           
}
/******************************************************************************
* SECTION: FS Specific Structure - Disk structure
*******************************************************************************/
struct newfs_super_d {     //磁盘结构的超级快
    uint32_t           magic_num;                     // 幻数
    int                max_ino;                       // 最多支持的文件数
    int                map_inode_blks;                // inode位图占用的块数
    int                map_inode_offset;              // inode位图在磁盘上的偏移
    int                map_data_blks;                 // data位图占用的块数
    int                map_data_offset;               // data位图在磁盘上的偏移
    int                sz_usage;

    int                inode_offset;
    int                data_offset;
};


struct newfs_inode_d {     //磁盘结构的索引块
    int                 ino;                     // 在inode位图中的下标
    int                 size;                    // 文件已占用空间
    int                 link;                    // 链接数
    NEWFS_FILE_TYPE     ftype;                   // 文件类型（目录类型、普通文件类型）
    int                 dir_cnt;                 // 如果是目录类型文件，下面有几个目录项
    int                 block_pointer[6];        // 数据块指针（可固定分配）
};

struct newfs_dentry_d {    //磁盘结构的目录项
    char              fname[NEWFS_MAX_FILE_NAME];   // 指向的ino文件名
    NEWFS_FILE_TYPE   ftype;                        // 指向的ino文件类型
    int               ino;                          // 指向的ino号
    int               valid;                        // 该目录项是否有效
};


#endif /* _TYPES_H_ */