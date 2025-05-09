#include "symbolTable.h"

// 构造函数：初始化全局作用域，并添加内置函数
SymbolTable::SymbolTable()
{
    // 建立全局作用域
    enterScope();
    addBuiltinFunctions();
}

void SymbolTable::enterScope()
{
    scopes_.emplace_back();
}

void SymbolTable::exitScope()
{
    if (!scopes_.empty())
    {
        scopes_.pop_back();
    }
    else
    {
        std::cerr << "Error: Attempted to exit scope when no scope exists." << std::endl;
    }
}

bool SymbolTable::addSymbol(std::unique_ptr<Symbol> symbol)
{
    if (scopes_.empty())
    {
        std::cerr << "Error: No scope exists to add symbol " << symbol->name_ << std::endl;
        return false;
    }
    auto &currentScope = scopes_.back();
    if (currentScope.find(symbol->name_) != currentScope.end())
    {
        // 当前作用域中已存在同名符号，视为重定义错误
        std::cerr << "Error: Duplicate definition of symbol " << symbol->name_ << std::endl;
        return false;
    }
    currentScope[symbol->name_] = std::move(symbol);
    return true;
}

Symbol *SymbolTable::lookup(const std::string &name)
{
    for (auto scopeIt = scopes_.rbegin(); scopeIt != scopes_.rend(); ++scopeIt)
    {
        auto it = scopeIt->find(name);
        if (it != scopeIt->end())
        {
            return it->second.get();
        }
    }
    return nullptr;
}

Symbol *SymbolTable::lookupInCurrentScope(const std::string &name)
{
    if (scopes_.empty())
        return nullptr;
    auto &currentScope = scopes_.back();
    auto it = currentScope.find(name);
    if (it != currentScope.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void SymbolTable::addBuiltinFunctions()
{
    // 添加 getint 函数：返回 int，无参数
    auto getintSymbol = std::make_unique<FunctionSymbol>();
    getintSymbol->name_ = "getint";
    getintSymbol->symbolType_ = SymbolType::FUNCTION;
    getintSymbol->dataType_ = TokenType::KEYWORD_INT; // 返回 int
    getintSymbol->lineDefined_ = 0;

    // 对于 getint，无参数列表，paramTypes_ 为空
    getintSymbol->paramTypes_ = {};

    getintSymbol->hasReturn_ = true;

    if (!this->addSymbol(std::move(getintSymbol)))
    {
        // 添加失败，说明已经定义过，报告错误,可以调用错误处理函数
        std::cerr << "Error: Function 'getint' is redefined." << std::endl;
    }

    auto printfSymbol = std::make_unique<FunctionSymbol>();
    printfSymbol->name_ = "printf";
    printfSymbol->symbolType_ = SymbolType::FUNCTION;
    printfSymbol->dataType_ = TokenType::KEYWORD_INT;
    printfSymbol->lineDefined_ = 0;
    printfSymbol->paramTypes_ = {TokenType::CONSTANT_STRING};
    printfSymbol->hasReturn_ = true;

    if (!this->addSymbol(std::move(printfSymbol)))
    {
        std::cerr << "Error: Function 'printf' is redefined." << std::endl;
    }
}
