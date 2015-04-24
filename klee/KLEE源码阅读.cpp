/******************************************************
 * lib/Core/Common.h
 ******************************************************/
namespace klee {
	class Solver;
	extern FILE* klee_warning_file;
	extern FILE* klee_message_file;
	// 打印"KLEE: ERROR"和msg，同时定向到stderr和warnings.txt，退出程序
	void klee_error(const char *msg, ...);
	// 打印"KLEE: "和msg，同时定向到stderr和messages.txt
	void klee_message(const char *msg, ...);
	// 打印"KLEE: "和msg，定向到messages.txt
	void klee_message_to_file(const char *msg, ...);
	// 打印"KLEE: WARNING"和msg，同时定向到stderr和warnings.txt
	void klee_warning(const char *msg, ...);
	// 打印"KLEE: WARNING"和msg，同时定向到stderr和warnings.txt，对于每一唯一对(id, msg)只打印一次
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
		// 返回N位二进制的最大值
		inline unsigned maxValueOfNBits(unsigned N);
		// 返回x的低N位
		inline unsigned truncateToNBits(unsigned x, unsigned N);
		inline unsigned withoutRightmostBit(unsigned x) {return x&(x-1);}
		inline unsigned isolateRightmostBit(unsigned x) {return x&-x;}
		// 各种位操作
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
		//构造/析构函数
		ref() : ptr(0) { }
		~ref () {dec();}
		ref(T *p) : ptr(p) {inc();}	//借指针p构造引用
		ref(const ref<T> &r) : ptr(r.ptr) {inc();} //借已存在引用r构造引用
		template<class U> ref (const ref<U> &r) : ptr(r.ptr);
		//其它函数
		T *get ();	//获取ref.ptr
		bool isNull();
		int compare(const ref &rhs);
		//操作符重载
		...
	}
}

namespace llvm {
	// ref<>的simplify_type实现
	template<typename T> struct simplify_type<const ::klee::ref<T>>;
	template<typename T> struct simplify_type< ::klee::ref<T>> : public simplify_type<const ::klee::ref<T>>;
}

/******************************************************
 * include/klee/Expr.h
 ******************************************************/
namespace klee {
	class Expr {
		//类型
		typedef unsigned Width;
		typedef llvm::DenseSet<std::pair<const Expr *, const Expr *> > ExprEquivSet;	//密集型集合
		//常量
		static const Width InvalidWidth = 0;
		static const Width Bool = 1;
		static const Width Int8 = 8;
		static const Width Int16 = 16;
		static const Width Int32 = 32;
		static const Width Int64 = 64;
		static const Width Fl80 = 80;
		static const unsigned MAGIC_HASH_CONSTANT = 39;
		//变量
	public:
		static unsigned count;
		unsigned refCount;
		//枚举
		enum Kind {...};
		//结构体
		struct CreateArg;
	//函数
	public:
		// 构造/析构函数
		Expr() : refCount(0) {Expr::count++;}
		virtual ~Expr() { Expr::count--; }
		// 一般函数
		void dump() const;		// 打印表达式到stderr
		int compare(const Expr &b, ExprEquivSet &equivs) const;
		int compare(const Expr &b) const;
		/// isZero - Is this a constant zero.
		bool isZero() const;
		/// isTrue - Is this the true expression.
		bool isTrue() const;
		/// isFalse - Is this the false expression.
		bool isFalse() const;
		// 静态函数
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
		//虚函数
		virtual Kind getKind() const = 0;				//返回类型
		virtual Width getWidth() const = 0;				//返回宽度
		virtual unsigned getNumKids() const = 0;		//返回孩子数(表达式用树表示)
		virtual ref<Expr> getKid(unsigned i) const = 0;	//返回第i个孩子表达式
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
 