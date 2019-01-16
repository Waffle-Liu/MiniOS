#ifndef _ZJUNIX_FS_FAT_H
#define _ZJUNIX_FS_FAT_H

#include <zjunix/type.h>
#include <zjunix/fs/fscache.h>

/* 4k data buffer number in each file struct */
#define LOCAL_DATA_BUF_NUM 4

<<<<<<< HEAD
/* Sector Size */
#define SECTOR_SIZE 512
/* Cluster Size */
#define CLUSTER_SIZE 4096

/* The attribute of dir entry */
=======
#define SECTOR_SIZE 512
#define CLUSTER_SIZE 4096


>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
struct __attribute__((__packed__)) dir_entry_attr {
    u8 name[8];                   /* Name */
    u8 ext[3];                    /* Extension */
    u8 attr;                      /* attribute bits */
    u8 lcase;                     /* Case for base and extension */
    u8 ctime_cs;                  /* Creation time, centiseconds (0-199) */
    u16 ctime;                    /* Creation time */
    u16 cdate;                    /* Creation date */
    u16 adate;                    /* Last access date */
    u16 starthi;                  /* Start cluster (Hight 16 bits) */
    u16 time;                     /* Last modify time */
    u16 date;                     /* Last modify date */
    u16 startlow;                 /* Start cluster (Low 16 bits) */
    u32 size;                     /* file size (in bytes) */
};

<<<<<<< HEAD
/* the directory of FAT32 */
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
union dir_entry {
    u8 data[32];
    struct dir_entry_attr attr;
};

/* file struct */
typedef struct fat_file {
<<<<<<< HEAD
    u8 path[256];
    /* Current file pointer */
    u32 loc;
    /* Current directory entry position */
    u32 dir_entry_pos;
    /* Current directory entry sector */
    u32 dir_entry_sector;
    /* current directory entry */
    union dir_entry entry;
    /* Buffer clock head */
    u32 clock_head;
=======
    unsigned char path[256];
    /* Current file pointer */
    unsigned long loc;
    /* Current directory entry position */
    unsigned long dir_entry_pos;
    unsigned long dir_entry_sector;
    /* current directory entry */
    union dir_entry entry;
    /* Buffer clock head */
    unsigned long clock_head;
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    /* For normal FAT32, cluster size is 4k */
    BUF_4K data_buf[LOCAL_DATA_BUF_NUM];
} FILE;

<<<<<<< HEAD
/* file system directory attribute */
typedef struct fs_fat_dir {
    u32 cur_sector;
    u32 loc;
    u32 sec;
} FS_FAT_DIR;

/* The attribute of BPB */
=======
typedef struct fs_fat_dir {
    unsigned long cur_sector;
    unsigned long loc;
    unsigned long sec;
} FS_FAT_DIR;

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
struct __attribute__((__packed__)) BPB_attr {
    // 0x00 ~ 0x0f
    u8 jump_code[3];
    u8 oem_name[8];
    u16 sector_size;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    // 0x10 ~ 0x1f
    u8 number_of_copies_of_fat;
    u16 max_root_dir_entries;
    u16 num_of_small_sectors;
    u8 media_descriptor;
    u16 sectors_per_fat;
    u16 sectors_per_track;
    u16 num_of_heads;
    u32 num_of_hidden_sectors;
    // 0x20 ~ 0x2f
    u32 num_of_sectors;
    u32 num_of_sectors_per_fat;
    u16 flags;
    u16 version;
    u32 cluster_number_of_root_dir;
    // 0x30 ~ 0x3f
    u16 sector_number_of_fs_info;
    u16 sector_number_of_backup_boot;
    u8 reserved_data[12];
    // 0x40 ~ 0x51
    u8 logical_drive_number;
    u8 unused;
    u8 extended_signature;
    u32 serial_number;
    u8 volume_name[11];
    // 0x52 ~ 0x1fe
    u8 fat_name[8];
    u8 exec_code[420];
    u8 boot_record_signature[2];
};

union BPB_info {
    u8 data[512];
    struct BPB_attr attr;
};

<<<<<<< HEAD
/* The information of file system */
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
struct fs_info {
    u32 base_addr;
    u32 sectors_per_fat;
    u32 total_sectors;
    u32 total_data_clusters;
    u32 total_data_sectors;
    u32 first_data_sector;
    union BPB_info BPB;
    u8 fat_fs_info[SECTOR_SIZE];
};

<<<<<<< HEAD
/* find file */
unsigned long fs_find(FILE *file);

/* initial file system */
unsigned long init_fs();

/* the open instruction */
unsigned long fs_open(FILE *file, unsigned char *filename);

/* the close instruction */
unsigned long fs_close(FILE *file);

/* the read instruction */
unsigned long fs_read(FILE *file, unsigned char *buf, unsigned long count);

/* the write instruction */
=======
unsigned long fs_find(FILE *file);

unsigned long init_fs();

unsigned long fs_open(FILE *file, unsigned char *filename);

unsigned long fs_close(FILE *file);

unsigned long fs_read(FILE *file, unsigned char *buf, unsigned long count);

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
unsigned long fs_write(FILE *file, const unsigned char *buf, unsigned long count);

unsigned long fs_fflush();

<<<<<<< HEAD
/* the lseek instruction */
void fs_lseek(FILE *file, unsigned long new_loc);

/* the create instruction */
unsigned long fs_create(unsigned char *filename);

/* the make directory instruction */
unsigned long fs_mkdir(unsigned char *filename);

/* the remove directory instruction */
unsigned long fs_rmdir(unsigned char *filename);

/* the remove file instruction */
unsigned long fs_rm(unsigned char *filename);

/* the move file instruction */
unsigned long fs_mv(unsigned char *src, unsigned char *dest);

/* the rename file instruction */
u32 fs_rename(u8 *filepath,u8*newfilename);

/* the copy file instruction */
unsigned long fs_cp(unsigned char *src, unsigned char *dest);

/* the open directory instruction */
unsigned long fs_open_dir(FS_FAT_DIR *dir, unsigned char *filename);

/* the read directory instruction */
unsigned long fs_read_dir(FS_FAT_DIR *dir, unsigned char *buf);

/* the cat instruction */
unsigned long fs_cat(unsigned char * path);

/* get file name */
void get_filename(unsigned char *entry, unsigned char *buf);

/* get file date */
void get_filedate(unsigned char *entry, unsigned char *buf);

/* get file time */
void get_filetime(unsigned char *entry, unsigned char *buf);

/* get file size */
void get_filesize(unsigned char *entry, unsigned char *buf);

=======
void fs_lseek(FILE *file, unsigned long new_loc);

unsigned long fs_create(unsigned char *filename);

unsigned long fs_mkdir(unsigned char *filename);

unsigned long fs_rm(unsigned char *filename);

unsigned long fs_mv(unsigned char *src, unsigned char *dest);

unsigned long fs_open_dir(FS_FAT_DIR *dir, unsigned char *filename);

unsigned long fs_read_dir(FS_FAT_DIR *dir, unsigned char *buf);

unsigned long fs_cat(unsigned char * path);

void get_filename(unsigned char *entry, unsigned char *buf);

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
u32 read_block(u8 *buf, u32 addr, u32 count);

u32 write_block(u8 *buf, u32 addr, u32 count);

u32 get_entry_filesize(u8 *entry);

u32 get_entry_attr(u8 *entry);

<<<<<<< HEAD
#endif // !_ZJUNIX_FS_FAT_H
=======
#endif  // !_ZJUNIX_FS_FAT_H
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
