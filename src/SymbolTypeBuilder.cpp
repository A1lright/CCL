#include "SymbolTypeBuilder.h"

void SymbolManager::visit(CompUnit &node)
{
    // 遍历所有全局声明（ConstDecl|VarDecl）
    for (auto &decl : node.decls_)
    {
        decl->accept(*this); // 分发到ConstDecl|VarDecl
    }

    // 处理函数定义
    for (auto &funcDef : node.funcDefs_)
    {
        funcDef->accept(*this);
    }

    // 处理主函数
    if (node.mainfuncDef_)
    {
        node.mainfuncDef_->accept(*this);
    }
    // 全局作用域无需退出，持续到程序结束
}

void SymbolManager::visit(ConstDef &node)
{
    auto sym = std::make_unique<VariableSymbol>();
    sym->name_ = node.name_;
    sym->symbolType_ = CONSTANT;
    sym->dataType_ = (current_decl_type_ == "int") ? TokenType::KEYWORD_INT : TokenType::KEYWORD_VOID;
    sym->isConst_ = true;
    sym->isArray_ = !node.dimensions_.empty();

    // 处理数组维度
    for (auto &dim : node.dimensions_)
    {
        //auto val = eval_constant_expr(dim.get());
        // if (!val)
        // {
        //     errors_.push_back("常量定义 '" + node.name_ + "' 维度必须是常量表达式");
        //     return;
        // }
        // sym->dimensions_.push_back(val->intValue);
    }

    // 处理初始化值
    if (!process_const_init_val(*sym, node.initVal_.get()))
    {
        errors_.push_back("常量 '" + node.name_ + "' 初始化值不合法");
        return;
    }

    if (!symtab_.addSymbol(std::move(sym)))
    {
        errors_.push_back("重复定义的常量 '" + node.name_ + "'");
    }
}

void SymbolManager::visit(ConstDecl &node)
{
    current_is_const_ = true;
    node.bType_->accept(*this); // 设置类型
    for (auto &def : node.constDefs_)
    {
        def->accept(*this);
    }
    current_is_const_ = false;
}

void SymbolManager::visit(VarDef &node)
{

    // 创建符号
    auto sym = std::make_unique<VariableSymbol>();
    sym->name_ = node.name_;
    sym->symbolType_ = SymbolType::VARIABLE;
    sym->dataType_ = (current_decl_type_ == "int") ? TokenType::KEYWORD_INT : TokenType::KEYWORD_VOID;
    sym->isArray_ = !node.constExps_.empty();
    sym->lineDefined_ = 0;
    sym->columnDefined_ = 0;

    // 处理数组函数
    for (auto &dim : node.constExps_)
    {
        // auto val = eval_constant_expr(dim.get());
        // if (!val)
        // {
        //     errors_.push_back("变量 '" + node.name_ + "' 维度必须是常量表达式");
        //     return;
        // }
        // sym->dimensions_.push_back(val->intValue);
    }

    // 处理初始化值
    if (node.initVal_)
    {
        // // 初始化值验证逻辑
        // if (!process_var_init_val(*sym, node.initVal_.get()))
        // {
        //     errors_.push_back("变量 '" + node.name_ + "' 初始化值类型不匹配");
        // }
    }

    // 符号表插入
    if (!symtab_.addSymbol(std::move(sym)))
    {
        errors_.emplace_back("重复定义的变量");
    }
}

void SymbolManager::visit(VarDecl &node)
{
    current_is_const_ = false;
    // 获取基本类型
    node.bType_->accept(*this); // 设置current_decl_type

    // 遍历变量定义
    for (auto &var_def : node.varDefs_)
    {
        var_def->accept(*this);
    }

    current_decl_type_.clear();
}

void SymbolManager::visit(Block &node)
{
    //push_scope();
    for (auto &item : node.items_) {
        item->accept(*this);
    }
    //pop_scope();
}

