/*
map_new_virtual(page)����ҳ��������ַ���뵽pkmap_page_table�в��ڹ�ϣ���м���һ��Ԫ�ء�
�ú���ɨ��pkmap_count�е����м�����ֱ���ҵ�һ����ֵ
����ļ����������Կռ�ҳ�ı�������
*/

for (;;) {
	int count;
	DECLARE_WAITQUEUE(wait, current);
	for (count = LAST_PKMAP; count > 0; --count) {
		last_pkmap_nr = (last_pkmap_nr + 1) & (LAST_PKMAP - 1);
		//�Ӻ���ǰɨ��ȫ��LAST_PKMAPҳ
		if (!last_pkmap_nr) {
			//���ɨ����һ�ˣ�ˢ������(����ҳ���ö���Ϊ0)���´δ��¿�ʼɨ��
			flush_all_zero_pkmaps();
			count = LAST_PKMAP;
		}
		if (!pkmap_count[last_pkmap_nr]) {
			//�����ҳδ�����ã����ظ�ҳ�����Ե�ַ
			unsigned long vaddr = PKMAP_BASE +(last_pkmap_nr << PAGE_SHIFT);
			set_pte(&(pkmap_page_table[last_pkmap_nr]),mk_pte(page, _ _pgprot(0x63)));
			pkmap_count[last_pkmap_nr] = 1;
			set_page_address(page, (void *) vaddr);
			return vaddr;
		}
	}
	//ɨ����һ�˻�û�ҵ�����ҳ�����𣬵ȴ��ͷ�ҳ�������份��
	current->state = TASK_UNINTERRUPTIBLE;
	add_wait_queue(&pkmap_map_wait, &wait);
	spin_unlock(&kmap_lock);
	schedule();
	remove_wait_queue(&pkmap_map_wait, &wait);
	spin_lock(&kmap_lock);
	if (page_address(page))
	//���Ѻ��ȼ���ҳ���Ƿ��ѷ�������ҳ
		return (unsigned long) page_address(page);
}