/**************************************
 *	gdb/breakpoint.h
 **************************************/

// ���ϵ��ָ�������ֽ���
#define	BREAKPOINT_MAX	16

// �ϵ�����
enum bptype{...};

// �ϵ�״̬
enum enable_state{...};

// �ϵ㴦��
enum bpdisp{...};

// ÿ���ϵ�ά�����ֽṹ��struct breakpoint��struct bp_location������watchpoint��location�����Ƕ��

// location������
enum bp_loc_type{...};

struct bp_location
{
  // �öϵ����һ��location
  struct bp_location *next;
  enum bp_loc_type loc_type;
  // �ýṹ����struct breakpoint
  struct breakpoint *owner;
  // ��־��breakpoint������
  char inserted;
  // ��־����location�Ķϵ��ǵ�ַ�б��еĵ�һ����ַ
  char duplicate;  
  CORE_ADDR address;
  asection *section;
  // ���汻�ϵ��ָ������
  char shadow_contents[BREAKPOINT_MAX];
  // ͨ����address��ͬ������...
  CORE_ADDR requested_address;
  // ���gdb֧���̶߳ϵ㣬����ҪΪlocationָ���߳�id
};

// �ص�����
struct breakpoint_ops 
{
  // hit�ϵ�ʱ����
  enum print_stop_action (*print_it) (struct breakpoint *);

  // ִ��info breakpoints����ʱ����
  void (*print_one) (struct breakpoint *, CORE_ADDR *);

  // ���öϵ�����
  void (*print_mention) (struct breakpoint *);
};

struct breakpoint
  {
    struct breakpoint *next;
    enum bptype type;
    enum enable_state enable_state;
    enum bpdisp disposition;
    // �ϵ��
    int number;
    struct bp_location *loc;
    // �к�
    int line_number;
    char *source_file;
    // ��־��hitʱ����ӡջ֡��Ϣ
    unsigned char silent;
    // ��֪��
    int ignore_count;
    // hitʱҪִ�е�������
    struct command_line *commands;
    // Stack depth (address of frame).  If nonzero, break only if fp equals this.
    struct frame_id frame_id;
    // ֻ�е��ñ��ʽ��Ϊ0�Ż�break
    struct expression *cond;
    // String we used to set the breakpoint (malloc'd).
    char *addr_string;
    // Language we used to set the breakpoint.
    enum language language;
    // Input radix we used to set the breakpoint.
    int input_radix;
    // breakpoint condition�Ĵ���ʽ
    char *cond_string;
    // exp (malloc'd)�Ĵ���ʽ
    char *exp_string;
    // watchpoint�ı��ʽ
    struct expression *exp;
    struct block *exp_valid_block;
    // watchpoint���һ�ε�ֵ
    struct value *val;
    // Ӳ��watchpoint���ʽ��ֵ����
    struct value *val_chain;
    struct breakpoint *related_breakpoint;
    struct frame_id watchpoint_frame;
    // �̶߳ϵ���̺߳�
    int thread;
    // hit����
    int hit_count;
    // bp_catch_load��bp_catch_unload (malloc'd)ʹ�õĶ�̬���ӿ��·��
    char *dll_pathname;
    char *triggered_dll_pathname;
    // catch fork��Ч
    int forked_inferior_pid;
    // catch exec��Ч
    char *exec_pathname;
    struct breakpoint_ops *ops;
    int from_tty;
    int flag;
    int pending;
  };

struct bpstats
  {
    // ��ͬһ�ط������ж���ϵ�
    bpstat next;
    // ��bpstats��Ӧ�Ķϵ�
    struct breakpoint *breakpoint_at;
    // ʣ��Ҫִ�е�����
    struct command_line *commands;
    // watchpoint��صľ�ֵ
    struct value *old_val;
    // ��־����ӡջ֡
    char print;
    // ��־��hit�ϵ�stop
    char stop;
    // Tell bpstat_print and print_bp_stop_message how to print stuff associated with this element of the bpstat chain.
    enum bp_print_how print_it;
  };