void SymbolManager::visit(AssignStmt &node)
{
    node.lval_->accept(*this);
    node.exp_->accept(*this);

    Symbol *sym = symtab_.lookup(node.lval_->name_);
    if (!sym) {
        errors_.push_back("赋值给未声明的变量 '" + node.lval_->name_ + "'");
        return;
    }

    if (sym->symbolType_ == CONSTANT) {
        errors_.push_back("不能给常量 '" + node.lval_->name_ + "' 赋值");
    }

    // 类型检查：假设表达式类型为int
}

void SymbolManager::visit(IfStmt &node)
{
    node.cond_->accept(*this);
    node.thenBranch_->accept(*this);
    if (node.elseBranch_) {
        node.elseBranch_->accept(*this);
    }
}

void SymbolManager::visit(WhileStmt &node)
{
    contextStack.top().inLoop = true;
    node.cond_->accept(*this);
    node.body_->accept(*this);
    contextStack.top().inLoop = false;
}

void SymbolManager::visit(ReturnStmt &node)
{
    if (contextStack.empty()) {
        errors_.push_back("return语句不在函数内");
        return;
    }

    TokenType expected = contextStack.top().returnType;
    if (node.exp_) {
        node.exp_->accept(*this);
        if (expected == TokenType::KEYWORD_VOID) {
            errors_.push_back("void函数不能返回数值");
        }
    } else if (expected != TokenType::KEYWORD_VOID) {
        errors_.push_back("非void函数必须返回数值");
    }
}

void SymbolManager::visit(IOStmt &node)
{
    // TODO
     // 根据类型检查参数
     if (node.kind == IOStmt::IOKind::Getint) {
        if (!node.target_) {
            errors_.push_back("getint必须指定目标变量");
        }
        node.target_->accept(*this);
    } else {
        for (auto &arg : node.args_) {
            arg->accept(*this);
        }
    }
}

void SymbolManager::visit(LVal &node)
{
    Symbol *sym = symtab_.lookup(node.name_);
    if (!sym) {
        errors_.push_back("未声明的标识符 '" + node.name_ + "'");
        return;
    }

    // if (auto varSym = dynamic_cast<VariableSymbol*>(sym)) {
    //     if (varSym->isArray_ && node.indices_.size() != varSym->dimensions_.size()) {
    //         errors_.push_back("数组 '" + node.name_ + "' 索引数量不匹配");
    //     }
    // }
}

void SymbolManager::visit(PrimaryExp &node)
{
    if (auto exp = std::get_if<std::unique_ptr<Exp>>(&node.operand_)) {
        (*exp)->accept(*this);
    } else if (auto lval = std::get_if<std::unique_ptr<LVal>>(&node.operand_)) {
        (*lval)->accept(*this);
    } else if (auto num = std::get_if<std::unique_ptr<Number>>(&node.operand_)) {
        (*num)->accept(*this);
    }
}

void SymbolManager::visit(UnaryExp &node)
{
    node.operand_->accept(*this);
}

void SymbolManager::visit(AddExp &node)
{
    for (auto &elem : node.elements_) {
        if (auto exp = std::get_if<std::unique_ptr<Exp>>(&elem)) {
            (*exp)->accept(*this);
        }
    }
}

void SymbolManager::visit(MulExp &node)
{
    for (auto &elem : node.elements_) {
        if (auto exp = std::get_if<std::unique_ptr<Exp>>(&elem)) {
            (*exp)->accept(*this);
        }
    }
}

void SymbolManager::visit(LOrExp &node)
{
}

void SymbolManager::visit(LAndExp &node)
{
}

void SymbolManager::visit(EqExp &node)
{
}

void SymbolManager::visit(RelExp &node)
{
}

