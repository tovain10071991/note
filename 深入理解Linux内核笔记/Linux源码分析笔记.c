//include/asm-i386/pgtable.h
#define	PGDIR_SHIT	22	//���Ե�ַ��PGDƫ��
#define	PGDIR_SIZE	(1UL<<PGDIR_SHIFT)	//ÿ��ҳĿ¼��������Ŀռ�
#define	PTRS_PER_PGD	1024	//����ָ�������ҳĿ¼�����

//include/asm-i386/page.h
#define	__PAGE_OFFSET	(0xc0000000)
#define	PAGE_OFFSET		((unsigned long)__PAGE_OFFSET)
#define	__pa(x)	...
#define	__va(x)	...
#define	TASK_SIZE	(PAGE_SIZE)	//�����û��ռ�����3GB

//include/asm_i386/mmu_context.h
//����CR3
asm	volatile("movl %0, %%cr3"::"r")