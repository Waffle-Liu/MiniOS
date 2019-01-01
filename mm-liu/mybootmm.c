#include <mybootmm.h>
#include <arch.h>
#include <zjunix/utils.h>
#include <driver/vga.h>

struct bootmm bmm;
unsigned int firstusercode_start;
unsigned int firstusercode_len;

// const value for ENUM of mem_type
char *mem_msg[] = {"Kernel code/data", "Mm Bitmap", "Vga Buffer", "Kernel page directory", "Kernel page table", "Dynamic", "Reserved"};

unsigned char bootmmmap[MACHINE_MMSIZE >> PAGE_SHIFT];

void init_bootmm()
{
	UINT index;
	UINT end;
	end = 16*1024*1024;
	kernel_memset(&bmm, 0, sizeof(bmm));
	bmm.phymm = get_phymm_size();
	bmm.max_pfn = bmm.phymm >> PAGE_SHIFT;
	bmm.s_map = bootmmmap;
	bmm.e_map = bootmmmap + sizeof(bootmmmap);
	bmm.cnt_infos = 0;
	kernel_memset(&bmm.s_map, PAGE_FREE, sizeof(bootmmmap));
	insert_mminfo(&bmm, 0, (UINT)(end-1), __MM_KENERL);
	bmm.last_alloc_end = ((UINT)(end) >> PAGE_SHIFT) - 1;

	for(index=0; index<end>>PAGE_SHIFT; index++)
	{
		bmm.s_map[index] = PAGE_USED;
	}

}
UINT get_phymm_size();

void set_mminfo(struct bootmm *mm, UINT start, UINT end, UINT type)
{
	struct bootmm_info *info = mm->info[mm->cnt_infos];
	info->start = start;
	info->end = end;
	info->type =type;
}

UINT insert_mminfo(struct bootmm *mm, UINT start, UINT end, UINT type)
{	
	int i;
	struct bootmm_info *infolist = mm->info;
	for(i=0; i<mm->cnt_infos; i++)
	{
		if(infolist[i].start > start) 
			break;
		if(infolist[i].type != type)
			continue;
		if(infolist[i].end == start-1)
		{
			if(i+1<mm->cnt_infos)
			{
				if(infolist[i+1].type==type && infolist[i+1].start == end+1)
				{
					infolist[i].end = infolist[i+1].end;
					remove_mminfo(mm, i+1);
				}
			}
			infolist[i].end = end;
		}
	}

	if(i<mm->cnt_infos)
	{
		if(infolist[i].type==type && infolist[i].start==end+1)
		{
			infolist[i].start = start;
		}
	}

	set_mminfo(mm, start, end, type);
	mm->cnt_infos++;
}

void remove_mminfo(struct bootmm* mm, UINT index)
{
	int i;
	if(index>mm->cnt_infos)
		return;
	if(index+1<mm->cnt_infos)
	{
		for(i=index; i<mm->cnt_infos; i++)
		{
			mm->info[i] = mm->info[i+1];
		}
		mm->cnt_infos--;
	}

}

/* Split one sequential memory area into two parts
 * (set the former one.end = split_start-1)
 * (set the latter one.start = split_start)
 */
UINT split_mminfo(struct bootmm *mm, UINT index, UINT split_start)
{
	UINT start, end, tmp;
	start = mm->info[index].start;
	end = mm->info[index].end;

	if(split_start<stat || split_start >end)
		return 0;
	if(mm->cnt_infos == MAX_INFO || index >= mm->cnt_infos)
		return 0;
	for(tmp = mm->cnt_infos-1; tmp>=index; tmp--)
	{
		mm->info[tmp+1] = mm->info[temp]; 
	}
	mm->info[index].end = split_start-1;
	mm->info[index+1].start = split_start;
	mm->cnt_infos++;
	return 1;
}

/*
 * set value of page-bitmap-indicator
 * @param s_pfn	: page frame start number
 * @param cnt	: the number of pages to be set
 * @param value	: the value to be set
 */
void set_map(UINT s_pfn, UINT cnt, UCHAR value)
{
	while(cnt)
	{
		bmm.s_map[s_pfn] = value;
		cnt--;
		s_pfn++;
	}
}

UCHAR *find_pages(UINT page_cnt, UINT s_pfn, UINT e_pfn, UINT align_pfn)
{
	int i, temp, cnt;

	if (align_pfn == 0) align_pfn = 1;
    s_pfn = UPPER_ALLIGN(s_pfn, align_pfn);

	for(i=s_pfn; i<e_pfn; )
	{
		if(bmm.s_map[i] == PAGE_USED)
		{
			i++;
			continue;
		}
		temp = i;
		cnt = page_cnt;
		while(cnt)
		{
			if(tmp>=e_pfn)
				return 0;
			if(bmm.s_map[i] == PAGE_USED)
			{
				i = temp+1;
				break;
			}
			temp++;
			cnt--;
		}
		if(cnt==0)
		{
			bmm.last_alloc_end = temp-1;
			set_map(i, page_cnt, PAGE_USED);
			return (UCHAR *)(i<<PAGE_SHIFT);
		}
		i = UPPER_ALLIGN(i, align_pfn);
	}
	if(i >= e_pfn) return 0;
}

UCHAR *bootmm_alloc_pages(UINT size, UINT type, UINT align)
{
	UINT size_inpages;
	UCHAR *res;

	size = UPPER_ALLIGN(size, 1<<PAGE_SHIFT);
	size_inpages = size>>PAGE_SHIFT;

	res = find_pages(size_inpages, bmm.last_alloc_end+1, bmm.max_pfn, align>>PAGE_SHIFT);
	if(res)
	{
		insert_mminfo(&bmm, (UINT)res, (UINT)res+size-1, type);
		return res;
	}

	res = find_pages(size_inpages, 0, bmm.last_alloc_end, align>>PAGE_SHIFT);
	if(res)
	{
		insert_mminfo(&bmm, (UINT)res, (UINT)res+size-1, type);
		return res;
	}

	return 0;
}