#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "lexer.h"
#include "llvm/IR/Value.h"
#pragma once

// 符号类型分类
enum SymbolType
{
    VARIABLE, // 变量
    CONSTANT, // 常量
    FUNCTION, // 函数
    PARAM     // 函数参数
};

// 基础符号类
class Symbol
{
public:
    std::string name_;
    SymbolType symbolType_;
    TokenType dataType_; // 从TokenType继承的类型如INTTK, VOIDTK等
    int lineDefined_;
    int columnDefined_;

    Symbol() = default;

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

    llvm::Value *allocaInst_; // 变量对应的内存地址

    VariableSymbol() = default;
};

// 函数符号
class FunctionSymbol : public Symbol
{
public:
    std::vector<TokenType> paramTypes_; // 参数类型列表
    bool hasReturn_;                    // 是否包含返回值

    FunctionSymbol() = default;
};

// 符号表类（支持嵌套作用域）
class SymbolTable
{
    using Scope = std::unordered_map<std::string, std::unique_ptr<Symbol>>;
    std::vector<Scope> scopes_; // 作用域栈,最终全局函数，全局变量，全局常量都在这里

    std::unordered_map<std::string, Scope> functionSymbols_; // 函数局部符号表，保存函数内部局部变量和参数

    void addBuiltinFunctions();

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

    int currentScopeLevel()
    {
        return scopes_.size() - 1;
    }

    // 添加函数局部符号
    void addFunctionSymbol(const std::string &funcName)
    {
        functionSymbols_[funcName] = std::move(scopes_.back());
    }

    // 在函数局部符号表中查找指定函数内部的符号
    Symbol *lookupFunctionSymbol(const std::string &funcName, const std::string &name)
    {
        auto it = functionSymbols_.find(funcName);
        if (it != functionSymbols_.end())
        {
            auto &scope = it->second;
            auto found = scope.find(name);
            if (found != scope.end())
            {
                return found->second.get();
            }
        }
        return nullptr;
    }
};

#endif