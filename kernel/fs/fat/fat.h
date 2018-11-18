#ifndef _FAT_H
#define _FAT_H

#include <zjunix/fs/fat.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)

#define FAT_BUF_NUM 2
extern BUF_512 fat_buf[FAT_BUF_NUM];

extern struct fs_info fat_info;

u32 fs_create_with_attr(u8 *filename, u8 attr);

u32 read_fat_sector(u32 ThisFATSecNum);


extern char relative_path[192];
int fs_fullpath(char *, char *,int);

#endif  // ! _FS_FAT_H
