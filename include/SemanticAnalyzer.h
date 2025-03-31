#include "symbolTable.h"
#include "astSysy.h" // 假设你的AST节点类定义在此文件中
#include <iostream>
#include <variant>

using namespace AST;

class SemanticAnalyzer : public AST::Visitor
{
public:
    explicit SemanticAnalyzer(SymbolTable &symbolTable) : symbolTable_(symbolTable) {}
     ~SemanticAnalyzer() {}

    // 编译单元节点
    void visit(CompUnit &node);

    // 声明
    void visit(ConstDef &node);
    void visit(ConstDecl &node);
    void visit(VarDef &node);
    void visit(VarDecl &node);

    // 语句
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
    SymbolTable &symbolTable_;
    std::string currentFunction_;
    int value_; // 用于存储计算的值
};