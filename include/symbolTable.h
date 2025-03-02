#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "lexer.h"
#pragma once

// 符号类型分类
enum SymbolType
{
    VARIABLE,
    CONSTANT,
    FUNCTION,
    ARRAY
};

// 基础符号类
class Symbol
{
public:
    SymbolType symbolType_;
    std::string name_;
    TokenType dataType_; // 从TokenType继承的类型如INTTK, VOIDTK等
    int lineDefined_;
    int columnDefined_;

    Symbol(SymbolType type, const std::string &name, TokenType dtype, int line, int col);

    virtual ~Symbol() = default;
};

// 变量/常量符号
class VariableSymbol : public Symbol
{
public:
    bool isConst_;
    bool isArray_;
    std::vector<int> dimensions_; // 数组维度信息
    union
    {
        int intValue;
        bool boolValue;
    } initValue_;

    VariableSymbol(const std::string &name, TokenType dtype, bool isConst,
                   int line = 0, int col = 0, bool isArray = false);
};

// 函数符号
class FunctionSymbol : public Symbol
{
public:
    std::vector<TokenType> paramTypes_; // 参数类型列表
    bool hasReturn_;                    // 是否包含返回值

    FunctionSymbol(const std::string &name, TokenType returnType,
                   const std::vector<TokenType> &params, int line, int col);
};

// 符号表类（支持嵌套作用域）
class SymbolTable
{
    using Scope = std::unordered_map<std::string, std::unique_ptr<Symbol>>;
    std::vector<Scope> scopes_;

public:
    SymbolTable();

    // 进入新作用域
    void enterScope();

    // 退出当前作用域
    void exitScope();

    // 添加符号（返回是否成功）
    bool addSymbol(std::unique_ptr<Symbol> symbol);

    // 查找符号（从内到外）
    Symbol *lookup(const std::string &name);

    // 类型兼容性检查
    static bool checkTypeCompatibility(TokenType t1, TokenType t2);

    // 函数参数检查
    bool checkFunctionArgs(const std::string &funcName,
                           const std::vector<TokenType> &argTypes);
};


#endif