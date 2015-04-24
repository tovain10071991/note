int main(int argc, char** argv)
{
    xed_bool_t sixty_four_bit = false;
    unsigned int mpx_mode = 0;
    xed_bool_t decode_only = true;
    char* input_file_name = 0;
    char* symbol_search_path = 0;
    string decode_text("");
    string encode_text("");
    xed_state_t dstate;
    xed_bool_t encode = false;
    unsigned int ninst = 100*1000*1000; // FIXME: should use maxint...
    //perf_tail is for skipping first insts in performance measure mode
    unsigned int perf_tail = 0;         
    xed_bool_t decode_encode = false;
    int i,j;
    unsigned int loop_decode = 0;
    xed_bool_t decode_raw = false;
    xed_bool_t decode_hex = false;
    xed_bool_t assemble  = false;
    char* target_section = 0;
    xed_bool_t use_binary_mode = true;
    xed_int64_t addr_start = 0;
    xed_int64_t addr_end = 0;
    xed_int64_t fake_base = 0;
    xed_bool_t xml_format =0;
    xed_bool_t resync = 0;
    xed_bool_t ast = 0;
    xed_bool_t histo = 0;
    int line_numbers = 0;
    xed_chip_enum_t xed_chip = XED_CHIP_INVALID;

    char* dot_output_file_name = 0;
    xed_bool_t dot = 0;

    
    /* I have this here to test the functionality, if you are happy with
     * the XED formatting options, then you do not need to set this or call
     * xed_format_set_options() */

    xed_format_options_t format_options;
    memset(&format_options,0,sizeof(xed_format_options_t));
    format_options.hex_address_before_symbolic_name=1;
    format_options.write_mask_curly_k0 = 1;

    xed_example_utils_init();	//设置支持的扩展指令集

    xed_state_init(&dstate,
                   XED_MACHINE_MODE_LEGACY_32,
                   XED_ADDRESS_WIDTH_32b,  /* 2nd parameter ignored */
                   XED_ADDRESS_WIDTH_32b);
	// dstate={mmode = XED_MACHINE_MODE_LEGACY_32, stack_addr_width = XED_ADDRESS_WIDTH_32b}

    resync = 1;
    client_verbose = 3;
    xed_set_verbosity( client_verbose );	//设置XED的信息显示级别
    for( i=1; i < argc ; i++ )    {
        if (strcmp(argv[i], "-no-resync") ==0)   {
            resync = 0;		//不使用重新同步反汇编算法
	    continue;
        }
        if (strcmp(argv[i], "-ast") ==0)   {
            ast = 1;		//显示AVX/SSE类别
	    continue;
        }
        if (strcmp(argv[i], "-histo") ==0)   {
            histo = 1;		//反汇编次数直方图
	    continue;
        }
        else if (strcmp(argv[i],"-d")==0)         {	//反汇编16进制
            test_argc(i,argc);	//测试该选项之后是否还有参数
            for(j=i+1; j< argc;j++) 
                decode_text = decode_text + argv[j];
            break; // leave the i=1...argc loop
        }
        else if (strcmp(argv[i],"-i")==0)        {	//指定输入文件
            test_argc(i,argc);
            input_file_name = argv[i+1];
            i++;
        }
#if defined(XED_USING_DEBUG_HELP)
        else if (strcmp(argv[i],"-sp")==0)        {
            test_argc(i,argc);
            symbol_search_path = argv[i+1];
            i++;
        }
#endif
        else if (strcmp(argv[i],"-s")==0)        {	//指定节
            test_argc(i,argc);
            target_section = argv[i+1];
            i++;
        }
        else if (strcmp(argv[i],"-xml")==0)      {	//xml格式
            format_options.xml_a = 1;
            format_options.xml_f = 1;
            xml_format = 1;
        }
        else if (strcmp(argv[i],"-nwm")==0)      {
            format_options.write_mask_curly_k0 = 0;
        }
#if defined(XED_DWARF) 
        else if (strcmp(argv[i],"-line")==0)      {
            line_numbers = 1;
        }
#endif
        else if (strcmp(argv[i],"-dot")==0)      {	//dot图
            test_argc(i,argc);
            dot_output_file_name = argv[i+1];
            dot = 1;
            i++;
        }
        else if (strcmp(argv[i],"-ir")==0)        {	//raw二进制文件
            test_argc(i,argc);
            input_file_name = argv[i+1];
            decode_raw = true;
            i++;
        }
        else if (strcmp(argv[i],"-ih")==0)        {	//raw16进制文件
            test_argc(i,argc);
            input_file_name = argv[i+1];
            decode_hex = true;
            i++;
        }
#if defined(XED_ENCODER)
        else if (strcmp(argv[i],"-e") ==0)         { //encode指令
            encode = true;
            test_argc(i,argc);
            // merge the rest of the args in to the encode_text string.
            for( j = i+1; j< argc; j++ ) 
                encode_text = encode_text + argv[j] + " ";
            break;  // leave the loop
        }
        else if (strcmp(argv[i],"-de")==0)        {	//先反汇编再汇编
            test_argc(i,argc);
            decode_encode = true;
            for(j=i+1; j< argc;j++) 
                decode_text = decode_text + argv[j];
            break; // leave the i=1...argc loop
        }
        else if (strcmp(argv[i],"-ie")==0)        {	//encode文件
            test_argc(i,argc);
            input_file_name = argv[i+1];
            assemble = true;
            i++;
        }
        else if (strcmp(argv[i],"-ide")==0)        { //输入文件
            test_argc(i,argc);
            input_file_name = argv[i+1];
            decode_only = false;
            i++;
        }
#endif
        else if (strcmp(argv[i],"-n") ==0)         {	//限定指令数
            test_argc(i,argc);
            ninst = XED_STATIC_CAST(unsigned int,
                xed_atoi_general(argv[i+1],1000));
            i++;
        }
        else if (strcmp(argv[i],"-perftail") ==0)         {
            // undocumented. not an interesting knob for most users.
            test_argc(i,argc);
            perf_tail = XED_STATIC_CAST(unsigned int,
                xed_atoi_general(argv[i+1],1000));
            i++;
        }
        else if (strcmp(argv[i],"-b") ==0)         { //偏移基址
            test_argc(i,argc);
            fake_base = xed_atoi_general(argv[i+1],1000);
            printf("ASSUMED BASE = " XED_FMT_LX "\n",fake_base);
            i++;
        }
        else if (strcmp(argv[i],"-as") == 0 || strcmp(argv[i],"-sa") == 0)    {	//起始地址
            test_argc(i,argc);
            addr_start = XED_STATIC_CAST(xed_int64_t,
                                         xed_atoi_general(argv[i+1],1000));
            i++;
        }
        else if (strcmp(argv[i],"-ae") == 0 || strcmp(argv[i],"-ea") == 0)    {	//结束地址
            test_argc(i,argc);
            addr_end = XED_STATIC_CAST(xed_int64_t,xed_atoi_general(argv[i+1],1000));
            i++;
        }

        else if (strcmp(argv[i],"-loop") ==0)         {
            test_argc(i,argc);
            loop_decode = XED_STATIC_CAST(unsigned int,
                                          xed_atoi_general(argv[i+1],1000));
            i++;
        }
        else if (strcmp(argv[i],"-v") ==0)         {	//verbose
            test_argc(i,argc);
            client_verbose = XED_STATIC_CAST(int,xed_atoi_general(argv[i+1],1000));
            xed_set_verbosity(client_verbose);

            i++;
        }
        else if (strcmp(argv[i],"-xv") ==0)        { //XED engine verbosity
            test_argc(i,argc);
            unsigned int xed_engine_verbose =
                XED_STATIC_CAST(unsigned int,xed_atoi_general(argv[i+1],1000));
            xed_set_verbosity(xed_engine_verbose);
            i++;
        }
        else if (strcmp(argv[i],"-chip-check")==0) { //统计无效指令数
            test_argc(i,argc);
            xed_chip = str2xed_chip_enum_t(argv[i+1]);
            printf("Setting chip to %s\n", xed_chip_enum_t2str(xed_chip));
            if (xed_chip == XED_CHIP_INVALID) {
                printf("Invalid chip name specified. Use -chip-check-list to "
                       "see the valid chip names.\n");
                exit(1);
            }
            i++;
        }
        else if (strcmp(argv[i],"-chip-check-list")==0) { //列出有效chip
            list_chips();
            exit(0);
        }
        else if (strcmp(argv[i],"-A")==0)        {
            global_syntax = XED_SYNTAX_ATT;
        }
        else if (strcmp(argv[i],"-I")==0)        {
            global_syntax = XED_SYNTAX_INTEL;
        }
        else if (strcmp(argv[i],"-X")==0)        { // undocumented
            global_syntax = XED_SYNTAX_XED;
        }
        else if (strcmp(argv[i],"-r")==0)         {
            sixty_four_bit = false;
            dstate.mmode = XED_MACHINE_MODE_REAL_16;
            dstate.stack_addr_width = XED_ADDRESS_WIDTH_16b;
            use_binary_mode = false;
        }
        else if (strcmp(argv[i],"-16")==0)         {
            sixty_four_bit = false;
            dstate.mmode = XED_MACHINE_MODE_LEGACY_16;
            use_binary_mode = false;
        }
        else if (strcmp(argv[i],"-32")==0) { // default
            sixty_four_bit = false;
            dstate.mmode = XED_MACHINE_MODE_LEGACY_32;
            use_binary_mode = false;
        }
        else if (strcmp(argv[i],"-64")==0)         {
            sixty_four_bit = true;
            dstate.mmode = XED_MACHINE_MODE_LONG_64;
            use_binary_mode = false;
        }
#if defined(XED_MPX)
        else if (strcmp(argv[i],"-mpx")==0)         {
            mpx_mode = 1;
        }
#endif
        else if (strcmp(argv[i],"-s32")==0) {
            dstate.stack_addr_width = XED_ADDRESS_WIDTH_32b;
            use_binary_mode = false;
        }
        else if (strcmp(argv[i],"-s16")==0) {
            dstate.stack_addr_width = XED_ADDRESS_WIDTH_16b;
            use_binary_mode = false;
        }
#if 0
        else if (strcmp(argv[i],"-ti") ==0)        {
            client_verbose = 5;
            xed_set_verbosity(5);
            test_immdis();
            exit(1);
        }
#endif
        else if (strcmp(argv[i],"-emit") ==0) {
            intel_asm_emit = 1;
        }
        else   {
            usage(argv[0]);
            exit(1);
        }
    }
    if (!encode)     {
        if (input_file_name == 0 && decode_text == "")        {...}
    }
    if (CLIENT_VERBOSE2)
        ...

    xed_tables_init();

    if (CLIENT_VERBOSE2)
        ...

    xed_format_set_options(format_options);	//设置反汇编格式

    if (CLIENT_VERBOSE1) 
        ...
    xed_decoded_inst_t xedd;
    init_xedd(&xedd, &dstate, xed_chip, mpx_mode);	//xedd清0，但是设置其中的机器state/mode信息；设置chip的名字
    xed_uint_t retval_okay = 1;
    unsigned int obytes=0;

    
    if (assemble)
    {
		...
    }
    else if (decode_encode)
    {
		...
    }
    else if (encode)
    {
		...
    }
    else if (decode_text != "")
    {
		...
    }
    else {
#if defined(XED_DECODER)
        xed_disas_info_t decode_info;
        xed_disas_info_init(&decode_info);	//清0
        decode_info.input_file_name  = input_file_name;
        decode_info.symbol_search_path = symbol_search_path;
        decode_info.dstate           = dstate;
        decode_info.ninst            = ninst;
        decode_info.decode_only      = decode_only;
        decode_info.sixty_four_bit   = sixty_four_bit;
        decode_info.target_section   = target_section;
        decode_info.use_binary_mode  = use_binary_mode;
        decode_info.addr_start       = addr_start;
        decode_info.addr_end         = addr_end;
        decode_info.xml_format       = xml_format;
        decode_info.fake_base        = fake_base;
        decode_info.resync           = resync;
        decode_info.line_numbers     = line_numbers;
        decode_info.perf_tail_start  = perf_tail;
        decode_info.ast              = ast;
        decode_info.histo            = histo;
        decode_info.chip             = xed_chip;
        decode_info.mpx_mode         = mpx_mode;   
        
        if (dot) {...}


        if (xml_format) {...}
        if (decode_raw) {...}
        else if (decode_hex) {...}
        else {
            xed_disas_elf(&decode_info);
			// 注册xed_disassembly_callback_fn_t函数；将文件映射到内存中；找到.strtab和.symtab，读取.symtab的符号；遍历并反汇编可执行节
#elif defined(XED_PECOFF_FILE_READER)
            xed_disas_pecoff(&decode_info);
#else
            xedex_derror("No PECOFF, ELF or MACHO support compiled in");
#endif
            printf("# Total Errors: " XED_FMT_LD "\n", decode_info.errors);
            if (decode_info.chip)
                printf("# Total Chip Check Errors: " XED_FMT_LD "\n",
                       decode_info.errors_chip_check);
        }
#endif // XED_DECODER
    }
    if (xml_format) 
	printf("</XEDDISASM>\n");


    if (retval_okay==0) 
        exit(1);
    return 0;
    (void) obytes;
#if !defined(XED_DECODER)
    // pacify the compiler for encoder-only builds:
    (void) sixty_four_bit;
    (void) decode_only;
    (void) symbol_search_path;
    (void) ninst;
    (void) perf_tail;
    (void) loop_decode;
    (void) decode_raw;
    (void) decode_hex;
    (void) target_section;
    (void) addr_start;
    (void) addr_end;
    (void) resync;
    (void) ast;
    (void) histo;
    (void) line_numbers;
    (void) dot_output_file_name;
    (void) dot;
    (void) use_binary_mode;
#endif
}
 

////////////////////////////////////////////////////////////////////////////