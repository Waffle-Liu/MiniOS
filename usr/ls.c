#include <driver/vga.h>
#include <zjunix/fs/fat.h>
<<<<<<< HEAD
#include "../kernel/fs/fat/fat.h"
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab

char *cut_front_blank(char *str) {
    char *s = str;
    unsigned int index = 0;

    while (*s == ' ') {
        ++s;
        ++index;
    }

    if (!index)
        return str;

    while (*s) {
        *(s - index) = *s;
        ++s;
    }

    --s;
    *s = 0;

    return str;
}

unsigned int strlen(unsigned char *str) {
    unsigned int len = 0;
    while (str[len])
        ++len;
    return len;
}

unsigned int each_param(char *para, char *word, unsigned int off, char ch) {
    int index = 0;

    while (para[off] && para[off] != ch) {
        word[index] = para[off];
        ++index;
        ++off;
    }

    word[index] = 0;

    return off;
}

int ls(char *para) {
    char pwd[128];
<<<<<<< HEAD
    char fullpath[256];
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    struct dir_entry_attr entry;
    char name[32];
    char *p = para;
    unsigned int next;
    unsigned int r;
    unsigned int p_len;
    FS_FAT_DIR dir;

    p = cut_front_blank(p);
    p_len = strlen(p);
    next = each_param(p, pwd, 0, ' ');
<<<<<<< HEAD
    fs_fullpath(pwd,fullpath,256);

    if (fs_open_dir(&dir, fullpath)) {
        kernel_printf("open dir(%s) failed : No such directory! \n", pwd);
        kernel_printf("%s\n", fullpath);
=======

    if (fs_open_dir(&dir, pwd)) {
        kernel_printf("open dir(%s) failed : No such directory!\n", pwd);
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        return 1;
    }

readdir:
    r = fs_read_dir(&dir, (unsigned char *)&entry);
    if (1 != r) {
        if (-1 == r) {
            kernel_printf("\n");
        } else {
            get_filename((unsigned char *)&entry, name);
            if (entry.attr == 0x10)  // sub dir
                kernel_printf("%s/", name);
            else
                kernel_printf("%s", name);
            kernel_printf("\n");
            goto readdir;
        }
    } else
        return 1;

    return 0;
}
<<<<<<< HEAD
int cd(char *para){
    char pwd[128];
    char fullpath[256];
    char *p = para;
    unsigned int next;
    unsigned int p_len;
    int i;
    FS_FAT_DIR dir;

    p = cut_front_blank(p);
    p_len = strlen(p);
    next = each_param(p, pwd, 0, ' ');
    fs_fullpath(pwd,fullpath,256);

    if (fs_open_dir(&dir, fullpath)) {
        kernel_printf("access dir(%s) failed : No such directory!\n", pwd);
        return 1;
    }
    for(i=0;i<192&&fullpath[i]!='\0';i++)
        relative_path[i]=fullpath[i];
    while(i<192)relative_path[i++]='\0';
    return 0;
}
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
