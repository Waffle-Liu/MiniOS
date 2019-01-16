
#ifndef _ZJUNIX_VFS_FAT16_H

#define _ZJUNIX_VFS_FAT16_H

#define FAT16_BLOCKSIZE 1024  // block size
#define FAT16_SIZE 1024000  // estimated disk size( in fact, the real is larger)
#define FAT16_END 65535  // EOF of FAT16
#define FAT16_FREE 0  // free maek of FAT16
#define FAT16_ROOTBLOCKNUM 2  // number of blocks occupied by the root
#define FAT16_MAXOPENFILE 10  // max number of file to be opened
#define FAT16_MAXFILENAME 8
#define FAT16_MAXTEXT 10000
#define FAT16_ROOT_INO 2

#include <zjunix/vfs/vfs.h>

// fat16 file info
struct fat16_basic_information {
	struct fat16_dos_boot_record* fa_DBR;// 分区引导扇区//dos boot record
	struct fat16_file_system_information* fa_FSINFO;// FSINFO auxiliary info
	struct fat16_file_allocation_table* fa_FAT;// 文件分配表//File Allocation Table
};

// fat16 dos boot record sector info
struct fat16_dos_boot_record {
	//0x0
	u8 x86asm_jmp[2];
	//0x2
	u8 x86asm_nop;
	//0x3
	u8 OEM[8];
	//0xb
	u16 byte_per_sector;
	//0xd
	u8 sector_per_cluster;
	//0xe
	u16 dbr_and_reserved;//DBR->FAT1
	//0x10
	u8 fat_num;// number of file allocate table//文件分配表的个数
	//0x11
	u16 max_dir_size;
	//0x13
	u16 sector_num;//扇区总数
	//0x15
	u8 medium_descriptor;
	//0x16
	u16 sector_per_fat;
	//0x18
	u16 sector_per_track;
	//0x1a
	u16 head_num;//磁头数
	//0x1c
	u32 hiden_sector_num;
	//0x20
	u32 sector_num_32M;//扇区总数,大于32M的分区扇区总数记录在这里
	//0x24
	u8 BIOS_drivernum;
	//0x25
	u8 __;//not used
	//0x26
	u8 extension_tag;
	//0x27
	u32 volume_num;
	//0x2B
	u8 volume_label[11];
	//0x36
	u8 fs_name[8];
	//0x3e

	u32 base;
	//u32 FDT_base;		s					   // sectors after fat whose size is 32
	u16 root_clu;//saying fdt( in cluster)									   //根目录起始所在簇号（算上0号和1号簇），
	//u32 sec_per_clu;									// 每一簇的扇区数
	u8 data[SECTOR_SIZE];							   // 数据
};

// FSINFO 文件系统信息
struct fat16_file_system_information {
	u32 base;										   // 基地址（绝对扇区地址）
	u8 data[SECTOR_SIZE*32];							   // 数据
};

// FAT 文件分配表汇总
struct fat16_file_allocation_table {
	u32 base;										   // 基地址（绝对扇区地址）
	u32 data_sec;									   // （FAT表无关）数据区起始位置的绝对扇区(方便)
	u32 root_sec;									   // （FAT表无关）根目录内容所在绝对扇区（方便）
};

//sizeof = 0x20
//文件控制块 file control block
struct __attribute__((__packed__)) fat16_dir_entry
{
	//0x0
	u8 filename[FAT16_MAXFILENAME];  // file name
	//0x8:
	u8 exname[3];  // ex-name
	//0xb
	u8 flags;
	//0xc
	u8 __[10];//reserved
	//0x16
	u16 time;  // create time
	//0x18
	u16 date;  // create date
	//0x1a
	u16 first;  // first cluster
	//0x1c
	u32 length;  // size

	u8 free;  // dir empty()? 0 for empty and  1 for occupied
	u8 attribute;  // attr, 0 for filefolder and 1 for data
};

struct fat16_open_fcb
{
	struct __attribute__((__packed__)) fat16_dir_entry dummy;
	
	unsigned short dirno;  // 相应打开文件的目录项在父目录文件中的盘块号
	int diroff;  // 相应打开文件的目录项在父目录文件的dirno盘块中的目录项序号
	char dir[80];  // 相应打开文件所在的目录名，这样方便快速检查出指定文件是否已经打开
	int father;  // 父目录在打开文件表项的位置
	int count;  // 读写指针在文件中的位置,文件的总字符数
	char fcbstate;  // 是否修改了文件的FCB的内容，如果修改了置为1，否则为0
	char topenfile;  // 表示该用户打开表项是否为空，若值为0，表示为空，否则表示已被某打开文件占据
}
openfilelist[FAT16_MAXOPENFILE];

// fat16.c
u32 init_fat16(u32);
u32 fat16_delete_inode(struct dentry *);
u32 fat16_write_inode(struct inode *, struct dentry *);
struct dentry* fat16_inode_lookup(struct inode *, struct dentry *, struct nameidata *);
u32 fat16_create(struct inode *, struct dentry *, u32 mode, struct nameidata *);
u32 fat16_readdir(struct file *, struct getdent *);
void fat16_convert_filename(struct qstr*, const struct qstr*, u8, u32);
u32 fat16_readpage(struct vfs_page *);
u32 fat16_writepage(struct vfs_page *);
u32 fat16_bmap(struct inode *, u32);
u32 read_fat16(struct inode *, u32);
#endif