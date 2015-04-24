/******************************************************
 * lib/Core/Common.h
 ******************************************************/
namespace klee {
	class Solver;
	extern FILE* klee_warning_file;
	extern FILE* klee_message_file;
	// ��ӡ"KLEE: ERROR"��msg��ͬʱ����stderr��warnings.txt���˳�����
	void klee_error(const char *msg, ...);
	// ��ӡ"KLEE: "��msg��ͬʱ����stderr��messages.txt
	void klee_message(const char *msg, ...);
	// ��ӡ"KLEE: "��msg������messages.txt
	void klee_message_to_file(const char *msg, ...);
	// ��ӡ"KLEE: WARNING"��msg��ͬʱ����stderr��warnings.txt
	void klee_warning(const char *msg, ...);
	// ��ӡ"KLEE: WARNING"��msg��ͬʱ����stderr��warnings.txt������ÿһΨһ��(id, msg)ֻ��ӡһ��
	void klee_warning_once(const void *id, const char *msg, ...);
}

/******************************************************
 * include/klee/Config/Version.h
 ******************************************************/
#define LLVM_TYPE_Q

/******************************************************
 * include/klee/util/Bits.h
 ******************************************************/
namespace klee {
	namespace bits32 {
		// ����Nλ�����Ƶ����ֵ
		inline unsigned maxValueOfNBits(unsigned N);
		// ����x�ĵ�Nλ
		inline unsigned truncateToNBits(unsigned x, unsigned N);
		inline unsigned withoutRightmostBit(unsigned x) {return x&(x-1);}
		inline unsigned isolateRightmostBit(unsigned x) {return x&-x;}
		// ����λ����
		...
	}
}

/******************************************************
 * include/klee/util/Ref.h
 ******************************************************/
namespace klee {
	template<class T> class ref {
		T *ptr;
	public:
		template<class U> friend class ref;
		//����/��������
		ref() : ptr(0) { }
		~ref () {dec();}
		ref(T *p) : ptr(p) {inc();}	//��ָ��p��������
		ref(const ref<T> &r) : ptr(r.ptr) {inc();} //���Ѵ�������r��������
		template<class U> ref (const ref<U> &r) : ptr(r.ptr);
		//��������
		T *get ();	//��ȡref.ptr
		bool isNull();
		int compare(const ref &rhs);
		//����������
		...
	}
}

namespace llvm {
	// ref<>��simplify_typeʵ��
	template<typename T> struct simplify_type<const ::klee::ref<T>>;
	template<typename T> struct simplify_type< ::klee::ref<T>> : public simplify_type<const ::klee::ref<T>>;
}

/******************************************************
 * include/klee/Expr.h
 ******************************************************/
namespace klee {
	class Expr {
		//����
		typedef unsigned Width;
		typedef llvm::DenseSet<std::pair<const Expr *, const Expr *> > ExprEquivSet;	//�ܼ��ͼ���
		//����
		static const Width InvalidWidth = 0;
		static const Width Bool = 1;
		static const Width Int8 = 8;
		static const Width Int16 = 16;
		static const Width Int32 = 32;
		static const Width Int64 = 64;
		static const Width Fl80 = 80;
		static const unsigned MAGIC_HASH_CONSTANT = 39;
		//����
	public:
		static unsigned count;
		unsigned refCount;
		//ö��
		enum Kind {...};
		//�ṹ��
		struct CreateArg;
	//����
	public:
		// ����/��������
		Expr() : refCount(0) {Expr::count++;}
		virtual ~Expr() { Expr::count--; }
		// һ�㺯��
		void dump() const;		// ��ӡ���ʽ��stderr
		int compare(const Expr &b, ExprEquivSet &equivs) const;
		int compare(const Expr &b) const;
		/// isZero - Is this a constant zero.
		bool isZero() const;
		/// isTrue - Is this the true expression.
		bool isTrue() const;
		/// isFalse - Is this the false expression.
		bool isFalse() const;
		// ��̬����
		static void printKind(llvm::raw_ostream &os, Kind k);
		static void printWidth(llvm::raw_ostream &os, Expr::Width w);
		/// returns the smallest number of bytes in which the given width fits
		static inline unsigned getMinBytesForWidth(Width w) {return (w + 7) / 8;}
		/* Kind utilities */
		/* Utility creation functions */
		static ref<Expr> createSExtToPointerWidth(ref<Expr> e);
		static ref<Expr> createZExtToPointerWidth(ref<Expr> e);
		static ref<Expr> createImplies(ref<Expr> hyp, ref<Expr> conc);
		static ref<Expr> createIsZero(ref<Expr> e);
		/// Create a little endian read of the given type at offset 0 of the given object.
		static ref<Expr> createTempRead(const Array *array, Expr::Width w);
		static ref<ConstantExpr> createPointer(uint64_t v);
		static ref<Expr> createFromKind(Kind k, std::vector<CreateArg> args);
		static bool isValidKidWidth(unsigned kid, Width w) { return true; }
		static bool needsResultType() { return false; }
		static bool classof(const Expr *) { return true; }
		//�麯��
		virtual Kind getKind() const = 0;				//��������
		virtual Width getWidth() const = 0;				//���ؿ��
		virtual unsigned getNumKids() const = 0;		//���غ�����(���ʽ������ʾ)
		virtual ref<Expr> getKid(unsigned i) const = 0;	//���ص�i�����ӱ��ʽ
		virtual void print(llvm::raw_ostream &os) const;
		/// Returns the pre-computed hash of the current expression
		virtual unsigned hash() const { return hashValue; }
		/// Returns the (re)computed hash value. 
		virtual unsigned computeHash();
		virtual int compareContents(const Expr &b) const { return 0; }
		// Given an array of new kids return a copy of the expression but using those children. 
		virtual ref<Expr> rebuild(ref<Expr> kids[/* getNumKids() */]) const = 0;
	}
	
	// Terminal Exprs
	class ConstantExpr : public Expr {...}
	class NonConstantExpr : public Expr {...}
	class BinaryExpr : public NonConstantExpr {...}
	class CmpExpr : public BinaryExpr {...}
	class NotOptimizedExpr : public NonConstantExpr {...}
	
	/// Class representing a byte update of an array.
	class UpdateNode {...}
	class Array {...}
	/// Class representing a complete list of updates into an array.
	class UpdateList {...}
	/// Class representing a one byte read from an array.
	class ReadExpr : public NonConstantExpr {...}
	/// Class representing an if-then-else expression.
	class SelectExpr : public NonConstantExpr {...}
	/** Children of a concat expression can have arbitrary widths.  
    Kid 0 is the left kid, kid 1 is the right kid.
	*/
	class ConcatExpr : public NonConstantExpr {...}
	...
		
}

/******************************************************
 * include/klee/Constraints.h
 ******************************************************/


/******************************************************
 * include/klee/ExecutionState.h
 ******************************************************/
 