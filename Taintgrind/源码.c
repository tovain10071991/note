/*****************************************************
 * taintgring.h
 *****************************************************/

// 客户请求启动宏(其实是语句)

/*****************************************************
 * tnt_include.h
 *****************************************************/

// 脏标志宏
 
// run --tool=taintgrind ~/Documents/test/asm_test/hello
 
 /*****************************************************
 * tnt_main.c
 *****************************************************/
static void tnt_pre_clo_init(void)
{
   VG_(basic_tool_funcs)        (tnt_post_clo_init,
                                 TNT_(instrument),
                                 tnt_fini);

   /* Track syscalls for tainting purposes */
   // TODO: will this conflict?
   VG_(needs_syscall_wrapper)     ( tnt_pre_syscall,		//pre_什么也没做
                                    tnt_post_syscall );
   init_shadow_memory();
   init_soaap_data();
   VG_(needs_command_line_options)(tnt_process_cmd_line_options,
                                   tnt_print_usage,
                                   tnt_print_debug_usage);
   VG_(needs_malloc_replacement)  (TNT_(malloc),
                                   TNT_(__builtin_new),
                                   TNT_(__builtin_vec_new),
                                   TNT_(memalign),
                                   TNT_(calloc),
                                   TNT_(free),
                                   TNT_(__builtin_delete),
                                   TNT_(__builtin_vec_delete),
                                   TNT_(realloc),
                                   TNT_(malloc_usable_size),
                                   TNT_MALLOC_REDZONE_SZB );
   VG_(needs_client_requests)  (TNT_(handle_client_requests));

   // Taintgrind: Needed for tnt_malloc_wrappers.c
   TNT_(malloc_list)  = VG_(HT_construct)( "TNT_(malloc_list)" );
   // 创建一个名为"TNT_(malloc_list)"的表，会自动逐渐扩大，返回VgHashTable

//   VG_(track_new_mem_mmap)        ( tnt_new_mem_mmap );
   VG_(track_copy_mem_remap)      ( TNT_(copy_address_range_state) );
   VG_(track_die_mem_stack_signal)( TNT_(make_mem_untainted) );
   VG_(track_die_mem_brk)         ( TNT_(make_mem_untainted) );
   VG_(track_die_mem_munmap)      ( TNT_(make_mem_untainted) );
}

static void init_shadow_memory ( void )
{
   Int     i;
   SecMap* sm;
   /*
    *	typedef struct {
	*		UChar vabits8[SM_CHUNKS];	// 一个元素为1个字节，共16384个，所以共16KB
	*	} SecMap;
    */

   /* Build the 3 distinguished secondaries */
   sm = &sm_distinguished[SM_DIST_NOACCESS];	//SM_DIST_NOACCESS = 0
   // static SecMap sm_distinguished[3];		//一个SecMap结构共16384个元素，每个元素即每个字节表示32bit即4B，所以每个sm_distinguished可表示4*16KB=64KB
   for (i = 0; i < SM_CHUNKS; i++) sm->vabits8[i] = VA_BITS8_NOACCESS;	// VA_BITS8_NOACCESS = 0x00

   sm = &sm_distinguished[SM_DIST_TAINTED];		//1
   for (i = 0; i < SM_CHUNKS; i++) sm->vabits8[i] = VA_BITS8_TAINTED;	// VA_BITS8_TAINTED = 0x55

   sm = &sm_distinguished[SM_DIST_UNTAINTED];	//2
   for (i = 0; i < SM_CHUNKS; i++) sm->vabits8[i] = VA_BITS8_UNTAINTED;	// VA_BITS8_UNTAINTED = 0xaa
   
   /* Set up the primary map. */
   /* These entries gradually get overwritten as the used address
      space expands. */
   // Taintgrind: Initialise all memory as untainted
   for (i = 0; i < N_PRIMARY_MAP; i++)
      primary_map[i] = &sm_distinguished[SM_DIST_UNTAINTED];
	  // static SecMap* primary_map[N_PRIMARY_MAP];		//1<<19。每一项表示64KB，共2^19项，总共可表示32GB，每一项 都指向同一个sm_distinguished[SM_DIST_UNTAINTED]

   /* 初始化辅助一级映射 */
   init_auxmap_L1_L2();

   /* auxmap_size = auxmap_used = 0;
      no ... these are statically initialised */

   /* Secondary V bit table */
   secVBitTable = createSecVBitTable();
}
/*
 *	sm_distinguished[SM_DIST_NOACCESS] = 16384B的0x00
 *	sm_distinguished[SM_DIST_TAINTED] = 16384B的0x55
 *	sm_distinguished[SM_DIST_UNTAINTED] = 16384B的0xaa
 *	primary_map[1<<19] = 每项都指向sm_distinguished[SM_DIST_UNTAINTED]
 *	auxmap_L1[24]=每项都是{base=0, ent(指向auxmap_L2)=NULL}
 *	auxmap_L2 = 空的Oset
 *	secVBitTable = 空的Oset
 */

