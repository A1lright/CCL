#include "symbolTable.h"

Symbol::Symbol(SymbolType type, const std::string &name, TokenType dtype, int line, int col)
    : symbolType_(type), name_(name), dataType_(dtype), lineDefined_(line), columnDefined_(col) {}


VariableSymbol::VariableSymbol(const std::string &name, TokenType dtype, bool isConst,
    int line, int col, bool isArray): Symbol(isConst ? CONSTANT : VARIABLE, name, dtype, line, col),
      isConst_(isConst), isArray_(isArray) {}

  

FunctionSymbol::FunctionSymbol(const std::string &name, TokenType returnType,
                               const std::vector<TokenType> &params, int line, int col)
    : Symbol(FUNCTION, name, returnType, line, col),
      paramTypes_(params), hasReturn_(false) {}

SymbolTable::SymbolTable()
{

    // 初始化全局作用域
    enterScope();
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
