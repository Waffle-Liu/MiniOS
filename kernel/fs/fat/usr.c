#include <driver/vga.h>
#include <zjunix/log.h>
#include <zjunix/slab.h>
#include "fat.h"
#include "utils.h"
<<<<<<< HEAD
#include "../kernel/fs/fat/fat.h"
=======
#include <zjunix/vfs/vfs.h>

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab

u8 mk_dir_buf[32];
FILE file_create;

/* remove directory entry */
u32 fs_rm(u8 *filename) {
    u32 clus;
    u32 next_clus;
    FILE mk_dir;

<<<<<<< HEAD
    u8 tmp[128];
    fs_fullpath(filename,tmp,128);

    /* open file error */
    if (fs_open(&mk_dir, tmp) == 1)
        goto fs_rm_err;
    
=======
    if (fs_open(&mk_dir, filename) == 1)
        goto fs_rm_err;

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    /* Mark 0xE5 */
    mk_dir.entry.data[0] = 0xE5;

    /* Release all allocated block */
    clus = get_start_cluster(&mk_dir);
<<<<<<< HEAD
    
    while (clus != 0 && clus <= fat_info.total_data_clusters + 1) {
        /* get the value of cluster */
        if (get_fat_entry_value(clus, &next_clus) == 1)
            goto fs_rm_err;

        /* modify the value of cluster */
        if (fs_modify_fat(clus, 0) == 1)
            goto fs_rm_err;

        /* go to next cluster */
=======

    while (clus != 0 && clus <= fat_info.total_data_clusters + 1) {
        if (get_fat_entry_value(clus, &next_clus) == 1)
            goto fs_rm_err;

        if (fs_modify_fat(clus, 0) == 1)
            goto fs_rm_err;

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        clus = next_clus;
    }

    if (fs_close(&mk_dir) == 1)
        goto fs_rm_err;

    return 0;
fs_rm_err:
    return 1;
}

/* move directory entry */
u32 fs_mv(u8 *src, u8 *dest) {
    u32 i;
    FILE mk_dir;
<<<<<<< HEAD

    /* if src not exists */
    u8 tmp[128];
    fs_fullpath(src,tmp,128);
    if (fs_open(&mk_dir, tmp) == 1)
        goto fs_mv_err;

    /* create dest */
    fs_fullpath(dest,tmp,128);
    if (fs_create_with_attr(tmp, mk_dir.entry.data[11]) == 1)
=======
    u8 filename11[13];

    /* if src not exists */
    if (fs_open(&mk_dir, src) == 1)
        goto fs_mv_err;

    /* create dest */
    if (fs_create_with_attr(dest, mk_dir.entry.data[11]) == 1)
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        goto fs_mv_err;

    /* copy directory entry */
    for (i = 0; i < 32; i++)
        mk_dir_buf[i] = mk_dir.entry.data[i];
<<<<<<< HEAD
    /* create a new file on the dest */
    if (fs_open(&file_create, tmp) == 1)
=======

    /* new path */
    for (i = 0; i < 11; i++)
        mk_dir_buf[i] = filename11[i];

    if (fs_open(&file_create, dest) == 1)
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        goto fs_mv_err;

    /* copy directory entry to dest */
    for (i = 0; i < 32; i++)
        file_create.entry.data[i] = mk_dir_buf[i];

<<<<<<< HEAD
    /* close the file that we create */
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    if (fs_close(&file_create) == 1)
        goto fs_mv_err;

    /* mark src directory entry 0xE5 */
    mk_dir.entry.data[0] = 0xE5;

    if (fs_close(&mk_dir) == 1)
        goto fs_mv_err;

    return 0;
fs_mv_err:
    return 1;
}
<<<<<<< HEAD
/* move directory entry */
u32 fs_rename(u8 *filepath,u8 * newfilename) {
    u32 i,j;
    FILE mk_dir;

    /* if src not exists */
    u8 tmp[128];
    fs_fullpath(filepath,tmp,128);
    if (fs_open(&mk_dir, tmp) == 1)
        goto fs_rename_err;

    
    for (i = 0; i < 32; i++)
        mk_dir_buf[i] = mk_dir.entry.data[i];
    for(i=0;i<8&&newfilename[i]!='.'&&newfilename[i];i++)
        mk_dir_buf[i]=newfilename[i];
    j=i;
    while(j<11)
        mk_dir_buf[j++]=' ';
    if(newfilename[i]!='.')
        goto skip_ex;
    i++;
    for(j=0;j<3&&newfilename[j];j++)
        mk_dir_buf[j+8]=newfilename[j];
    
  skip_ex:  
    for (i = 0; i < 32; i++)
        mk_dir.entry.data[i] = mk_dir_buf[i];

    /* close the file that we create */
    if (fs_close(&mk_dir) == 1)
        goto fs_rename_err;

    return 0;
fs_rename_err:
    return 1;
}
void strcat(char *src,char*dest){
    int i,j;
    for(i=0;src[i];i++);
    for(j=0;dest[j];j++)
        src[i+j]=dest[j];
    return ;
}
/* copy directory entry */
u32 fs_cp(u8 *src, u8 *dest) {
    u32 i;
    FILE mk_dir;
    u8 filename11[13];

    /* if src not exists */
    u8 tmp[128];
    u8 tmp2[128];
    fs_fullpath(src,tmp,128);
    if (fs_open(&mk_dir, tmp) == 1)
        goto fs_cp_err;

    /* create dest */
    fs_fullpath(dest,tmp2,128);
    if (fs_create_with_attr(tmp2, mk_dir.entry.data[11]) == 1)
        goto fs_cp_err;

    /* copy directory entry */
    for (i = 0; i < 32; i++)
        mk_dir_buf[i] = mk_dir.entry.data[i];

    if (fs_open(&file_create, tmp2) == 1)
        goto fs_cp_err;

    /* copy directory entry to dest */
    for (i = 0; i < 32; i++)
        file_create.entry.data[i] = mk_dir_buf[i];

    if (fs_close(&file_create) == 1)
        goto fs_cp_err;

    struct dir_entry_attr entry;
    unsigned int r;
    char name[32];
    FS_FAT_DIR dir;
    fs_fullpath(src,tmp,128);
    if (fs_open_dir(&dir, tmp)) {
        //kernel_printf("open dir(%s) failed : No such directory! \n", tmp);
        //kernel_printf("%s\n", tmp);
        return 0;
    }
readdir:
    r = fs_read_dir(&dir, (unsigned char *)&entry);
    if (1 != r) {
        if (-1 == r) {
            //kernel_printf("\n");
        } else {
            get_filename((unsigned char *)&entry, name);
            if(name[0]=='.')
                goto readdir;
            if (entry.attr == 0x10){  // sub dir
                fs_fullpath(src,tmp,128);
                strcat(tmp,"/");
                strcat(tmp,name);
                fs_fullpath(dest,tmp2,128);
                strcat(tmp2,"/");
                strcat(tmp2,name);
                fs_cp(tmp,tmp2);
            }
            //kernel_printf("\n");
            goto readdir;
        }
    } else
        return 0;
    /* mark src directory entry 0xE5 */
    //mk_dir.entry.data[0] = 0xE5;

    if (fs_close(&mk_dir) == 1)
        goto fs_cp_err;

    return 0;
fs_cp_err:
    return 1;
}

/* rmdir */
u32 fs_rmdir(u8 *filename) {
    u32 clus;
    u32 next_clus;
    FILE rm_dir;

    /* if dir does not exist */
    u8 tmp[128];
    fs_fullpath(filename,tmp,128);
    if (fs_open(&rm_dir, tmp) == 1)
        goto fs_rmdir_err;

    /* Mark 0xE5 */
    rm_dir.entry.data[0] = 0xE5;

    /* Release all allocated block */
    clus = get_start_cluster(&rm_dir);

    while (clus != 0 && clus <= fat_info.total_data_clusters + 1) {
        /* get the value of cluster */
        if (get_fat_entry_value(clus, &next_clus) == 1)
            goto fs_rmdir_err;

        /* Release block */
        if (fs_modify_fat(clus, 0) == 1)
            goto fs_rmdir_err;

        /* go to next cluster */
        clus = next_clus;
    }

    if (fs_close(&rm_dir) == 1)
        goto fs_rmdir_err;

    return 0;
fs_rmdir_err:
    return 1;
}
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab

/* mkdir, create a new file and write . and .. */
u32 fs_mkdir(u8 *filename) {
    u32 i;
    FILE mk_dir;
    FILE file_creat;

<<<<<<< HEAD
    /* create a file */
    u8 tmp[128];
    fs_fullpath(filename,tmp,128);
    if (fs_create_with_attr(tmp, 0x30) == 1)
        goto fs_mkdir_err;

    /* open the file that we create */
    if (fs_open(&mk_dir, tmp) == 1)
        goto fs_mkdir_err;
    
    /* initial buffer */
=======
    if (fs_create_with_attr(filename, 0x30) == 1)
        goto fs_mkdir_err;

    if (fs_open(&mk_dir, filename) == 1)
        goto fs_mkdir_err;

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    mk_dir_buf[0] = '.';
    for (i = 1; i < 11; i++)
        mk_dir_buf[i] = 0x20;

<<<<<<< HEAD
    mk_dir_buf[11] = 0x30; /* the attribute of file */
    for (i = 12; i < 32; i++)
        mk_dir_buf[i] = 0;
    /* write the data to the create file */
    if (fs_write(&mk_dir, mk_dir_buf, 32) == 1)
        goto fs_mkdir_err;
    
=======
    mk_dir_buf[11] = 0x30;
    for (i = 12; i < 32; i++)
        mk_dir_buf[i] = 0;

    if (fs_write(&mk_dir, mk_dir_buf, 32) == 1)
        goto fs_mkdir_err;

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    fs_lseek(&mk_dir, 0);

    mk_dir_buf[20] = mk_dir.entry.data[20];
    mk_dir_buf[21] = mk_dir.entry.data[21];
    mk_dir_buf[26] = mk_dir.entry.data[26];
    mk_dir_buf[27] = mk_dir.entry.data[27];

    if (fs_write(&mk_dir, mk_dir_buf, 32) == 1)
        goto fs_mkdir_err;

    mk_dir_buf[0] = '.';
    mk_dir_buf[1] = '.';

    for (i = 2; i < 11; i++)
        mk_dir_buf[i] = 0x20;

    mk_dir_buf[11] = 0x30;
    for (i = 12; i < 32; i++)
        mk_dir_buf[i] = 0;

    set_u16(mk_dir_buf + 20, (file_creat.dir_entry_pos >> 16) & 0xFFFF);
    set_u16(mk_dir_buf + 26, file_creat.dir_entry_pos & 0xFFFF);

    if (fs_write(&mk_dir, mk_dir_buf, 32) == 1)
        goto fs_mkdir_err;

    for (i = 28; i < 32; i++)
        mk_dir.entry.data[i] = 0;

<<<<<<< HEAD
    /* close the file */
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    if (fs_close(&mk_dir) == 1)
        goto fs_mkdir_err;

    return 0;
fs_mkdir_err:
    return 1;
}

<<<<<<< HEAD
/* output the content in the file
 * 
 *Function output the content in the file
 *@param param is the path of file
 *@return int 1 means that cat unsuccessfully, 0 means that cat successfully
 */
u32 fs_cat(u8 *path) 
{
    FILE cat_file;

    /* Open */
    u8 tmp[128];
    fs_fullpath(path,tmp,128);
    if (0 != fs_open(&cat_file, tmp)) {
        log(LOG_FAIL, "File %s open failed", path);
        return 1;
    }
    /* Read */
    u32 file_size = get_entry_filesize(cat_file.entry.data);
    /* apply for the memory space */
    u8 *buf = (u8 *)kmalloc( (file_size + 1));
    kernel_printf("  %d\n", file_size);
    /* read the content of file by using buffer */
    fs_read(&cat_file, buf, file_size);
    buf[file_size] = 0;
    /* output the content of file */
    kernel_printf("  %s\n", buf);
    /* close the file */
    fs_close(&cat_file);
    kfree(buf);
    return 0;
}

=======
u32 fs_cat(u8 *path) {
    u8 filename[12];
    FILE cat_file;

    /* Open */
    if (0 != fs_open(&cat_file, path)) {
        log(LOG_FAIL, "File %s open failed", path);
        return 1;
    }

    /* Read */
    u32 file_size = get_entry_filesize(cat_file.entry.data);
    u8 *buf = (u8 *)kmalloc(file_size + 1);
    fs_read(&cat_file, buf, file_size);
    buf[file_size] = 0;
    kernel_printf("%s\n", buf);
    fs_close(&cat_file);
    kfree(buf);
    return 0;

}
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