void init_soaap_data() {
	persistent_sandbox_nesting_depth = 0;
	ephemeral_sandbox_nesting_depth = 0;
	have_created_sandbox = False;
	VG_(memset)(shared_vars_perms, 0, sizeof(Int)*VAR_MAX);
	// UInt shared_vars_perms[VAR_MAX]; //100
	VG_(memset)(shared_fds, 0, sizeof(Int)*FD_MAX);
	// UInt shared_fds[FD_MAX];		//256
	VG_(memset)(allowed_syscalls, 0, sizeof(Bool)*SYSCALLS_MAX);
	// Bool allowed_syscalls[SYSCALLS_MAX];	//500
	next_shared_variable_to_update = NULL;
	client_binary_name = NULL;
}

static void tnt_post_clo_init(void)
{
   if(*TNT_(clo_file_filter) == '\0'){

      if( !TNT_(clo_tainted_ins_only) || !TNT_(clo_critical_ins_only) )
         TNT_(do_print) = 1;

   }else if(*TNT_(clo_file_filter) != '/') {...}

   if( TNT_(clo_critical_ins_only) )
      TNT_(clo_tainted_ins_only) = True;

   // Initialise temporary variables/reg SSA index array
   Int i;
   /*
	*	#define TI_MAX 440
	*	#define RI_MAX 740
	*	#define STACK_SIZE 102400
	*	#define MAX_LEN 256
    *	// Tmp variable indices; the MSB indicates whether it's tainted (1) or not (0)
	*	UInt  ti[TI_MAX];
	*	// Tmp variable values
	*	ULong tv[TI_MAX];
	*	// Reg variable indices; values are obtained in real-time
	*	UInt  ri[RI_MAX];
	*	int   lvar_i[STACK_SIZE];
	*	struct   myStringArray{char m[STACK_SIZE][MAX_LEN];int size;} lvar_s;
    */
   for( i=0; i< TI_MAX; i++ ) {
      ti[i] = 0;
      tv[i] = 0;
   }
   for( i=0; i< RI_MAX; i++ )
      ri[i] = 0;
   for( i=0; i< STACK_SIZE; i++ )
      lvar_i[i] = 0;
   lvar_s.size = 0;

   // If stdout is not a tty, don't highlight text
   istty = tnt_isatty();
   // istty = 1
}

/*****************************************************
 * tnt_translate.c
 *****************************************************/

