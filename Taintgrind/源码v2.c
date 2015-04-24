// run --tool=taintgrind --file-filter=/home/user/Documents/test/asm_test/input --taint-all=yes ~/Documents/test/asm_test/hello


/*****************************************************
 * tnt_main.c
 *****************************************************/

static void tnt_pre_clo_init(void)
{
	// ���ø��ֺ���
	...
	init_shadow_memory();
	/*	��ʼ��Ӱ���ڴ棬���ս��Ϊ��
	 *	sm_distinguished[SM_DIST_NOACCESS] = 16384B��0x00
	 *	sm_distinguished[SM_DIST_TAINTED] = 16384B��0x55
	 *	sm_distinguished[SM_DIST_UNTAINTED] = 16384B��0xaa
	 *	primary_map[1<<19] = ÿ�ָ��sm_distinguished[SM_DIST_UNTAINTED]
	 *	auxmap_L1[24]=ÿ���{base=0, ent(ָ��auxmap_L2)=NULL}
	 *	auxmap_L2 = �յ�Oset
	 *	secVBitTable = �յ�Oset
	 */
	 */
	init_soaap_data();
	//��ʼ����һЩ���ݣ���֪�������
	...
	TNT_(malloc_list)  = VG_(HT_construct)( "TNT_(malloc_list)" );
	// ����һ����Ϊ"TNT_(malloc_list)"�ı����Զ������󣬷���VgHashTable
	...
}

static Bool tnt_process_cmd_line_options(const HChar* arg)
{
	...
	/*
	 *	TNT_(clo_file_filter) = "/home/user/Documents/test/asm_test/input"
	 *	TNT_(do_print) = 0
	 *	TNT_(clo_taint_all) = True
	 *	TNT_(clo_tainted_ins_only) = True
	 */
}

static void tnt_post_clo_init(void)
{
	/*
	 *	UInt ti[440] = {0};
	 *	UInt tv[440] = {0};
	 *	UInt ri[740] = {0};
	 *	UInt lvar_i[102400] = {0};
	 *	lvar_s = {char m[102400][256];int size=0;};
	 *	istty = 1;
	 */
}

IRSB* TNT_(instrument)( VgCallbackClosure* closure,
                        IRSB* sb_in,
                        VexGuestLayout* layout, 
                        VexGuestExtents* vge,
                        VexArchInfo* archinfo_host,
                        IRType gWordTy, IRType hWordTy )
{
	...
	for (/*use existing i*/; i < sb_in->stmts_used; i++) {
		st = sb_in->stmts[i];
		first_stmt = sb_out->stmts_used;
		IRStmt *clone = deepMallocIRStmt(st);
		switch (st->tag) {
		case Ist_IMark:
			break;
		case Ist_Put:
			do_shadow_PUT( &mce,
						   clone,
						   st->Ist.Put.offset,
						   st->Ist.Put.data,
						   NULL /* shadow atom */, 
						   NULL /* guard */ );
            break;
		}
		if (st->tag != Ist_CAS && st->tag != Ist_WrTmp && st->tag != Ist_Dirty )
			// ��st���뵽mce->sb��'C'��ʾ�����stΪԭ�����st
			stmt('C', &mce, st);
	}
}