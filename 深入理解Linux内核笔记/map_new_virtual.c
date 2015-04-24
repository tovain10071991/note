/*
map_new_virtual(page)：把页框的物理地址插入到pkmap_page_table中并在哈希表中加入一个元素。
该函数扫描pkmap_count中的所有计数器直到找到一个空值
这里的计数器的线性空间页的被引用数
*/

for (;;) {
	int count;
	DECLARE_WAITQUEUE(wait, current);
	for (count = LAST_PKMAP; count > 0; --count) {
		last_pkmap_nr = (last_pkmap_nr + 1) & (LAST_PKMAP - 1);
		//从后往前扫描全部LAST_PKMAP页
		if (!last_pkmap_nr) {
			//如果扫描完一趟，刷新引用(所有页引用都变为0)，下次从新开始扫描
			flush_all_zero_pkmaps();
			count = LAST_PKMAP;
		}
		if (!pkmap_count[last_pkmap_nr]) {
			//如果该页未被引用，返回该页的线性地址
			unsigned long vaddr = PKMAP_BASE +(last_pkmap_nr << PAGE_SHIFT);
			set_pte(&(pkmap_page_table[last_pkmap_nr]),mk_pte(page, _ _pgprot(0x63)));
			pkmap_count[last_pkmap_nr] = 1;
			set_page_address(page, (void *) vaddr);
			return vaddr;
		}
	}
	//扫描了一趟还没找到空闲页，挂起，等待释放页函数将其唤醒
	current->state = TASK_UNINTERRUPTIBLE;
	add_wait_queue(&pkmap_map_wait, &wait);
	spin_unlock(&kmap_lock);
	schedule();
	remove_wait_queue(&pkmap_map_wait, &wait);
	spin_lock(&kmap_lock);
	if (page_address(page))
	//唤醒后先检查该页框是否已分配线性页
		return (unsigned long) page_address(page);
}