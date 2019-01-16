#ifndef _FAT_H
#define _FAT_H

#include <zjunix/fs/fat.h>
<<<<<<< HEAD

#define PAGE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SHIFT)
=======
#include <zjunix/vfs/vfs.h>

// #ifndef VFS_DEBUG
// #define PAGE_SHIFT 12
// #define PAGE_SIZE (1 << PAGE_SHIFT)
// #endif
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab

#define FAT_BUF_NUM 2
extern BUF_512 fat_buf[FAT_BUF_NUM];

extern struct fs_info fat_info;

u32 fs_create_with_attr(u8 *filename, u8 attr);

u32 read_fat_sector(u32 ThisFATSecNum);

<<<<<<< HEAD
extern char relative_path[192];
int fs_fullpath(char *, char *,int);


#endif  // ! _FS_FAT_H
=======
#endif  // ! _FS_FAT_H
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
