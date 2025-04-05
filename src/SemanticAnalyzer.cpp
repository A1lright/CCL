#include "SemanticAnalyzer.h"
#include <optional>
#include <variant>

void SemanticAnalyzer::visit(CompUnit &node)
{
    // 处理所有声明
    for (auto &decl : node.decls_)
    {
        decl->accept(*this);
    }

    // 处理所有函数定义
    for (auto &funcDef : node.funcDefs_)
    {
        funcDef->accept(*this);
    }

    // 处理主函数定义
    if (node.mainfuncDef_)
    {
        node.mainfuncDef_->accept(*this);
    }
}

void SemanticAnalyzer::visit(ConstDef &node)
{
    // 检查常量是否已定义

    // 处理常量初始化值
    if (node.initVal_)
    {
        node.initVal_->accept(*this);
    }

    auto symbol = symbolTable_.lookup(node.name_);
    static_cast<VariableSymbol *>(symbol)->initValue_.intValue = value_;

    // 在符号表中添加常量
    // auto symbol = std::make_unique<VariableSymbol>();
    // symbol->name_ = node.name_;
    // symbol->symbolType_ = CONSTANT;
    // symbol->isConst_ = true;
    // symbolTable_.addSymbol(std::move(symbol));
}

void SemanticAnalyzer::visit(ConstDecl &node)
{
    for (auto &constDef : node.constDefs_)
    {
        constDef->accept(*this);
    }
}

void SemanticAnalyzer::visit(VarDef &node)
{
    // 检查变量是否已定义
    //  if (symbolTable_.lookup(node.name_)) {
    //     std::cerr << "Error: Variable '" << node.name_ << "' already defined.\n";
    //     return;
    // }

    // 在符号表中添加变量
    // auto symbol = std::make_unique<VariableSymbol>();
    // symbol->name_ = node.name_;
    // symbol->symbolType_ = SymbolType::VARIABLE;
    // symbol->dataType_ = node.bType_->typeName_;  // 假设 `bType_` 存储类型
    // symbolTable_.addSymbol(std::move(symbol));

    // 处理变量的初始化值
    if (node.initVal_)
    {
        node.initVal_->accept(*this);
    }

    auto symbol = symbolTable_.lookup(node.name_);

    auto variableSymbol = static_cast<VariableSymbol *>(symbol);

    if (node.hasInit)
    {
        if (variableSymbol->isArray_)
        {
        }
        else
        {
            variableSymbol->initValue_.intValue = value_;
        }
    }
}

void SemanticAnalyzer::visit(VarDecl &node)
{
    // 处理每个变量定义
    for (auto &varDef : node.varDefs_)
    {
        varDef->accept(*this);
    }
}

void SemanticAnalyzer::visit(ExpStmt &)
{
}

void SemanticAnalyzer::visit(Block &node)
{
    // 进入新的作用域
    // symbolTable_.enterScope();

    // 处理每个块内的项目（声明或语句）
    for (auto &item : node.items_)
    {
        item->accept(*this);
    }

    // 退出当前作用域
    // symbolTable_.exitScope();
}

void SemanticAnalyzer::visit(AssignStmt &node)
{
    // 检查左值是否合法
    node.lval_->accept(*this);

    // 计算右值
    node.exp_->accept(*this);

    // 赋值操作
    value_ = value_; // 此时 `value_` 已经被更新为右值的计算结果
}

void SemanticAnalyzer::visit(IfStmt &node)
{
    // 计算条件表达式
    node.cond_->accept(*this);

    // 根据条件判断是否执行相应的代码块
    if (value_ != 0)
    {                                    // 如果条件为真
        node.thenBranch_->accept(*this); // 执行 then 分支
    }
    else if (node.elseBranch_)
    {                                    // 如果有 else 分支
        node.elseBranch_->accept(*this); // 执行 else 分支
    }
}

void SemanticAnalyzer::visit(WhileStmt &node)
{
    // 计算条件表达式
    node.cond_->accept(*this);

    while (value_ != 0)
    {                              // 如果条件为真
        node.body_->accept(*this); // 执行循环体
        node.cond_->accept(*this); // 再次计算条件，确保循环继续
    }
}

void SemanticAnalyzer::visit(ReturnStmt &node)
{
    // 如果返回的是一个表达式，计算其值
    if (node.exp_)
    {
        node.exp_->accept(*this);
    }

    // 如果函数没有返回值（如 `void` 类型），value_ 设置为特定值（例如 -1）
    value_ = value_; // 设置返回值
}

void SemanticAnalyzer::visit(IOStmt &node)
{
    if (node.kind == AST::IOStmt::IOKind::Printf)
    {
        // 处理 printf 语句
        // 检查格式字符串和参数是否匹配
        for (auto &arg : node.args_)
        {
            arg->accept(*this); // 计算每个参数的值
        }
    }
    else if (node.kind == AST::IOStmt::IOKind::Getint)
    {
        // 处理 getint 语句
        node.target_->accept(*this); // 计算目标左值
    }
}

