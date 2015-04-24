//VGO_linux

/*************************************************
 * ������������
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
 * ������������
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
   SysRes;			//ϵͳ���÷���ֵ
#define VG_WORDSIZE VEX_HOST_WORDSIZE
#define VG_BIGENDIAN 1
#if defined(VGA_x86)
#  define VG_REGPARM(n)            __attribute__((regparm(n)))
...
#endif

//һЩ����ϵͳ���÷���ֵ�ĺ���

/*************************************************
 * �����в���
 * include/pub_tool_basics.h
 *************************************************/

//����
static Bool tg_process_cmd_line_option(const HChar* arg)	//���������ĺ�������ƥ��ѡ��
 
//�궨��

VG_BOOL_CLO(arg, HChar* ��--option_name��, Bool var)	// --option_name=yes|no��ѡ��ֵ���浽option_val
VG_STR_CLO(arg, ��--option_name��, HChar* var)	// --option_name=string
VG_INT_CLO(arg, ��--option_name��, Int|UInt var)	// --option_name=10
VG_BHEX_CLO(arg, ��--option_name��, var, low, high)	// --option_name=0xf, [low, high]

/*************************************************
 * VEXIR
 * VEX/pub/libvex_ir.h
 *************************************************/

//���ʽ
typedef
   enum { 
      Iex_Binder=0x1900,	//Vex����ģʽƥ��
      Iex_Get,				//���Ĵ������̶�ƫ��
      Iex_GetI,				//���Ĵ������ǹ̶�ƫ��
      Iex_RdTmp,			//��temporary
      Iex_Qop,				//��Ԫ����
      Iex_Triop,			//��Ԫ����
      Iex_Binop,			//��Ԫ����
      Iex_Unop,				//һԪ����
      Iex_Load,				//���ڴ�
      Iex_Const,			//����
      Iex_ITE,				//if-then-else����
      Iex_CCall,			//��������
      Iex_VECRET,
      Iex_BBPTR
   }
   IRExprTag;

//������SBIN
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
IRSB* emptyIRSB(void);		//����յ�IRSB
IRSB* deepCopyIRSB(IRSB*);	//��ȸ���IRSB
IRSB* deepCopyIRSBExceptStmts(IRSB*);	//��ȸ���IRSB���������
void ppIRSB(IRSB*);			//��ӡ������
void addStmtToIRSB(IRSB*, IRStmt*);	//�����׷�ӽ�������

//���
typedef 
   enum {
      Ist_NoOp=0x1E00,	//�ղ���
      Ist_IMark,     	//����ʶ
      Ist_AbiHint,		//An ABI hint, ��֪��
      Ist_Put,			//д�Ĵ������̶�ƫ��
      Ist_PutI,			//д�Ĵ������ǹ̶�ƫ��
      Ist_WrTmp,		//дtemporary
      Ist_Store,		//д�ڴ�
      Ist_LoadG,		//Guarded load
      Ist_StoreG,		//Guarded store
      Ist_CAS,			//�ȽϺͽ���
      Ist_LLSC,			//Load-Linked or Store-Conditional
      Ist_Dirty,		//�������
      Ist_MBE,			//�ڴ������¼���fence/����/�ͷ�������
      Ist_Exit			//(�����е�)IRSB����
   } 
   IRStmtTag;

typedef
   struct _IRStmt {
      IRStmtTag tag;
      union {...} Ist;
   } IRStmt;

//��乹�캯��

IRStmt* deepCopyIRStmt(IRStmt*);	//��ȸ������
void ppIRStmt(IRStmt*);				//��ӡ���