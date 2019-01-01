#ifndef _ZJUNIX_BOOTMM_H
#define _ZJUNIX_BOOTMM_H

#define UINT unsigned int
#define UCHAR unsigned char


// 4K per page
#define PAGE_SHIFT 12
#define PAGE_FREE 0x00
#define PAGE_USED 0xff

#define PAGE_ALIGN (~((1 << PAGE_SHIFT) - 1))

#define MAX_INFO 10

extern UCHAR _end[];

enum mm_usage { _MM_KERNEL, _MM_MMMAP, _MM_VGABUFF, _MM_PDTABLE, _MM_PTABLE, _MM_DYNAMIC, _MM_RESERVED, _MM_COUNT };

struct bootmm_info
{
	UINT start;
	UINT end;
	unsigned type;	
};

struct bootmm
{
	UINT phymm;		// the actual physical memory
	UINT max_pfn;	// record the max page number
	UCHAR *s_map;	// map begin place
	UCHAR *e_map;
	UINT last_alloc_end;
	UINT cnt_infos;	// get number of infos stored in bootmm now
	struct bootmm_info info[MAX_INFO];
};


// function
extern void init_bootmm();
extern UINT get_phymm_size();

extern void set_mminfo(struct bootmm_info *info, UINT start, UINT end, UINT type);
extern UINT insert_mminfo(struct bootmm *mm, UINT start, UINT end, UINT type);
extern void remove_mminfo(struct bootmm* mm, UINT index);
extern UINT split_mminfo(struct bootmm *mm, UINT index, UINT split_start);

extern void set_map(UINT s_pfn, UINT cnt, UCHAR value);
extern UCHAR* find_pages(UINT page_cnt, UINT s_pfn, UINT e_pfn, UINT align_pfn);

extern UCHAR *bootmm_alloc_pages(UINT size, UINT type, UINT align);

#endif