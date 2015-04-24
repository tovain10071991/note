// Module类
#include "llvm/IR/Module.h"
// 表示一个可执行文件或库。包含可执行文件的全局变量，函数，依赖库，符号表，字符数据。维持了一个GlobalValRefMap对象，用于保存所有对模块中变量的常量引用。当一个全局变量被破坏，在GlobalValRefMap中就没有其对应的项。
class Module {
/// @name Types And Enumerations
/// @{
public:
  /// The type for 全局变量列表
  typedef iplist<GlobalVariable>			GlobalListType;
  /// The type for 函数列表
  typedef iplist<Function>					FunctionListType;
  /// The type for 别名列表
  typedef iplist<GlobalAlias>				AliasListType;
  /// The type for the list of named metadata.
  typedef ilist<NamedMDNode>				NamedMDListType;
  /// 全局变量迭代器
  typedef GlobalListType::iterator			global_iterator;
  /// 全局常量迭代器
  typedef GlobalListType::const_iterator	const_global_iterator;
  /// 函数迭代器
  typedef FunctionListType::iterator		iterator;
  /// 函数常量迭代器
  typedef FunctionListType::const_iterator	const_iterator;
  /// 全局别名迭代器
  typedef AliasListType::iterator			alias_iterator;
  /// 全局别名常量迭代器
  typedef AliasListType::const_iterator		const_alias_iterator;
  /// The named metadata iterators.
  typedef NamedMDListType::iterator			named_metadata_iterator;
  /// The named metadata constant interators.
  typedef NamedMDListType::const_iterator	const_named_metadata_iterator;
  
  /// 大小端枚举类型
  enum Endianness	{AnyEndianness, LittleEndian, BigEndian};
  /// 指针大小的枚举类型
  enum PointerSize	{AnyPointerSize, Pointer32, Pointer64};
  /// This enumeration defines the supported behaviors of module flags.
  enum ModFlagBehavior {
    /// Emits an error if two values disagree, otherwise the resulting value is
    /// that of the operands.
    Error = 1,
    /// Emits a warning if two values disagree. The result value will be the
    /// operand for the flag from the first module being linked.
    Warning  = 2,
    /// Adds a requirement that another module flag be present and have a
    /// specified value after linking is performed. The value must be a metadata
    /// pair, where the first element of the pair is the ID of the module flag
    /// to be restricted, and the second element of the pair is the value the
    /// module flag should be restricted to. This behavior can be used to
    /// restrict the allowable results (via triggering of an error) of linking
    /// IDs with the **Override** behavior.
    Require = 3,
    /// Uses the specified value, regardless of the behavior or value of the
    /// other module. If both modules specify **Override**, but the values
    /// differ, an error will be emitted.
    Override = 4,
    /// Appends the two values, which are required to be metadata nodes.
    Append = 5,
    /// Appends the two values, which are required to be metadata
    /// nodes. However, duplicate entries in the second list are dropped
    /// during the append operation.
    AppendUnique = 6
  };

  struct ModuleFlagEntry {
    ModFlagBehavior Behavior;
    MDString *Key;			// A single uniqued string
    Value *Val;
    ModuleFlagEntry(ModFlagBehavior B, MDString *K, Value *V)
      : Behavior(B), Key(K), Val(V) {}
  };
/// @}
/// @name Member Variables
/// @{
private:
  LLVMContext &Context;           ///< The LLVMContext from which types and constants are allocated.
  // LLVMContext类，用于线程上下文，拥有和管理LLVM核心架构的核心“全局”数据，包括类型和唯一常量表，每个线程有一个LLVMContext对象
  GlobalListType GlobalList;      ///< The Global Variables in the module
  FunctionListType FunctionList;  ///< The Functions in the module
  AliasListType AliasList;        ///< The Aliases in the module
  NamedMDListType NamedMDList;    ///< The named metadata in the module
  std::string GlobalScopeAsm;     ///< Inline Asm at global scope.
  ValueSymbolTable *ValSymTab;    ///< Symbol table for values
  OwningPtr<GVMaterializer> Materializer;  ///< Used to materialize GlobalValues
  std::string ModuleID;           ///< Human readable identifier for the module
  std::string TargetTriple;       ///< Platform target triple Module compiled on
  std::string DataLayout;         ///< Target data description
  void *NamedMDSymTab;            ///< NamedMDNode names.

