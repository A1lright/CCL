#include "symbolTable.h"
#include "astSysy.h"
#include <optional>
#include <stack>

using namespace AST;

// 常量表达式求值结果类型
struct ConstValue
{
  int int_val;
  // 可以扩展其他类型如 float、数组等
};

// 辅助结构，跟踪当前函数上下文
struct Context
{
  TokenType returnType; // 当前函数返回类型
  bool inLoop;          // 是否在循环中（处理break/continue）
};

class SymbolManager : public AST::Visitor
{
public:
  // 编译单元节点
  void visit(CompUnit &node);

  // 声明
  void visit(ConstDef &node);
  void visit(ConstDecl &node);
  void visit(VarDef &node);
  void visit(VarDecl &node);

  // 语句
  void visit(ExpStmt &) ;
  void visit(Block &node);
  void visit(AssignStmt &node);
  void visit(IfStmt &node);
  void visit(WhileStmt &node);
  void visit(ReturnStmt &node);
  void visit(IOStmt &node);

  // 表达式
  void visit(LVal &node);
  void visit(PrimaryExp &node);

  void visit(UnaryExp &node);
  void visit(AddExp &node);
  void visit(MulExp &node);
  void visit(LOrExp &node);
  void visit(LAndExp &node);
  void visit(EqExp &node);
  void visit(RelExp &node);

  void visit(CallExp &node);
  void visit(Number &node);
  void visit(FuncParam &node);
  void visit(FuncDef &node);
  void visit(MainFuncDef &node);
  void visit(BType &node);
  void visit(InitVal &node);
  void visit(ConstInitVal &node);
  void visit(BlockItem &node);

private:
  SymbolTable &symtab_;
  std::vector<std::string> &errors_;

  // 上下文栈
  std::stack<Context> contextStack;
  // 当前声明上下文
  std::string current_decl_type_; // “int"或自定义类型
  bool current_is_const_ = false;


  // 当前是否在全局作用域
  bool in_global_scope()
  {
    return symtab_.currentScopeLevel() == 0;
  }

  // 进入作用域
  void push_scope() { symtab_.enterScope(); }

  // 退出作用域
  void pop_scope() { symtab_.exitScope(); }

  // 初始化值处理（示例）
  bool process_const_init_val(VariableSymbol &sym, ConstInitVal *init)
  {
    // 根据sym的维度处理初始化值
    return true; // 简化处理
  }

  bool process_var_init_val(VariableSymbol &sym, InitVal *init)
  {
    // 处理变量初始化
    return true;
  }

public:
  SymbolManager(SymbolTable &st, std::vector<std::string> &err) : symtab_(st), errors_(err) {}
};