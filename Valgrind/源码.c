/*	$ export VALGRIND_LAUNCHER=coregrind/valgrind
	$ gdb .in_place/none-amd64-linux
	(gdb) handle SIGILL SIGSEGV nostop noprint
	(gdb) b valgrind_main
	(gdb) run --tool=none /bin/ls
*/

/*---------------------------------------------------
 *coregrind/m_main.c
 *---------------------------------------------------*/
static
Int valgrind_main ( Int argc, HChar **argv, HChar **envp )
/*	argc = 3
	argv[0] = ""/home/user/Documents/valgrind-3.10.1/.in_place/none-amd64-linux""
	argv[1] = "--tool=none"
	argv[2] = "/bin/ls"
	argv[3] = 0
*/
{
   const HChar* toolname      = "memcheck";    // default to Memcheck
   Int     need_help          = 0; // 0 = no, 1 = --help, 2 = --help-debug
   ThreadId tid_main          = VG_INVALID_THREADID;
   Bool    logging_to_fd      = False;
   HChar* xml_fname_unexpanded = NULL;
   Int     loglevel, i;
   struct vki_rlimit zero = { 0, 0 };
   XArray* addr2dihandle = NULL;
   
   VG_(client_envp) = (HChar**)envp;
   
   loglevel = 0;
   for (i = 1; i < argc; i++) {
	  //i=1
      const HChar* tmp_str;
	  //循环第二次，i=2
	  if (argv[i][0] != '-') break;	//argv[2][0]=="/", break
   }
   
   { HChar* limLo  = (HChar*)(&VG_(interim_stack).bytes[0]);
     HChar* limHi  = limLo + sizeof(VG_(interim_stack));
     HChar* volatile 
            aLocal = (HChar*)&limLo;
   }
   the_iicii.clstack_end = VG_(am_startup)( the_iicii.sp_at_startup );
   { void* p = VG_(malloc)( "main.vm.1", 12345 );
     if (p) VG_(free)( p );
   }
   VG_(di_initialise)();
   { HChar *cp = VG_(getenv)(VALGRIND_LIB);
   }
   VG_(name_of_launcher) = VG_(getenv)(VALGRIND_LAUNCHER);
   VG_(getrlimit)(VKI_RLIMIT_DATA, &VG_(client_rlimit_data));
   zero.rlim_max = VG_(client_rlimit_data).rlim_max;
   VG_(setrlimit)(VKI_RLIMIT_DATA, &zero);
   VG_(getrlimit)(VKI_RLIMIT_STACK, &VG_(client_rlimit_stack));
   { VexArch     vex_arch;
     VexArchInfo vex_archinfo;
     Bool ok = VG_(machine_get_hwcaps)();
   }
   
   VG_(split_up_argv)( argc, argv );
   early_process_cmd_line_options(&need_help, &toolname);
   LibVEX_default_VexControl(& VG_(clo_vex_control));

   if (!need_help) {
      the_iicii.argv              = argv;
      the_iicii.envp              = envp;
      the_iicii.toolname          = toolname;

      the_iifii = VG_(ii_create_image)( the_iicii );
   }
   setup_file_descriptors();

   if (!need_help) {
      HChar  buf[50], buf2[VG_(mkstemp_fullname_bufsz)(50-1)];
      HChar  nul[1];
      Int    fd, r;
      const HChar* exename;

      VG_(sprintf)(buf, "proc_%d_cmdline", VG_(getpid)());
      fd = VG_(mkstemp)( buf, buf2 );

      nul[0] = 0;
      exename = VG_(args_the_exename);
      VG_(write)(fd, exename, VG_(strlen)( exename ));
      VG_(write)(fd, nul, 1);

      r = VG_(unlink)( buf2 );		//r=0

      VG_(cl_cmdline_fd) = fd;

      VG_(sprintf)(buf, "proc_%d_auxv", VG_(getpid)());
      fd = VG_(mkstemp)( buf, buf2 );

      UWord *client_auxv = VG_(client_auxv);
      unsigned int client_auxv_len = 0;

      client_auxv_len += 2 * sizeof(UWord);

      VG_(write)(fd, VG_(client_auxv), client_auxv_len);

      r = VG_(unlink)( buf2 );

      VG_(cl_auxv_fd) = fd;
   }
   
   VG_(tl_pre_clo_init)();

   main_process_cmd_line_options ( &logging_to_fd, &xml_fname_unexpanded,
                                   toolname );
   (void) VG_(read_millisecond_timer)();
   print_preamble(logging_to_fd, xml_fname_unexpanded, toolname);

   VG_TDICT_CALL(tool_post_clo_init);
   {
      ok = VG_(sanity_check_needs)( &s );	   
   }
   VG_(init_tt_tc)();
   VG_(redir_initialise)();

   addr2dihandle = VG_(newXA)( VG_(malloc), "main.vm.2",
                               VG_(free), sizeof(Addr_n_ULong) );

   { Addr* seg_starts;
     Int   n_seg_starts;
     Addr_n_ULong anu;

     seg_starts = VG_(get_segment_starts)( &n_seg_starts );

     /* show them all to the debug info reader.  allow_SkFileV has to
        be True here so that we read info from the valgrind executable
        itself. */
     for (i = 0; i < n_seg_starts; i++) {
		 // n_seg_starts=13
        anu.ull = VG_(di_notify_mmap)( seg_starts[i], True/*allow_SkFileV*/,
                                       -1/*Don't use_fd*/);
        /* anu.ull holds the debuginfo handle returned by di_notify_mmap,
           if any. */
        if (anu.ull > 0) {
           anu.a = seg_starts[i];
           VG_(addToXA)( addr2dihandle, &anu );
        }
     }

     VG_(free)( seg_starts );
   }