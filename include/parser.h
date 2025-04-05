#ifndef PARSER_H
#define PARSER_H
#pragma once
#include "astSysy.h"
#include "lexer.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include "symbolTable.h"
#include "ErrorManager.h"
using namespace AST;

class Parser
{
public:
    explicit Parser(std::vector<Token> &tokens, SymbolTable &symbolTable);
    // 编译单元解析
    std::unique_ptr<CompUnit> parseCompUnit();

private:
    SymbolTable &symbolTable_;
    std::vector<Token> &tokens_;
    size_t current_;
    Token token_;

    int value_;

    // =======辅助方法====================================================================
    Token peek(size_t ahead = 0) const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;

    // 恢复错误：同步到下一个声明或语句边界
    void synchronize();

    // 非终结符解析函数
    // 声明解析
    std::unique_ptr<Decl> parseDecl();
    std::unique_ptr<ConstDecl> parseConstDecl();
    std::unique_ptr<ConstDef> parseConstDef();
    std::unique_ptr<VarDecl> parseVarDecl();
    std::unique_ptr<VarDef> parseVarDef();

    // 函数定义解析
    std::unique_ptr<FuncDef> parseFuncDef();
    std::unique_ptr<MainFuncDef> parseMainFuncDef();
    std::unique_ptr<FuncParam> parseFuncParam();

    // 语句解析
    std::unique_ptr<Block> parseBlock();
    std::unique_ptr<BlockItem> parseBlockItem();
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<IfStmt> parseIfStmt();         // if语句
    std::unique_ptr<WhileStmt> parseWhileStmt();   // while循环
    std::unique_ptr<ReturnStmt> parseReturnStmt(); // return语句
    std::unique_ptr<AssignStmt> parseAssignStmt();
    std::unique_ptr<IOStmt> parsePrintfStmt();
    std::unique_ptr<IOStmt> parseGetintStmt();

    // 表达式
    std::unique_ptr<Exp> parseExp();
    std::unique_ptr<Exp> parseLogicalOrExp();
    std::unique_ptr<Exp> parseLogicalAndExp();
    std::unique_ptr<Exp> parseEqExp();
    std::unique_ptr<Exp> parseRelExp();
    std::unique_ptr<AddExp> parseAddExp();
    std::unique_ptr<MulExp> parseMulExp();
    std::unique_ptr<Exp> parseUnaryExp();
    std::unique_ptr<PrimaryExp> parsePrimaryExp();
    std::unique_ptr<LVal> parseLVal();
    std::unique_ptr<Number> parseNumber();

    // 初始化值
    std::unique_ptr<ConstInitVal> parseConstInitVal();
    std::unique_ptr<InitVal> parseInitVal();

    std::unique_ptr<FuncType> parseFuncType();
    std::unique_ptr<BType> parseBType();
    std::unique_ptr<Exp> parseConstExp();
    std::unique_ptr<Stmt> parseExpStmt();
    std::unique_ptr<CallExp> parseCallExp();

    bool isAtEnd();
    Token previous();

    bool isAssignStmt();
};

#endif