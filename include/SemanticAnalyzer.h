#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "symbolTable.h"
#include "astSysy.h"
#include "evalConstant.h"
#include <iostream>
#include <variant>

using namespace AST;

class SemanticAnalyzer : public Visitor
{
public:
    SymbolTable symbolTable;
    ErrorManager &errorManager; // 错误管理器
    EvalConstant evalConstant;  // 用于常量表达式求值

    SemanticAnalyzer() : errorManager(ErrorManager::getInstance())
    {
    }

    // 遍历编译单元
    void visit(CompUnit &node) override;

    // 声明相关
    void visit(ConstDef &) override;
    void visit(VarDef &node) override;
    void visit(ConstDecl &node) override;
    void visit(VarDecl &node) override;
    void visit(BType &node) override;
    void visit(InitVal &node) override;
    void visit(ConstInitVal &node) override;

    // 函数及形参
    void visit(FuncDef &node) override;
    void visit(MainFuncDef &node) override;
    void visit(FuncParam &node) override;

    // 语句
    void visit(ExpStmt &node) override;
    void visit(Block &node) override;
    void visit(BlockItem &node) override;
    void visit(AssignStmt &node) override;
    void visit(IfStmt &node) override;
    void visit(WhileStmt &node) override;
    void visit(ReturnStmt &node) override;
    void visit(IOStmt &node) override;

    // 表达式
    void visit(Number &node) override;
    void visit(LVal &node) override;
    void visit(PrimaryExp &node) override;
    void visit(UnaryExp &node) override;
    void visit(AddExp &node) override;
    void visit(MulExp &node) override;
    void visit(LOrExp &node) override;
    void visit(LAndExp &node) override;
    void visit(EqExp &node) override;
    void visit(RelExp &node) override;
    void visit(CallExp &node) override;
    void visit(FuncType &);

private:
    int evaluateConstExp(ConstInitVal *initVal);
    int evaluateExp(Node *node);
    bool checkAndEvaluateInitList(const std::vector<std::unique_ptr<InitVal>> &initList, const std::vector<int> &dimensions, size_t currentDim, EvalConstant &evaluator, std::vector<int> &evaluatedValues, ErrorManager &errorManager, const std::string &varName);
    bool checkAndEvaluateConstInitList(const std::vector<std::unique_ptr<ConstInitVal>> &initList, const std::vector<int> &dimensions, size_t currentDim, EvalConstant &evaluator, std::vector<int> &evaluatedValues, ErrorManager &errorManager, const std::string &varName);
};

#endif // SEMANTICANALYZER_H