void SymbolManager::visit(CallExp &node)
{
    Symbol *sym = symtab_.lookup(node.funcName);
    if (!sym || sym->symbolType_ != FUNCTION) {
        errors_.push_back("未定义的函数 '" + node.funcName + "'");
        return;
    }

    auto funcSym = static_cast<FunctionSymbol*>(sym);
    if (funcSym->paramTypes_.size() != node.args_.size()) {
        errors_.push_back("函数 '" + node.funcName + "' 参数数量不匹配");
        return;
    }

    for (auto &arg : node.args_) {
        arg->accept(*this);
        // 类型检查（假设参数都是int）
    }
}

void SymbolManager::visit(Number &node)
{
    // 无需处理，类型为int
}

void SymbolManager::visit(FuncParam &node)
{
// 参数类型已在FuncDef中处理
}

void SymbolManager::visit(FuncDef &node)
{
    // 处理返回类型
    TokenType retType = (node.returnType_->typeName_ == "int") ? TokenType::KEYWORD_INT : TokenType::KEYWORD_VOID;

    auto funcSym = std::make_unique<FunctionSymbol>();
    funcSym->name_ = node.name_;
    funcSym->symbolType_ = FUNCTION;
    funcSym->dataType_ = retType;

    // 收集参数类型
    for (auto &param : node.params_)
    {
        param->accept(*this);
        // 假设参数类型为int，如果是数组则标记
        funcSym->paramTypes_.push_back(TokenType::KEYWORD_INT);
    }

    if (!symtab_.addSymbol(std::move(funcSym)))
    {
        errors_.push_back("重复定义的函数 '" + node.name_ + "'");
    }

    // 进入函数作用域
    push_scope();
    contextStack.push({retType, false});

    // 添加参数到当前作用域
    for (auto &param : node.params_)
    {
        auto paramSym = std::make_unique<VariableSymbol>();
        paramSym->name_ = param->name_;
        paramSym->symbolType_ = PARAM;
        paramSym->dataType_ = TokenType::KEYWORD_INT;
        paramSym->isArray_ = param->isArray_;
        if (!symtab_.addSymbol(std::move(paramSym)))
        {
            errors_.push_back("函数参数 '" + param->name_ + "' 重复定义");
        }
    }

    node.body_->accept(*this); // 处理函数体

    pop_scope();
    contextStack.pop();
}

void SymbolManager::visit(MainFuncDef &node)
{
    auto funcSym = std::make_unique<FunctionSymbol>();
    funcSym->name_ = "main";
    funcSym->symbolType_ = FUNCTION;
    funcSym->dataType_ = TokenType::KEYWORD_INT;

    if (!symtab_.addSymbol(std::move(funcSym))) {
        errors_.push_back("重复定义的主函数");
    }

    push_scope();
    contextStack.push({TokenType::KEYWORD_INT, false});

    node.body_->accept(*this);

    pop_scope();
    contextStack.pop();
}

void SymbolManager::visit(BType &node)
{
    current_decl_type_ = node.typeName_;
}

void SymbolManager::visit(InitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
    {
        auto *exp = std::get_if<std::unique_ptr<AST::Exp>>(&node.value_);

        (*exp)->accept(*this);
    }
    else
    {
        auto *list = std::get_if<std::vector<std::unique_ptr<AST::InitVal>>>(&node.value_);

        for (size_t i = 0; i < (*list).size(); ++i)
        {
            (*list)[i]->accept(*this); // 递归解析子初始化值
        }
    }
}

void SymbolManager::visit(ConstInitVal &node)
{
    // 逻辑与InitVal类似，但只能包含ConstExp
    if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
    {
        auto *exp = std::get_if<std::unique_ptr<AST::Exp>>(&node.value_);

        (*exp)->accept(*this);
    }
    else
    {
        auto *list = std::get_if<std::vector<std::unique_ptr<AST::ConstInitVal>>>(&node.value_);
        for (size_t i = 0; i < (*list).size(); ++i)
        {
            (*list)[i]->accept(*this);
        }
    }
}

void SymbolManager::visit(BlockItem &node)
{
    node.item_->accept(*this);
}