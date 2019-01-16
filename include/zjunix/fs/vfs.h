#ifndef _ZJUNIX_FS_VFS_H
#define _ZJUNIX_FS_VFS_H

#include <zjunix/type.h>
#include <zjunix/fs/fat.h>

struct ext2 {
    u32 (*init)();
    u32 (*find)();
    u32 (*open)();
    u32 (*close)();
    u32 (*read)();
    u32 (*write)();
    u32 (*fflush)();
    void (*lseek)();
};

struct fat16 {
    u32 (*init)();
    u32 (*find)();
    u32 (*open)();
    u32 (*close)();
    u32 (*read)();
    u32 (*write)();
    u32 (*fflush)();
    void (*lseek)();
};

struct fat32 {
    struct FILE * file;
    u32 (*find)(FILE *file); /* find the file function */
    u32 (*init)(); /* initial the file system */
    u32 (*open)(FILE *file, unsigned char *filename); /* open file function */
    u32 (*close)(FILE *file); /* close file function */
    u32 (*read)(FILE *file, unsigned char *buf, unsigned long count); /* read file function */
    u32 (*write)(FILE *file, const unsigned char *buf, unsigned long count); /* write file function */
    u32 (*fflush)(); /* fflush file function */
    void (*lseek)(FILE *file, unsigned long new_loc); /* lseek file function */
    u32 (*create)(unsigned char *filename); /* create file function */
    u32 (*mkdir)(unsigned char *filename); /* make directory function */
    u32 (*rmdir)(unsigned char *filename); /* remove directory function */
    u32 (*rm)(unsigned char *filename);  /* remove file function */
    u32 (*mv)(unsigned char *src, unsigned char *dest); /* move file function */
    u32 (*cp)(unsigned char *src, unsigned char *dest); /* copy file function */
    u32 (*open_dir)(FS_FAT_DIR *dir, unsigned char *filename); /* open directory function */
    u32 (*read_dir)(FS_FAT_DIR *dir, unsigned char *buf); /* read directory function */
    u32 (*cat)(unsigned char * path); /* cat file function */
    u32 (*rename)(unsigned char * path,unsigned char *newfilename); /* just rename the file*/
};

struct vfs {
    u8 attr;                      /* attribute bits */
    u8 ctime_cs;                  /* Creation time, centiseconds (0-199) */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u16 adate;                    /* Last access date */
    u16 time;                     /* Last modify time */
    u16 date;                     /* Last modify date */
    u32 size;                     /* file size (in bytes) */
    u32 count;                    /* reference count */
    u32 uid;                      /* user id */
    u32 gid;                      /* group id */
    struct fat32 * fat32_file;    /* fat32 file operation */
    struct ext2 * ext2_file;      /* ext2 file operation */
    struct fat16 * fat16_file;    /* fat16 file operation */
};

struct vfs* vfsfile;

void initial_vfs(); /* initial virtual file system */

#endif // !_ZJUNIX_FS_VFS_H