  friend class Constant;
/// @}
/// @name Constructors
/// @{
public:
  /// The Module constructor. Note that there is no default constructor. You
  /// must provide a name for the module upon construction.
  explicit Module(StringRef ModuleID, LLVMContext& C);
  /// The module destructor. This will dropAllReferences.
  ~Module();
/// @}
/// @name Module Level Accessors
/// @{
  /// Get the module identifier which is, essentially, the name of the module.
  /// @returns the module identifier as a string
  const std::string &getModuleIdentifier() const { return ModuleID; }

  /// Get the data layout string for the module's target platform.  This encodes
  /// the type sizes and alignments expected by this module.
  /// @returns the data layout as a string
  const std::string &getDataLayout() const { return DataLayout; }

  /// Get the target triple which is a string describing the target host.
  /// @returns a string containing the target triple.
  const std::string &getTargetTriple() const { return TargetTriple; }

  /// Get the target endian information.
  /// @returns Endianess - an enumeration for the endianess of the target
  Endianness getEndianness() const;

  /// Get the target pointer size.
  /// @returns PointerSize - an enumeration for the size of the target's pointer
  PointerSize getPointerSize() const;

  /// Get the global data context.
  /// @returns LLVMContext - a container for LLVM's global information
  LLVMContext &getContext() const { return Context; }

  /// Get any module-scope inline assembly blocks.
  /// @returns a string containing the module-scope inline assembly blocks.
  const std::string &getModuleInlineAsm() const { return GlobalScopeAsm; }

/// @}
/// @name Module Level Mutators
/// @{

  /// Set the module identifier.
  void setModuleIdentifier(StringRef ID) { ModuleID = ID; }

  /// Set the data layout
  void setDataLayout(StringRef DL) { DataLayout = DL; }

  /// Set the target triple.
  void setTargetTriple(StringRef T) { TargetTriple = T; }

  /// Set the module-scope inline assembly blocks.
  void setModuleInlineAsm(StringRef Asm) {
    GlobalScopeAsm = Asm;
    if (!GlobalScopeAsm.empty() &&
        GlobalScopeAsm[GlobalScopeAsm.size()-1] != '\n')
      GlobalScopeAsm += '\n';
  }

  /// Append to the module-scope inline assembly blocks, automatically inserting
  /// a separating newline if necessary.
  void appendModuleInlineAsm(StringRef Asm) {
    GlobalScopeAsm += Asm;
    if (!GlobalScopeAsm.empty() &&
        GlobalScopeAsm[GlobalScopeAsm.size()-1] != '\n')
      GlobalScopeAsm += '\n';
  }

/// @}
/// @name Generic Value Accessors
/// @{

  /// getNamedValue - Return the global value in the module with
  /// the specified name, of arbitrary type.  This method returns null
  /// if a global with the specified name is not found.
  GlobalValue *getNamedValue(StringRef Name) const;

  /// getMDKindID - Return a unique non-zero ID for the specified metadata kind.
  /// This ID is uniqued across modules in the current LLVMContext.
  unsigned getMDKindID(StringRef Name) const;

  /// getMDKindNames - Populate client supplied SmallVector with the name for
  /// custom metadata IDs registered in this LLVMContext.
  void getMDKindNames(SmallVectorImpl<StringRef> &Result) const;

  
  typedef DenseMap<StructType*, unsigned, DenseMapInfo<StructType*>> NumeredTypesMapTy;

  /// getTypeByName - Return the type with the specified name, or null if there
  /// is none by that name.
  StructType *getTypeByName(StringRef Name) const;

/// @}
/// @name Function Accessors
/// @{