void SemanticAnalyzer::visit(LVal &node)
{
    // 在符号表中查找左值
    auto *varSymbol = symbolTable_.lookup(node.name_);
    if (!varSymbol)
    {
        std::cerr << "Error: Left value '" << node.name_ << "' not defined.\n";
        return;
    }

    // 如果是数组，检查下标是否合法
    if (!node.indices_.empty())
    {
        for (auto &index : node.indices_)
        {
            index->accept(*this); // 计算下标
            // 可以检查数组下标是否越界
        }
    }
}

void SemanticAnalyzer::visit(PrimaryExp &node)
{

    // 检查主表达式的类型
    if (std::holds_alternative<std::unique_ptr<AST::LVal>>(node.operand_))
    {
        // 处理左值
        auto lVal = std::move(std::get<std::unique_ptr<AST::LVal>>(node.operand_));
        lVal->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<AST::Number>>(node.operand_))
    {
        // 处理数字常量
        auto number = std::move(std::get<std::unique_ptr<AST::Number>>(node.operand_));
        number->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.operand_))
    {
        // 处理括号内的表达式
        auto exp = std::move(std::get<std::unique_ptr<AST::Exp>>(node.operand_));
        exp->accept(*this);
    }
}

void SemanticAnalyzer::visit(UnaryExp &node)
{
    node.operand_->accept(*this);
    // 计算操作数的值
    int operand = value_;
    switch (node.op)
    {
    case AST::UnaryExp::Op::Plus:
        value_ = operand; // 正号
        break;
    case AST::UnaryExp::Op::Minus:
        value_ = -operand; // 负号
        break;
    case AST::UnaryExp::Op::Not:
        value_ = !operand; // 逻辑非
        break;
    default:
        break;
    }
}

void SemanticAnalyzer::visit(AddExp &node)
{
    std::move(std::get<std::unique_ptr<Exp>>(node.elements_[0]))->accept(*this); // 计算第一个操作数的值
    int result = value_;                                                         // 获取左操作数的值
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        std::move(std::get<std::unique_ptr<Exp>>(node.elements_[i + 1]))->accept(*this); // 计算第二个操作数的值
        int right = value_;                                                              // 右操作数的值
        if (op == TokenType::OPERATOR_PLUS)
        {
            result += right;
        }
        else if (op == TokenType::OPERATOR_MINUS)
        {
            result -= right;
        }
    }
    value_ = result; // 最终结果
}

void SemanticAnalyzer::visit(MulExp &node)
{
    std::move(std::get<std::unique_ptr<Exp>>(node.elements_[0]))->accept(*this); // 计算第一个操作数的值
    int result = value_;                                                         // 获取左操作数的值
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        std::move(std::get<std::unique_ptr<Exp>>(node.elements_[i + 1]))->accept(*this); // 计算第一个操作数的值
        int right = value_;                                                              // 右操作数的值
        if (op == TokenType::OPERATOR_MULTIPLY)
        {
            result *= right;
        }
        else if (op == TokenType::OPERATOR_DIVIDE)
        {
            result /= right;
        }
        else if (op == TokenType::OPERATOR_MODULO)
        {
            result %= right;
        }
    }
    value_ = result; // 最终结果
}

void SemanticAnalyzer::visit(LOrExp &node)
{
    // 获取第一个操作数的值
    int left = value_;
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        int right = value_; // 计算右操作数的值

        // 如果是逻辑或运算符
        if (op == TokenType::OPERATOR_LOGICAL_OR)
        {
            left = left || right; // 左右操作数进行逻辑或运算
        }
    }
    value_ = left; // 最终结果
}

void SemanticAnalyzer::visit(LAndExp &node)
{
    // 获取第一个操作数的值
    int left = value_;
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        int right = value_; // 计算右操作数的值

        // 如果是逻辑与运算符
        if (op == TokenType::OPERATOR_LOGICAL_AND)
        {
            left = left && right; // 左右操作数进行逻辑与运算
        }
    }
    value_ = left; // 最终结果
}

void SemanticAnalyzer::visit(EqExp &node)
{
    // 获取第一个操作数的值
    int left = value_;
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        int right = value_; // 计算右操作数的值

        // 如果是相等操作符
        if (op == TokenType::OPERATOR_EQUAL)
        {
            left = (left == right); // 左右操作数进行相等判断
        }
        // 如果是不等操作符
        else if (op == TokenType::OPERATOR_NOT_EQUAL)
        {
            left = (left != right); // 左右操作数进行不等判断
        }
    }
    value_ = left; // 最终结果
}

void SemanticAnalyzer::visit(RelExp &node)
{
    // 获取第一个操作数的值
    int left = value_;
    for (size_t i = 1; i < node.elements_.size(); i += 2)
    {
        TokenType op = std::get<TokenType>(node.elements_[i]);
        int right = value_; // 计算右操作数的值

        // 如果是小于操作符
        if (op == TokenType::OPERATOR_LESS)
        {
            left = (left < right);
        }
        // 如果是大于操作符
        else if (op == TokenType::OPERATOR_GREATER)
        {
            left = (left > right);
        }
        // 如果是小于等于操作符
        else if (op == TokenType::OPERATOR_LESS_EQUAL)
        {
            left = (left <= right);
        }
        // 如果是大于等于操作符
        else if (op == TokenType::OPERATOR_GREATER_EQUAL)
        {
            left = (left >= right);
        }
    }
    value_ = left; // 最终结果
}