IRSB* TNT_(instrument)( VgCallbackClosure* closure,
                        IRSB* sb_in,
                        VexGuestLayout* layout, 
                        VexGuestExtents* vge,
                        VexArchInfo* archinfo_host,
                        IRType gWordTy, IRType hWordTy )
	/*
		closure = {nraddr=0x40011d0, readdr=0x40011d0, tid=1}
		sb_in = {tyenv = 0x38be3394, stmts = 0x38be3920, stmts_size = 64, stmts_used = 54, next = 0x38be22c4, jumpkind = Ijk_Ret, offsIP = 68}
		
	*/
{
	Bool    verboze = 0||False;
	Bool    bogus;
	Int     i, j, first_stmt;
	IRStmt* st;
	MCEnv   mce;
	/*
	 *	typedef struct _MCEnv {
	 *		IRSB* sb;
	 *		Bool  trace;
	 *		XArray*  tmpMap;
	 *		Bool bogusLiterals;
	 *		Bool useLLVMworkarounds;
	 *		VexGuestLayout* layout;
	 *		IRType hWordTy;
	 *	} MCEnv;
	 */
	IRSB*   sb_out;

	// For complainIfTainted
	IRDirty* di2;

	/* Set up SB */
	sb_out = deepCopyIRSBExceptStmts(sb_in);
	//sb_out = {tyenv = 0x38be3dd4, stmts = 0x38be3db4, stmts_size = 8, stmts_used = 0, next = 0x38be3ee0, jumpkind = Ijk_Ret, offsIP = 68}

	/* Set up the running environment.  Both .sb and .tmpMap are modified as we go along.  Note that tmps are added to both .sb->tyenv and .tmpMap together, so the valid index-set for those two arrays should always be identical. */
	VG_(memset)(&mce, 0, sizeof(mce));
	mce.sb             = sb_out;
	mce.trace          = verboze;
	mce.layout         = layout;
	mce.hWordTy        = hWordTy;
	mce.bogusLiterals  = False;
	mce.useLLVMworkarounds = False;
	mce.tmpMap = VG_(newXA)( VG_(malloc), "mc.TNT_(instrument).1", VG_(free), sizeof(TempMapEnt));

	for (i = 0; i < sb_in->tyenv->types_used; i++) {
		//在tmpMap中为temporary设置空间
		TempMapEnt ent;
		/*
	   	 *	typedef struct {
		 *		TempKind kind;
		 *		IRTemp   shadowV;
		 *	} TempMapEnt;
		 */
		ent.kind    = Orig;
		ent.shadowV = IRTemp_INVALID;
		VG_(addToXA)( mce.tmpMap, &ent );
	}

   /* Make a preliminary inspection of the statements, to see if there
      are any dodgy-looking literals.  If there are, we generate
      extra-detailed (hence extra-expensive) instrumentation in
      places.  Scan the whole bb even if dodgyness is found earlier,
      so that the flatness assertion is applied to all stmts. */

   bogus = False;

   for (i = 0; i < sb_in->stmts_used; i++) {

      st = sb_in->stmts[i];

      if (!bogus) {
         bogus = checkForBogusLiterals(st);
         if (0 && bogus) {
            VG_(printf)("bogus: ");
            ppIRStmt(st);
            VG_(printf)("\n");
         }
      }
   }

   mce.bogusLiterals = bogus;

   i = 0;
   while (i < sb_in->stmts_used && sb_in->stmts[i]->tag != Ist_IMark) {
      st = sb_in->stmts[i];

      stmt( 'C', &mce, sb_in->stmts[i] );

      i++;
   }

   for (j = 0; j < i; j++) {
      if (sb_in->stmts[j]->tag == Ist_WrTmp) {
         /* findShadowTmpV checks its arg is an original tmp;
            no need to assert that here. */
         IRTemp tmp_o = sb_in->stmts[j]->Ist.WrTmp.tmp;
         IRTemp tmp_v = findShadowTmpV(&mce, tmp_o);
         IRType ty_v  = typeOfIRTemp(sb_out->tyenv, tmp_v);
         assign( 'V', &mce, tmp_v, definedOfType( ty_v ) );
      }
   }

   /* Iterate over the remaining stmts to generate instrumentation. */

   for (/*use existing i*/; i < sb_in->stmts_used; i++) {
      st = sb_in->stmts[i];
      first_stmt = sb_out->stmts_used;

      /* Emulate shadow operations for each stmt ... */
      // Taintgrind: clone the IRStmt. Defined in copy.c
      IRStmt *clone = deepMallocIRStmt(st);

      switch (st->tag) {

         case Ist_WrTmp:  // all the assign tmp stmts, e.g. t1 = GET:I32(0)
            do_shadow_WRTMP( &mce,
                             clone,
                             st->Ist.WrTmp.tmp,
                             st->Ist.WrTmp.data );
//            assign( 'V', &mce, findShadowTmpV(&mce, st->Ist.WrTmp.tmp),
//                               expr2vbits( &mce, st->Ist.WrTmp.data) );
            break;

         case Ist_Put:
            do_shadow_PUT( &mce,
                           clone,
                           st->Ist.Put.offset,
                           st->Ist.Put.data,
                           NULL /* shadow atom */, NULL /* guard */ );
            break;

         case Ist_PutI:
              do_shadow_PUTI( &mce, st->Ist.PutI.details);
            break;

         case Ist_Store:
            do_shadow_Store( &mce, clone,
                                   st->Ist.Store.end,
                                   st->Ist.Store.addr, 0/* addr bias */,
                                   st->Ist.Store.data,
                                   NULL /* shadow data */,
                                   NULL/*guard*/ );
            break;

         case Ist_StoreG:
            do_shadow_StoreG( &mce, st->Ist.StoreG.details );
            break;

         case Ist_LoadG:
            do_shadow_LoadG( &mce, st->Ist.LoadG.details );
            break;

         case Ist_Exit: // Conditional jumps, if(t<guard>) goto {Boring} <addr>:I32
            di2 = create_dirty_EXIT( &mce, clone, st->Ist.Exit.guard, 
                                     st->Ist.Exit.jk, st->Ist.Exit.dst );
            complainIfTainted( &mce, st->Ist.Exit.guard, di2 );
            break;

         case Ist_IMark:
            break;

         case Ist_NoOp:
         case Ist_MBE:
            break;

         case Ist_Dirty:
            do_shadow_Dirty( &mce, st->Ist.Dirty.details );
            break;

         // Taintgrind: Needed for x64
         case Ist_AbiHint:
            // Do nothing
//            do_AbiHint( &mce, st->Ist.AbiHint.base,
//                              st->Ist.AbiHint.len,
//                              st->Ist.AbiHint.nia );
            break;

         case Ist_CAS:
            do_shadow_CAS( &mce, st->Ist.CAS.details );
            /* Note, do_shadow_CAS copies the CAS itself to the output
               block, because it needs to add instrumentation both
               before and after it.  Hence skip the copy below.  Also
               skip the origin-tracking stuff (call to schemeS) above,
               since that's all tangled up with it too; do_shadow_CAS
               does it all. */
            break;

         case Ist_LLSC:
            do_shadow_LLSC( &mce,
                            st->Ist.LLSC.end,
                            st->Ist.LLSC.result,
                            st->Ist.LLSC.addr,
                            st->Ist.LLSC.storedata );
            break;

         default:
            VG_(printf)("\n");
            ppIRStmt(st);
            VG_(printf)("\n");
            VG_(tool_panic)("tnt_translate.c: TNT_(instrument): unhandled IRStmt");

      } /* switch (st->tag) */

      /* ... and finally copy the stmt itself to the output.  Except,
         skip the copy of IRCASs; see comments on case Ist_CAS
         above. */
      /* Taintgrind: Similarly, we execute the WrTmp's and Dirty's first,
                 so that we can print out the resulting values post-execution */
      if (st->tag != Ist_CAS &&
          st->tag != Ist_WrTmp &&
          st->tag != Ist_Dirty )
		// 将st加入到mce->sb，'C'表示加入的st为原程序的st
         stmt('C', &mce, st);
   }
   /* Now we need to complain if the jump target is undefined. */
   first_stmt = sb_out->stmts_used;

   di2 = create_dirty_NEXT( &mce, sb_in->next );
   if ( di2 ) complainIfTainted( &mce, sb_in->next, di2 );

   VG_(deleteXA)( mce.tmpMap );

   tl_assert(mce.sb == sb_out);
   return sb_out;
}



