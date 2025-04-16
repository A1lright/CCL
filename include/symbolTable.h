#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "lexer.h"
#include "llvm/IR/Value.h"
#include "ErrorManager.h"

// 符号类型分类
enum SymbolType
{
    VARIABLE, // 变量
    CONSTANT, // 常量
    FUNCTION, // 函数
    PARAM,    // 函数参数
    ARRAY     // 数组
};

// 基础符号类
class Symbol
{
public:
    std::string name_;
    SymbolType symbolType_;
    TokenType dataType_;
    int lineDefined_;

    Symbol() = default;

    virtual ~Symbol() = default;
};

// 变量/常量符号
class VariableSymbol : public Symbol
{
public:
    bool isConst_;
    int initValue_;

    llvm::Value *allocaInst_; // 变量对应的内存地址

    VariableSymbol() = default;
};

// 数组符号
class ArraySymbol : public Symbol
{
public:
    bool isConst_;
    std::vector<int> dimensions_; // 数组的维度
    std::vector<int> initValues_; // 数组初始化值

    llvm::Value *allocaInst_; // 数组对应的内存地址

    ArraySymbol() = default;
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
    std::vector<Scope> scopes_;

    void addBuiltinFunctions();

public:
    SymbolTable();

    // 进入新作用域
    void enterScope();

    // 退出当前作用域
    void exitScope();

    // 添加符号
    bool addSymbol(std::unique_ptr<Symbol> symbol);

    // 查找符号从内到外
    Symbol *lookup(const std::string &name);

    // 在当前作用域内
    Symbol *lookupInCurrentScope(const std::string &name);
};

#endif