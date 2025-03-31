#include "symbolTable.h"

SymbolTable::SymbolTable()
{
    // 初始化全局作用域
    enterScope();
    addBuiltinFunctions();
}

void SymbolTable::enterScope()
{
    scopes_.push_back(Scope());
}

void SymbolTable::exitScope()
{
    if (!scopes_.empty())
    {
        scopes_.pop_back();
    }
}

bool SymbolTable::addSymbol(std::unique_ptr<Symbol> symbol)
{
    if (scopes_.empty())
        return false;

    auto &current = scopes_.back();
    if (current.find(symbol->name_) != current.end())
    {
        
        ErrorManager::getInstance().addError(ErrorLevel::ERROR,'b',symbol->lineDefined_,"Symbol '" + symbol->name_ + "' is redefined.",ErrorType::SyntaxError);
        return false; // 重复定义
    }

    current[symbol->name_] = std::move(symbol);
    return true;
}

Symbol *SymbolTable::lookup(const std::string &name)
{

    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
        auto entry = it->find(name);
        if (entry != it->end())
        {
            return entry->second.get();
        }
    }
    return nullptr;
}

bool SymbolTable::checkTypeCompatibility(TokenType t1, TokenType t2)
{
    // 根据SysY规范实现类型转换规则
    return t1 == t2 || (t1 == TokenType::KEYWORD_INT && t2 == TokenType::CONSTANT_INTEGER);
}

bool SymbolTable::checkFunctionArgs(const std::string &funcName, const std::vector<TokenType> &argTypes)
{

    if (auto sym = lookup(funcName))
    {
        if (sym->symbolType_ == FUNCTION)
        {
            auto *func = static_cast<FunctionSymbol *>(sym);
            if (func->paramTypes_.size() != argTypes.size())
                return false;
            for (size_t i = 0; i < argTypes.size(); ++i)
            {
                if (!checkTypeCompatibility(func->paramTypes_[i], argTypes[i]))
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void SymbolTable::addBuiltinFunctions()
{
    // 添加 getint 函数：返回 int，无参数
    auto getintSymbol = std::make_unique<FunctionSymbol>();
    getintSymbol->name_ = "getint";
    getintSymbol->symbolType_ = SymbolType::FUNCTION;
    getintSymbol->dataType_ = TokenType::KEYWORD_INT; // 返回 int
    getintSymbol->lineDefined_ = 0;
    getintSymbol->columnDefined_ = 0;

// 对于 getint，无参数列表，paramTypes_ 为空
    getintSymbol->paramTypes_ = {};

    getintSymbol->hasReturn_ = true;
    

    if (!this->addSymbol(std::move(getintSymbol)))
    {
        // 添加失败，说明已经定义过，报告错误,可以调用错误处理函数
        std::cerr << "Error: Function 'getint' is redefined." << std::endl;
    }

    // 添加 printf 函数：返回 int，参数为格式字符串和可变参数
    auto printfSymbol = std::make_unique<FunctionSymbol>();
    printfSymbol->name_ = "printf";
    printfSymbol->symbolType_ = SymbolType::FUNCTION;
    printfSymbol->dataType_ = TokenType::KEYWORD_INT; // 返回 int（按照 C 标准库 printf）
    printfSymbol->lineDefined_ = 0;
    printfSymbol->columnDefined_ = 0;
    printfSymbol->paramTypes_ = {TokenType::CONSTANT_STRING}; // 第一个参数为格式字符串
    printfSymbol->hasReturn_ = true;

    if (!this->addSymbol(std::move(printfSymbol)))
    {
        std::cerr << "Error: Function 'printf' is redefined." << std::endl;
    }
}
