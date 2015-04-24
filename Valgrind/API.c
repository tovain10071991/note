//VGO_linux

/*************************************************
 * 基本数据类型
 * VEX/pub/libvex_basictypes.h
 *************************************************/

typedef	unsigned char	UChar;			//8bit
typedef	signed char		Char;			//8bit
typedef	char			HChar;			//8bit
typedef	unsigned short	UShort;			//16bit
typedef	signed short	Short;			//16bit
typedef	unsigned int	UInt;			//32bit
typedef	signed int		Int;			//32bit
typedef	unsigned long long int	ULong;	//64bit
typedef	signed long long int	Long;	//64bit
typedef	UInt			U128[4];		//128bit
typedef	UInt			U256[8];		//256nit
typedef	float			Float;
typedef	double			Double;
typedef	unsigned char	Bool;
#define	True			((Bool)1)
#define	False			((Bool)0)
typedef	UInt			Addr32;
typedef	ULong			Addr64;
typedef	unsigned long	HWord;			//host word
typedef
   union {
      UChar  w8[16];
      UShort w16[8];
      UInt   w32[4];
      ULong  w64[2];
   }
   V128;			//128-bit vector
typedef
   union {
      UChar  w8[32];
      UShort w16[16];
      UInt   w32[8];
      ULong  w64[4];
   }
   V256;			//256-bit vector

//Functions

static inline Bool toBool ( Int x );
static inline UChar toUChar ( Int x );
static inline HChar toHChar ( Int x );
static inline UShort toUShort ( Int x );
static inline Short toShort ( Int x );
static inline UInt toUInt ( Long x );

#if defined(__x86_64__)
#   define VEX_HOST_WORDSIZE 8
#   define VEX_REGPARM(_n) /* */

#elif defined(__i386__)
#   define VEX_HOST_WORDSIZE 4
#   define VEX_REGPARM(_n) __attribute__((regparm(_n)))
...
#endif

#if VEX_HOST_WORDSIZE == 8
   static inline ULong Ptr_to_ULong ( void* p ) {
      return (ULong)p;
   }
   static inline void* ULong_to_Ptr ( ULong n ) {
      return (void*)n;
   }
#elif VEX_HOST_WORDSIZE == 4
   static inline ULong Ptr_to_ULong ( void* p ) {
      UInt w = (UInt)p;
      return (ULong)w;
   }
   static inline void* ULong_to_Ptr ( ULong n ) {
      UInt w = (UInt)n;
      return (void*)w;
   }
...
#endif

/*************************************************
 * 基本数据类型
 * include/pub_tool_basics.h
 *************************************************/

#define VGAPPEND(str1,str2) str1##str2
#define VG_(str)    VGAPPEND(vgPlain_,          str)
#define ML_(str)    VGAPPEND(vgModuleLocal_,    str)

typedef	unsigned long	UWord;
typedef	signed long		Word;
typedef	UWord			Addr;
typedef	UWord			AddrH;
typedef	UWord			SizeT;
typedef	Word			SSizeT;
typedef	Word			PtrdiffT;
typedef	Word			OffT;
typedef	Long			Off64T;
typedef	struct{ UWord uw1; UWord uw2; }	UWordPair;
typedef	UInt			ThreadId;
typedef
   struct {
      UWord _val;
      UWord _valEx;
      Bool  _isError;
   }
   SysRes;			//系统调用返回值
#define VG_WORDSIZE VEX_HOST_WORDSIZE
#define VG_BIGENDIAN 1
#if defined(VGA_x86)
#  define VG_REGPARM(n)            __attribute__((regparm(n)))
...
#endif

//一些关于系统调用返回值的函数

/*************************************************
 * 命令行操作
 * include/pub_tool_basics.h
 *************************************************/

//函数
static Bool tg_process_cmd_line_option(const HChar* arg)	//形如这样的函数用来匹配选项
 
//宏定义

VG_BOOL_CLO(arg, HChar* “--option_name”, Bool var)	// --option_name=yes|no，选项值保存到option_val
VG_STR_CLO(arg, “--option_name”, HChar* var)	// --option_name=string
VG_INT_CLO(arg, “--option_name”, Int|UInt var)	// --option_name=10
VG_BHEX_CLO(arg, “--option_name”, var, low, high)	// --option_name=0xf, [low, high]

/*************************************************
 * VEXIR
 * VEX/pub/libvex_ir.h
 *************************************************/

//表达式
typedef
   enum { 
      Iex_Binder=0x1900,	//Vex用于模式匹配
      Iex_Get,				//读寄存器，固定偏移
      Iex_GetI,				//读寄存器，非固定偏移
      Iex_RdTmp,			//读temporary
      Iex_Qop,				//四元操作
      Iex_Triop,			//三元操作
      Iex_Binop,			//二元操作
      Iex_Unop,				//一元操作
      Iex_Load,				//读内存
      Iex_Const,			//常量
      Iex_ITE,				//if-then-else操作
      Iex_CCall,			//函数调用
      Iex_VECRET,
      Iex_BBPTR
   }
   IRExprTag;

//超级块SBIN
typedef
   struct {
      IRTypeEnv* tyenv;
      IRStmt**   stmts;
      Int        stmts_size;
      Int        stmts_used;
      IRExpr*    next;
      IRJumpKind jumpkind;
      Int        offsIP;
   } IRSB;
IRSB* emptyIRSB(void);		//分配空的IRSB
IRSB* deepCopyIRSB(IRSB*);	//深度复制IRSB
IRSB* deepCopyIRSBExceptStmts(IRSB*);	//深度复制IRSB，除了语句
void ppIRSB(IRSB*);			//打印超级块
void addStmtToIRSB(IRSB*, IRStmt*);	//将语句追加进超级块

//语句
typedef 
   enum {
      Ist_NoOp=0x1E00,	//空操作
      Ist_IMark,     	//语句标识
      Ist_AbiHint,		//An ABI hint, 不知道
      Ist_Put,			//写寄存器，固定偏移
      Ist_PutI,			//写寄存器，非固定偏移
      Ist_WrTmp,		//写temporary
      Ist_Store,		//写内存
      Ist_LoadG,		//Guarded load
      Ist_StoreG,		//Guarded store
      Ist_CAS,			//比较和交换
      Ist_LLSC,			//Load-Linked or Store-Conditional
      Ist_Dirty,		//调用语句
      Ist_MBE,			//内存总线事件，fence/请求/释放总线锁
      Ist_Exit			//(可能有的)IRSB出口
   } 
   IRStmtTag;

typedef
   struct _IRStmt {
      IRStmtTag tag;
      union {...} Ist;
   } IRStmt;

//语句构造函数

IRStmt* deepCopyIRStmt(IRStmt*);	//深度复制语句
void ppIRStmt(IRStmt*);				//打印语句