static
void tnt_post_syscall(ThreadId tid, UInt syscallno,
                            UWord* args, UInt nArgs, SysRes res)
{
	TNT_(syscall_allowed_check)(tid, syscallno);	//检查是否为允许的调用号(非沙盒环境下，所有系统调用都允许)

	switch ((int)syscallno) {
#if defined VGO_freebsd
    ...
#else
    // Should be defined by respective vki/vki-arch-os.h
    case __NR_read:
      TNT_(syscall_read)(tid, args, nArgs, res);
      break;
    case __NR_write:
      TNT_(syscall_write)(tid, args, nArgs, res);
      break;
    case __NR_open:
    case __NR_openat:
      TNT_(syscall_open)(tid, args, nArgs, res);
      break;
    case __NR_close:
      TNT_(syscall_close)(tid, args, nArgs, res);
      break;
    case __NR_lseek:
#ifdef __NR_llseek
    case __NR_llseek:
#endif
      TNT_(syscall_llseek)(tid, args, nArgs, res);
      break;
    case __NR_pread64:
      TNT_(syscall_pread)(tid, args, nArgs, res);
      break;
#ifdef __NR_recv
    case __NR_recv:
      TNT_(syscall_recv)(tid, args, nArgs, res);
      break;
#endif
#ifdef __NR_recvfrom
    case __NR_recvfrom:
      TNT_(syscall_recvfrom)(tid, args, nArgs, res);
      break;
#endif
#endif // VGO_freebsd
  }
}