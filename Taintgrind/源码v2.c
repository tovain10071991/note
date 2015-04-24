// run --tool=taintgrind --file-filter=/home/user/Documents/test/asm_test/input --taint-all=yes ~/Documents/test/asm_test/hello


/*****************************************************
 * tnt_main.c
 *****************************************************/

static void tnt_pre_clo_init(void)
{
	// 设置各种函数
	...
	init_shadow_memory();
	/*	初始化影子内存，最终结果为：
	 *	sm_distinguished[SM_DIST_NOACCESS] = 16384B的0x00
	 *	sm_distinguished[SM_DIST_TAINTED] = 16384B的0x55
	 *	sm_distinguished[SM_DIST_UNTAINTED] = 16384B的0xaa
	 *	primary_map[1<<19] = 每项都指向sm_distinguished[SM_DIST_UNTAINTED]
	 *	auxmap_L1[24]=每项都是{base=0, ent(指向auxmap_L2)=NULL}
	 *	auxmap_L2 = 空的Oset
	 *	secVBitTable = 空的Oset
	 */
	 */
	init_soaap_data();
	//初始化了一些数据，不知道干嘛的
	...
	TNT_(malloc_list)  = VG_(HT_construct)( "TNT_(malloc_list)" );
	// 创建一个名为"TNT_(malloc_list)"的表，会自动逐渐扩大，返回VgHashTable
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
			// 将st加入到mce->sb，'C'表示加入的st为原程序的st
			stmt('C', &mce, st);
	}
}