  /// getOrInsertFunction - Look up the specified function in the module symbol
  /// table.  Four possibilities:
  ///   1. If it does not exist, add a prototype for the function and return it.
  ///   2. If it exists, and has a local linkage, the existing function is
  ///      renamed and a new one is inserted.
  ///   3. Otherwise, if the existing function has the correct prototype, return
  ///      the existing function.
  ///   4. Finally, the function exists but has the wrong prototype: return the
  ///      function with a constantexpr cast to the right prototype.
  Constant *getOrInsertFunction(StringRef Name, FunctionType *T,
                                AttributeSet AttributeList);

  Constant *getOrInsertFunction(StringRef Name, FunctionType *T);

  /// getOrInsertFunction - Look up the specified function in the module symbol
  /// table.  If it does not exist, add a prototype for the function and return
  /// it.  This function guarantees to return a constant of pointer to the
  /// specified function type or a ConstantExpr BitCast of that type if the
  /// named function has a different type.  This version of the method takes a
  /// null terminated list of function arguments, which makes it easier for
  /// clients to use.
  Constant *getOrInsertFunction(StringRef Name,
                                AttributeSet AttributeList,
                                Type *RetTy, ...)  END_WITH_NULL;

  /// getOrInsertFunction - Same as above, but without the attributes.
  Constant *getOrInsertFunction(StringRef Name, Type *RetTy, ...)
    END_WITH_NULL;

  /// getFunction - Look up the specified function in the module symbol table.
  /// If it does not exist, return null.
  Function *getFunction(StringRef Name) const;

/// @}
/// @name Global Variable Accessors
/// @{

  /// getGlobalVariable - Look up the specified global variable in the module
  /// symbol table.  If it does not exist, return null. If AllowInternal is set
  /// to true, this function will return types that have InternalLinkage. By
  /// default, these types are not returned.
  const GlobalVariable *getGlobalVariable(StringRef Name,
                                          bool AllowInternal = false) const {
    return const_cast<Module *>(this)->getGlobalVariable(Name, AllowInternal);
  }

  GlobalVariable *getGlobalVariable(StringRef Name, bool AllowInternal = false);

  /// getNamedGlobal - Return the global variable in the module with the
  /// specified name, of arbitrary type.  This method returns null if a global
  /// with the specified name is not found.
  GlobalVariable *getNamedGlobal(StringRef Name) {
    return getGlobalVariable(Name, true);
  }
  const GlobalVariable *getNamedGlobal(StringRef Name) const {
    return const_cast<Module *>(this)->getNamedGlobal(Name);
  }

  /// getOrInsertGlobal - Look up the specified global in the module symbol
  /// table.
  ///   1. If it does not exist, add a declaration of the global and return it.
  ///   2. Else, the global exists but has the wrong type: return the function
  ///      with a constantexpr cast to the right type.
  ///   3. Finally, if the existing global is the correct declaration, return
  ///      the existing global.
  Constant *getOrInsertGlobal(StringRef Name, Type *Ty);

/// @}
/// @name Global Alias Accessors
/// @{

  /// getNamedAlias - Return the global alias in the module with the
  /// specified name, of arbitrary type.  This method returns null if a global
  /// with the specified name is not found.
  GlobalAlias *getNamedAlias(StringRef Name) const;

/// @}
/// @name Named Metadata Accessors
/// @{

  /// getNamedMetadata - Return the first NamedMDNode in the module with the
  /// specified name. This method returns null if a NamedMDNode with the
  /// specified name is not found.
  NamedMDNode *getNamedMetadata(const Twine &Name) const;

  /// getOrInsertNamedMetadata - Return the named MDNode in the module
  /// with the specified name. This method returns a new NamedMDNode if a
  /// NamedMDNode with the specified name is not found.
  NamedMDNode *getOrInsertNamedMetadata(StringRef Name);

  /// eraseNamedMetadata - Remove the given NamedMDNode from this module
  /// and delete it.
  void eraseNamedMetadata(NamedMDNode *NMD);

/// @}
/// @name Module Flags Accessors
/// @{