void SemanticAnalyzer::visit(CallExp &node)
{
    // 查找符号表中是否有定义的函数
    auto *funcSym = symbolTable_.lookup(node.funcName);
    if (!funcSym || funcSym->symbolType_ != SymbolType::FUNCTION)
    {
        std::cerr << "Error: Function '" << node.funcName << "' is not defined.\n";
        return;
    }

    // 验证函数参数个数和类型
    // const auto &func = dynamic_cast<FunctionSymbol *>(funcSym);
    // if (func->paramTypes_.size() != node.args_.size()) {
    //     std::cerr << "Error: Function '" << node.funcName << "' called with incorrect number of arguments.\n";
    //     return;
    // }

    // 检查每个参数类型是否匹配
    // for (size_t i = 0; i < func->paramTypes_.size(); ++i) {
    //     // if (func->paramTypes_[i] != node.args_[i]->getType()) {
    //     //     std::cerr << "Error: Function '" << node.funcName << "' argument type mismatch.\n";
    //     //     return;
    //     // }
    // }

    // 如果所有检查通过，设置该函数调用的返回类型（可选，取决于需求）
    // value_ = func->getReturnType();  // 假设 `FunctionSymbol` 有 `getReturnType()` 方法
}

void SemanticAnalyzer::visit(Number &node)
{
    value_ = node.value_; // 直接存储数字值
}

void SemanticAnalyzer::visit(FuncParam &node)
{
    // 假设你有对函数参数类型的验证需求，或者你需要检查参数在符号表中的记录：
    // 示例：检查是否有重复参数名
    if (symbolTable_.lookup(node.name_))
    {
        std::cerr << "Error: Parameter '" << node.name_ << "' already defined.\n";
    }

    // 将参数类型和名称加入符号表
    auto symbol = std::make_unique<VariableSymbol>();
    symbol->name_ = node.name_;
    symbol->symbolType_ = SymbolType::VARIABLE;
    symbol->dataType_ = node.bType_->typeName_ == "int" ? TokenType::KEYWORD_INT : TokenType::KEYWORD_VOID; // 假设 `bType_` 存储类型
    symbolTable_.addSymbol(std::move(symbol));
}

void SemanticAnalyzer::visit(FuncDef &node)
{
    // 检查函数是否已经定义
    if (symbolTable_.lookup(node.name_))
    {
        std::cerr << "Error: Function '" << node.name_ << "' already defined.\n";
        return;
    }

    // 将函数名、返回类型添加到符号表
    auto funcSymbol = std::make_unique<FunctionSymbol>();
    funcSymbol->name_ = node.name_;
    funcSymbol->symbolType_ = SymbolType::FUNCTION;
    // funcSymbol->returnType_ = node.returnType_->typeName_=="int"?TokenType::KEYWORD_INT:TokenType::KEYWORD_VOID;;  // 假设 `returnType_` 存储类型
    symbolTable_.addSymbol(std::move(funcSymbol));

    // 处理函数参数
    for (auto &param : node.params_)
    {
        param->accept(*this); // 调用 visit 方法来处理每个参数
    }

    // 处理函数体
    node.body_->accept(*this);
}

void SemanticAnalyzer::visit(MainFuncDef &node)
{
    // 检查主函数是否已定义
    if (!symbolTable_.lookup("main"))
    {
        std::cerr << "Error: Main function already defined.\n";
        return;
    }

    // 处理主函数体
    node.body_->accept(*this);
}

void SemanticAnalyzer::visit(BType &node)
{
    // 检查并记录类型（如 int、void 等）
    if (node.typeName_ == "int")
    {
        value_ = 0; // 假设 int 默认值为 0
    }
    else if (node.typeName_ == "void")
    {
        value_ = -1; // 假设 void 类型没有返回值
    }
    else
    {
        std::cerr << "Error: Unknown type '" << node.typeName_ << "'\n";
    }
}

// 处理常量初始化值（可能是单个常量或数组）
void SemanticAnalyzer::visit(InitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
    {
        // 如果是单个常量表达式
        auto exp = std::move(std::get<std::unique_ptr<AST::Exp>>(node.value_));
        exp->accept(*this);
    }
    else
    {
        // 如果是数组初始化（递归处理数组元素）
        int total = value_;
        for (auto &elem : std::get<std::vector<std::unique_ptr<AST::InitVal>>>(node.value_))
        {
            elem->accept(*this);
            total += value_;
        }
        value_ = total; // 计算数组的总和或其他值
    }
}

void SemanticAnalyzer::visit(ConstInitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
    {
        auto exp = std::move(std::get<std::unique_ptr<AST::Exp>>(node.value_));
        exp->accept(*this);
    }
    else
    {
        for (auto &elem : std::get<std::vector<std::unique_ptr<AST::ConstInitVal>>>(node.value_))
        {
            elem->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(BlockItem &node)
{
    node.item_->accept(*this);
}