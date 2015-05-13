/**************************************
 *	gdb/breakpoint.h
 **************************************/

// 被断点的指令的最大字节数
#define	BREAKPOINT_MAX	16

// 断点类型
enum bptype{...};

// 断点状态
enum enable_state{...};

// 断点处理
enum bpdisp{...};

// 每个断点维护两种结构，struct breakpoint和struct bp_location。例如watchpoint的location可能是多个

// location的类型
enum bp_loc_type{...};

struct bp_location
{
  // 该断点的下一个location
  struct bp_location *next;
  enum bp_loc_type loc_type;
  // 该结构所属struct breakpoint
  struct breakpoint *owner;
  // 标志，breakpoint被插入
  char inserted;
  // 标志，该location的断点是地址列表中的第一个地址
  char duplicate;  
  CORE_ADDR address;
  asection *section;
  // 保存被断点的指令内容
  char shadow_contents[BREAKPOINT_MAX];
  // 通常与address相同，除了...
  CORE_ADDR requested_address;
  // 如果gdb支持线程断点，则需要为location指定线程id
};

// 回调函数
struct breakpoint_ops 
{
  // hit断点时调用
  enum print_stop_action (*print_it) (struct breakpoint *);

  // 执行info breakpoints命令时调用
  void (*print_one) (struct breakpoint *, CORE_ADDR *);

  // 设置断点后调用
  void (*print_mention) (struct breakpoint *);
};

struct breakpoint
  {
    struct breakpoint *next;
    enum bptype type;
    enum enable_state enable_state;
    enum bpdisp disposition;
    // 断点号
    int number;
    struct bp_location *loc;
    // 行号
    int line_number;
    char *source_file;
    // 标志，hit时不打印栈帧信息
    unsigned char silent;
    // 不知道
    int ignore_count;
    // hit时要执行的命令链
    struct command_line *commands;
    // Stack depth (address of frame).  If nonzero, break only if fp equals this.
    struct frame_id frame_id;
    // 只有当该表达式不为0才会break
    struct expression *cond;
    // String we used to set the breakpoint (malloc'd).
    char *addr_string;
    // Language we used to set the breakpoint.
    enum language language;
    // Input radix we used to set the breakpoint.
    int input_radix;
    // breakpoint condition的串形式
    char *cond_string;
    // exp (malloc'd)的串形式
    char *exp_string;
    // watchpoint的表达式
    struct expression *exp;
    struct block *exp_valid_block;
    // watchpoint最后一次的值
    struct value *val;
    // 硬件watchpoint表达式的值的链
    struct value *val_chain;
    struct breakpoint *related_breakpoint;
    struct frame_id watchpoint_frame;
    // 线程断点的线程号
    int thread;
    // hit次数
    int hit_count;
    // bp_catch_load和bp_catch_unload (malloc'd)使用的动态链接库的路径
    char *dll_pathname;
    char *triggered_dll_pathname;
    // catch fork有效
    int forked_inferior_pid;
    // catch exec有效
    char *exec_pathname;
    struct breakpoint_ops *ops;
    int from_tty;
    int flag;
    int pending;
  };

struct bpstats
  {
    // 在同一地方可能有多个断点
    bpstat next;
    // 该bpstats对应的断点
    struct breakpoint *breakpoint_at;
    // 剩下要执行的命令
    struct command_line *commands;
    // watchpoint相关的旧值
    struct value *old_val;
    // 标志，打印栈帧
    char print;
    // 标志，hit断点stop
    char stop;
    // Tell bpstat_print and print_bp_stop_message how to print stuff associated with this element of the bpstat chain.
    enum bp_print_how print_it;
  };