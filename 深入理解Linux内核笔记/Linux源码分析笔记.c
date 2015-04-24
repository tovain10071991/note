//include/asm-i386/pgtable.h
#define	PGDIR_SHIT	22	//线性地址中PGD偏移
#define	PGDIR_SIZE	(1UL<<PGDIR_SHIFT)	//每个页目录项所代表的空间
#define	PTRS_PER_PGD	1024	//表中指针个数，页目录项个数

//include/asm-i386/page.h
#define	__PAGE_OFFSET	(0xc0000000)
#define	PAGE_OFFSET		((unsigned long)__PAGE_OFFSET)
#define	__pa(x)	...
#define	__va(x)	...
#define	TASK_SIZE	(PAGE_SIZE)	//进程用户空间上限3GB

//include/asm_i386/mmu_context.h
//更新CR3
asm	volatile("movl %0, %%cr3"::"r")