  /// getModuleFlagsMetadata - Returns the module flags in the provided vector.
  void getModuleFlagsMetadata(SmallVectorImpl<ModuleFlagEntry> &Flags) const;

  /// Return the corresponding value if Key appears in module flags, otherwise
  /// return null.
  Value *getModuleFlag(StringRef Key) const;

  /// getModuleFlagsMetadata - Returns the NamedMDNode in the module that
  /// represents module-level flags. This method returns null if there are no
  /// module-level flags.
  NamedMDNode *getModuleFlagsMetadata() const;

  /// getOrInsertModuleFlagsMetadata - Returns the NamedMDNode in the module
  /// that represents module-level flags. If module-level flags aren't found,
  /// it creates the named metadata that contains them.
  NamedMDNode *getOrInsertModuleFlagsMetadata();

  /// addModuleFlag - Add a module-level flag to the module-level flags
  /// metadata. It will create the module-level flags named metadata if it
  /// doesn't already exist.
  void addModuleFlag(ModFlagBehavior Behavior, StringRef Key, Value *Val);
  void addModuleFlag(ModFlagBehavior Behavior, StringRef Key, uint32_t Val);
  void addModuleFlag(MDNode *Node);

/// @}
/// @name Materialization
/// @{

  /// setMaterializer - Sets the GVMaterializer to GVM.  This module must not
  /// yet have a Materializer.  To reset the materializer for a module that
  /// already has one, call MaterializeAllPermanently first.  Destroying this
  /// module will destroy its materializer without materializing any more
  /// GlobalValues.  Without destroying the Module, there is no way to detach or
  /// destroy a materializer without materializing all the GVs it controls, to
  /// avoid leaving orphan unmaterialized GVs.
  void setMaterializer(GVMaterializer *GVM);
  /// getMaterializer - Retrieves the GVMaterializer, if any, for this Module.
  GVMaterializer *getMaterializer() const { return Materializer.get(); }

  /// isMaterializable - True if the definition of GV has yet to be materialized
  /// from the GVMaterializer.
  bool isMaterializable(const GlobalValue *GV) const;
  /// isDematerializable - Returns true if this GV was loaded from this Module's
  /// GVMaterializer and the GVMaterializer knows how to dematerialize the GV.
  bool isDematerializable(const GlobalValue *GV) const;

  /// Materialize - Make sure the GlobalValue is fully read.  If the module is
  /// corrupt, this returns true and fills in the optional string with
  /// information about the problem.  If successful, this returns false.
  bool Materialize(GlobalValue *GV, std::string *ErrInfo = 0);
  /// Dematerialize - If the GlobalValue is read in, and if the GVMaterializer
  /// supports it, release the memory for the function, and set it up to be
  /// materialized lazily.  If !isDematerializable(), this method is a noop.
  void Dematerialize(GlobalValue *GV);

  /// MaterializeAll - Make sure all GlobalValues in this Module are fully read.
  /// If the module is corrupt, this returns true and fills in the optional
  /// string with information about the problem.  If successful, this returns
  /// false.
  bool MaterializeAll(std::string *ErrInfo = 0);

  /// MaterializeAllPermanently - Make sure all GlobalValues in this Module are
  /// fully read and clear the Materializer.  If the module is corrupt, this
  /// returns true, fills in the optional string with information about the
  /// problem, and DOES NOT clear the old Materializer.  If successful, this
  /// returns false.
  bool MaterializeAllPermanently(std::string *ErrInfo = 0);

/// @}
/// @name Direct access to the globals list, functions list, and symbol table
/// @{

