###ud_t对象###

ud_t
<br>　　反汇编器状态的封装。
<br>　　首先需要定义一个对象并初始化。

	ud_t ud_obj;
	ud_init(&ud_obj);


###设置机器状态###

	void ud_set_mode(ud_t*, uint8_t mode_bits);
设置字长，`mode_bits`为16，32或64。

	void ud_set_pc(ud_t*, uint64_t pc);
设置PC。

	void ud_set_vendor(ud_t*, unsigned vendor)
设置vendor。只对选择VMX或SVM指令集有用。
<br>`verdor`:
<br>　　`UD_VENDOR_INTEL` - Intel指令集
<br>　　`UD_VENDOR_ATT` - AMD指令集
<br>　　`UD_VENDOR_ANY` - 随意

###设置输入###

可以输入二进制数据、内存缓冲区、标准文件对象，或作为回调函数。默认读取stdin。<br><br>

	void ud_set_input_buffer(ud_t*, unsigned char* buffer, size_t size);
设置输入的缓冲区和大小。<br><br>

	void ud_set_input_file(ud_t*, FILE* filep);
设置输入的文件。<br><br>

	void ud_set_input_hook(ud_t* ud_obj, int (*hook)(ud_t *ud_obj));
注册回调函数，每次需要读取输入的下一字节时执行。回调函数对于输入末尾需要返回`UD_EOI`。<br><br>

	void ud_input_skip(ud_t*, size_t n);
跳过输入的前n个字节。<br><br>

	int ud_input_end(const ud_t*);
测试是否到输入末尾了。<br><br>

	void ud_set_user_opaque_data(ud_t* ud_obj, void* opaque);
Associates a pointer with the udis86 object to be retrieved and used in client functions, such as the input hook callback function.<br><br>

	void* ud_get_user_opaque_data(const ud_t* ud_obj)
返回由`ud_set_user_opaque_data()`关联的任一指针。

###设置translation###

支持将机器码翻译为Intel语法(默认)和AT&T语法。<br><br>

	void ud_set_syntax(ud_t*, void (*translator)(ud_t*));
设置语法。
<br>　　`UD_SYN_INTEL` - Intel语法
<br>　　`UD_SYN_ATT` - AT&T语法

###反汇编###
	unsigned int ud_disassemble(ud_t*);
反汇编(下)一条指令，返回被反汇编的字节数。

	while (ud_disassemble(&ud_obj)) {
		/*
		 * use or print decode info.
		 */
	}
<br>

	unsigned int ud_insn_len(const ud_t* u);
返回本次反汇编字节数。<br><br>

	uint64_t ud_insn_off(const ud_t*);
返回本次反汇编偏移。<br><br>

	const char* ud_insn_hex(ud_t*);
返回指向被反汇编码的16进制的指针。<br><br>

	const uint8_t* ud_insn_ptr(const ud_t* u);
返回指向保存指令的缓冲区的指针。<br><br>

	const char* ud_insn_asm(const ud_t* u);
返回指向反汇编指令字符串的指针。<br><br>

	const ud_operand_t* ud_insn_opr(const ud_t* u, unsigned int n);
返回第n个操作数(从0起)的`ud_operand_t`对象或NULL。<br><br>

	enum ud_mnemonic_code ud_insn_mnemonic(const ud_t *u);
返回助记符。<br><br>

	const char* ud_const lookup_mnemonic(enum ud_mnemonic_code);
返回与助记符代码相关的字符串的指针或NULL。

###检查操作数###

###检查前缀###