  /// Get the Module's list of global variables (constant).
  const GlobalListType   &getGlobalList() const       { return GlobalList; }
  /// Get the Module's list of global variables.
  GlobalListType         &getGlobalList()             { return GlobalList; }
  static iplist<GlobalVariable> Module::*getSublistAccess(GlobalVariable*) {
    return &Module::GlobalList;
  }
  /// Get the Module's list of functions (constant).
  const FunctionListType &getFunctionList() const     { return FunctionList; }
  /// Get the Module's list of functions.
  FunctionListType       &getFunctionList()           { return FunctionList; }
  static iplist<Function> Module::*getSublistAccess(Function*) {
    return &Module::FunctionList;
  }
  /// Get the Module's list of aliases (constant).
  const AliasListType    &getAliasList() const        { return AliasList; }
  /// Get the Module's list of aliases.
  AliasListType          &getAliasList()              { return AliasList; }
  static iplist<GlobalAlias> Module::*getSublistAccess(GlobalAlias*) {
    return &Module::AliasList;
  }
  /// Get the Module's list of named metadata (constant).
  const NamedMDListType  &getNamedMDList() const      { return NamedMDList; }
  /// Get the Module's list of named metadata.
  NamedMDListType        &getNamedMDList()            { return NamedMDList; }
  static ilist<NamedMDNode> Module::*getSublistAccess(NamedMDNode*) {
    return &Module::NamedMDList;
  }
  /// Get the symbol table of global variable and function identifiers
  const ValueSymbolTable &getValueSymbolTable() const { return *ValSymTab; }
  /// Get the Module's symbol table of global variable and function identifiers.
  ValueSymbolTable       &getValueSymbolTable()       { return *ValSymTab; }

/// @}
/// @name Global Variable Iteration
/// @{

  global_iterator       global_begin()       { return GlobalList.begin(); }
  const_global_iterator global_begin() const { return GlobalList.begin(); }
  global_iterator       global_end  ()       { return GlobalList.end(); }
  const_global_iterator global_end  () const { return GlobalList.end(); }
  bool                  global_empty() const { return GlobalList.empty(); }

/// @}
/// @name Function Iteration
/// @{

  iterator                begin()       { return FunctionList.begin(); }
  const_iterator          begin() const { return FunctionList.begin(); }
  iterator                end  ()       { return FunctionList.end();   }
  const_iterator          end  () const { return FunctionList.end();   }
  size_t                  size() const  { return FunctionList.size(); }
  bool                    empty() const { return FunctionList.empty(); }

/// @}
/// @name Alias Iteration
/// @{

  alias_iterator       alias_begin()            { return AliasList.begin(); }
  const_alias_iterator alias_begin() const      { return AliasList.begin(); }
  alias_iterator       alias_end  ()            { return AliasList.end();   }
  const_alias_iterator alias_end  () const      { return AliasList.end();   }
  size_t               alias_size () const      { return AliasList.size();  }
  bool                 alias_empty() const      { return AliasList.empty(); }


/// @}
/// @name Named Metadata Iteration
/// @{

  named_metadata_iterator named_metadata_begin() { return NamedMDList.begin(); }
  const_named_metadata_iterator named_metadata_begin() const {
    return NamedMDList.begin();
  }

  named_metadata_iterator named_metadata_end() { return NamedMDList.end(); }
  const_named_metadata_iterator named_metadata_end() const {
    return NamedMDList.end();
  }

  size_t named_metadata_size() const { return NamedMDList.size();  }
  bool named_metadata_empty() const { return NamedMDList.empty(); }


/// @}
/// @name Utility functions for printing and dumping Module objects
/// @{

  /// Print the module to an output stream with an optional
  /// AssemblyAnnotationWriter.
  void print(raw_ostream &OS, AssemblyAnnotationWriter *AAW) const;

  /// Dump the module to stderr (for debugging).
  void dump() const;
  
  /// This function causes all the subinstructions to "let go" of all references
  /// that they are maintaining.  This allows one to 'delete' a whole class at
  /// a time, even though there may be circular references... first all
  /// references are dropped, and all use counts go to zero.  Then everything
  /// is delete'd for real.  Note that no operations are valid on an object
  /// that has "dropped all references", except operator delete.
  void dropAllReferences();
/// @}
};