#include "SemanticAnalyzer.h"
#include <iostream>

// 遍历编译单元
void SemanticAnalyzer::visit(CompUnit &node)
{
    // 处理全局声明
    for (auto &decl : node.decls_)
    {
        decl->accept(*this);
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
}

void SemanticAnalyzer::visit(ConstDef &)
{
}

void SemanticAnalyzer::visit(VarDef &node)
{
}

// --- 声明处理 ---
// 处理常量声明（例如 const int a=1, b[2]={1,2};）
void SemanticAnalyzer::visit(ConstDecl &node)
{
    EvalConstant evaluator; // 用于常量求值

    for (auto &constDef : node.constDefs_)
    {
        if (symbolTable.lookupInCurrentScope(constDef->name_) != nullptr)
        {
            errorManager.addError(ErrorLevel::ERROR, 'b', 0,
                                  "重复定义常量：" + constDef->name_,
                                  ErrorType::SemanticError);
        }
        else
        {
            // 如果存在数组维度，说明该常量是数组
            if (!constDef->dimensions_.empty())
            {
                auto arraySymbol = std::make_unique<ArraySymbol>();
                arraySymbol->name_ = constDef->name_;
                arraySymbol->symbolType_ = CONSTANT;
                arraySymbol->dataType_ = TokenType::KEYWORD_INT;
                arraySymbol->lineDefined_ = 0;
                arraySymbol->isConst_ = true;

                // 计算数组维度，维度必须是常量表达式
                for (auto &dimExp : constDef->dimensions_)
                {
                    int dimSize = evaluator.Eval(dimExp.get());
                    if (dimSize <= 0)
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'c', 0,
                                              "常量数组维度必须大于0：" + constDef->name_,
                                              ErrorType::SemanticError);
                    }
                    arraySymbol->dimensions_.push_back(dimSize);
                }

                // 数组初始化检查
                if (constDef->hasInit)
                {
                    auto &initValues = constDef->initVal_;
                    // 判断是否符合数组初始化的要求
                    if (std::holds_alternative<std::vector<std::unique_ptr<ConstInitVal>>>(initValues->value_))
                    {
                        auto &initList = std::get<std::vector<std::unique_ptr<ConstInitVal>>>(initValues->value_);
                        int totalElements = arraySymbol->dimensions_.size();
                        // for (size_t i = 0; i < arraySymbol->dimensions_.size(); ++i)
                        // {
                        //     totalElements *= arraySymbol->dimensions_[i];
                        // }

                        if (initList.size() != totalElements)
                        {
                            errorManager.addError(ErrorLevel::ERROR, 'd', 0,
                                                  "常量数组初始化元素数量与维度不匹配：" + constDef->name_,
                                                  ErrorType::SemanticError);
                        }

                        // 对数组初始化元素进行求值
                        std::vector<int> evaluatedValues;
                        if (!checkAndEvaluateConstInitList(initList, arraySymbol->dimensions_, 0, evaluator,
                                                           evaluatedValues, errorManager, constDef->name_))
                        {
                            // 如果有错误，返回
                            return;
                        }

                        // 初始化符号表中的常量数组
                        arraySymbol->initValues_ = std::move(evaluatedValues);
                    }
                    else
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'e', 0,
                                              "非数组初始化不能应用于常量数组：" + constDef->name_,
                                              ErrorType::SemanticError);
                    }
                }

                // 添加常量数组符号到符号表
                if (!symbolTable.addSymbol(std::move(arraySymbol)))
                {
                    errorManager.addError(ErrorLevel::ERROR, 'n', 0,
                                          "添加常量数组符号失败：" + constDef->name_,
                                          ErrorType::SemanticError);
                }
            }
            else // 处理普通常量
            {
                auto symbol = std::make_unique<VariableSymbol>();
                symbol->name_ = constDef->name_;
                symbol->symbolType_ = CONSTANT;
                symbol->dataType_ = TokenType::KEYWORD_INT;
                symbol->lineDefined_ = 0;
                symbol->isConst_ = true;

                // 常量必须有初始化值，且必须能在编译时求值
                try
                {
                    symbol->initValue_ = evaluator.Eval(constDef->initVal_.get());
                }
                catch (std::runtime_error &e)
                {
                    errorManager.addError(ErrorLevel::ERROR, 'f', 0,
                                          "常量求值失败：" + constDef->name_ + "，" + e.what(),
                                          ErrorType::SemanticError);
                }
                if (!symbolTable.addSymbol(std::move(symbol)))
                {
                    errorManager.addError(ErrorLevel::ERROR, 'n', 0,
                                          "添加常量符号失败：" + constDef->name_,
                                          ErrorType::SemanticError);
                }
            }
        }
    }
}

// 处理变量声明（例如 int a, b[2];）
void SemanticAnalyzer::visit(VarDecl &node)
{
    EvalConstant evaluator; // 用于常量求值

    for (auto &varDef : node.varDefs_)
    {
        if (symbolTable.lookupInCurrentScope(varDef->name_) != nullptr)
        {
            errorManager.addError(ErrorLevel::ERROR, 'b', 0,
                                  "重复定义变量：" + varDef->name_,
                                  ErrorType::SemanticError);
        }
        else
        {
            auto symbol = std::make_unique<VariableSymbol>();
            symbol->name_ = varDef->name_;
            symbol->symbolType_ = VARIABLE;
            symbol->dataType_ = TokenType::KEYWORD_INT;
            symbol->lineDefined_ = 0;
            symbol->isConst_ = false;

            // 处理数组声明
            if (!varDef->constExps_.empty()) // 判断是否是数组
            {
                // 数组符号
                auto arraySymbol = std::make_unique<ArraySymbol>();
                arraySymbol->name_ = varDef->name_;
                arraySymbol->symbolType_ = ARRAY;
                arraySymbol->dataType_ = TokenType::KEYWORD_INT;
                arraySymbol->lineDefined_ = 0;
                arraySymbol->isConst_ = false;

                // 计算数组维度，维度必须是常量表达式
                for (auto &dimExp : varDef->constExps_)
                {
                    int dimSize = evaluator.Eval(dimExp.get());
                    if (dimSize <= 0)
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'c', 0,
                                              "数组维度必须大于0：" + varDef->name_,
                                              ErrorType::SemanticError);
                    }
                    arraySymbol->dimensions_.push_back(dimSize);
                }

                // 数组初始化检查
                if (varDef->hasInit)
                {
                    auto &initValues = varDef->initVal_;
                    // 判断是否符合数组初始化的要求
                    if (std::holds_alternative<std::vector<std::unique_ptr<InitVal>>>(initValues->value_))
                    {
                        auto &initList = std::get<std::vector<std::unique_ptr<InitVal>>>(initValues->value_);
                        int totalElements = arraySymbol->dimensions_.size();
                        // for (size_t i = 0; i < arraySymbol->dimensions_.size(); ++i)
                        // {
                        //     totalElements *= arraySymbol->dimensions_[i];
                        // }

                        if (initList.size() != totalElements)
                        {
                            errorManager.addError(ErrorLevel::ERROR, 'd', 0,
                                                  "数组初始化元素数量与维度不匹配：" + varDef->name_,
                                                  ErrorType::SemanticError);
                        }

                        // 对数组初始化元素进行求值
                        std::vector<int> evaluatedValues;
                        if (!checkAndEvaluateInitList(initList, arraySymbol->dimensions_, 0, evaluator,
                                                      evaluatedValues, errorManager, varDef->name_))
                        {
                            // 如果有错误，返回
                            return;
                        }

                        // 初始化符号表中的数组
                        arraySymbol->initValues_ = std::move(evaluatedValues);
                    }
                    else
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'e', 0,
                                              "非数组初始化不能应用于数组：" + varDef->name_,
                                              ErrorType::SemanticError);
                    }
                }

                // 添加数组符号到符号表
                if (!symbolTable.addSymbol(std::move(arraySymbol)))
                {
                    errorManager.addError(ErrorLevel::ERROR, 'n', 0,
                                          "添加数组符号失败：" + varDef->name_,
                                          ErrorType::SemanticError);
                }
            }
            else // 处理普通变量
            {
                if (varDef->hasInit)
                {
                    symbol->initValue_ = evaluator.Eval(varDef->initVal_.get());
                }
                if (!symbolTable.addSymbol(std::move(symbol)))
                {
                    errorManager.addError(ErrorLevel::ERROR, 'n', 0,
                                          "添加变量符号失败：" + varDef->name_,
                                          ErrorType::SemanticError);
                }
            }
        }
    }
}

// 处理基本类型（目前只有 int，不必做过多检查）
void SemanticAnalyzer::visit(BType &node)
{
    // 可根据类型名扩展
}

// 处理变量初始化值（这里只作为占位，详细语义检查在各自节点中递归完成）
void SemanticAnalyzer::visit(InitVal &node)
{
    // 如果是表达式，直接检查；如果是数组初始化列表，递归处理每个子 InitVal
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.value_))
    {
        std::get<std::unique_ptr<Exp>>(node.value_)->accept(*this);
    }
    else
    {
        auto &list = std::get<std::vector<std::unique_ptr<InitVal>>>(node.value_);
        for (auto &elem : list)
        {
            elem->accept(*this);
        }
    }
}

// 处理常量初始化值（同 InitVal 处理方式类似，但要求其表达式能在编译期求值）
void SemanticAnalyzer::visit(ConstInitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.value_))
    {
        std::get<std::unique_ptr<Exp>>(node.value_)->accept(*this);
    }
    else
    {
        auto &list = std::get<std::vector<std::unique_ptr<ConstInitVal>>>(node.value_);
        for (auto &elem : list)
        {
            elem->accept(*this);
        }
    }
}

// --- 函数与形参 ---
// 处理函数定义
void SemanticAnalyzer::visit(FuncDef &node)
{
    // 在当前作用域中检查函数是否已定义
    if (symbolTable.lookupInCurrentScope(node.name_) != nullptr)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "重复定义函数：" + node.name_, ErrorType::SemanticError);
    }
    else
    {
        auto symbol = std::make_unique<FunctionSymbol>();
        symbol->name_ = node.name_;
        symbol->symbolType_ = FUNCTION;
        // 根据函数返回类型来设置 dataType_，此处简单处理为 int 或 void
        symbol->dataType_ = (node.returnType_->typeName_ == "int") ? TokenType::KEYWORD_INT : TokenType::KEYWORD_VOID;
        symbol->lineDefined_ = 0;
        // 收集参数类型（这里只以 int 为例）
        for (auto &param : node.params_)
        {
            param->accept(*this); // 处理形参时也可以加入符号表
            symbol->paramTypes_.push_back(TokenType::KEYWORD_INT);
        }
        if (!symbolTable.addSymbol(std::move(symbol)))
        {
            errorManager.addError(ErrorLevel::ERROR, 'n', 0, "添加函数符号失败：" + node.name_, ErrorType::SemanticError);
        }
    }
    // 进入函数新作用域，添加形参符号
    symbolTable.enterScope();
    for (auto &param : node.params_)
    {
        // 这里可以将每个形参添加到符号表中，类似 VarDecl 的处理
        // …（略）
    }
    // 处理函数体
    node.body_->accept(*this);
    symbolTable.exitScope();
}

// 处理主函数（与普通函数类似）
void SemanticAnalyzer::visit(MainFuncDef &node)
{
    // symbolTable.enterScope();
    node.body_->accept(*this);
    // symbolTable.exitScope();
}

// 处理函数形参（示例中暂不详细展开，可在此处添加数组参数等检查）
void SemanticAnalyzer::visit(FuncParam &node)
{
    // 例如，检查形参名是否重复（在函数内部作用域中由 FuncDef 添加）
    if (symbolTable.lookupInCurrentScope(node.name_) != nullptr)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "重复定义函数参数：" + node.name_, ErrorType::SemanticError);
    }
    else
    {
        auto symbol = std::make_unique<VariableSymbol>();
        symbol->name_ = node.name_;
        symbol->symbolType_ = PARAM;
        symbol->dataType_ = TokenType::KEYWORD_INT;
        symbol->lineDefined_ = 0;
        symbol->isConst_ = false;
        if (!symbolTable.addSymbol(std::move(symbol)))
        {
            errorManager.addError(ErrorLevel::ERROR, 'n', 0, "添加函数参数符号失败：" + node.name_, ErrorType::SemanticError);
        }
    }
}

// --- 语句 ---
// 表达式语句
void SemanticAnalyzer::visit(ExpStmt &node)
{
    if (node.exp_)
    {
        node.exp_->accept(*this);
    }
}

// 语句块
void SemanticAnalyzer::visit(Block &node)
{
    symbolTable.enterScope();
    for (auto &item : node.items_)
    {
        item->accept(*this);
    }
    symbolTable.exitScope();
}

// 块项（声明或语句）
void SemanticAnalyzer::visit(BlockItem &node)
{
    node.item_->accept(*this);
}

// 赋值语句
void SemanticAnalyzer::visit(AssignStmt &node)
{
    // 检查左值是否定义以及是否为可修改的变量
    node.lval_->accept(*this);
    Symbol *symbol = symbolTable.lookup(node.lval_->name_);
    if (symbol == nullptr)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "赋值语句中未定义的变量：" + node.lval_->name_, ErrorType::SemanticError);
    }
    else if (symbol->symbolType_ == CONSTANT)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "不能修改常量：" + node.lval_->name_, ErrorType::SemanticError);
    }
    // 检查右侧表达式
    node.exp_->accept(*this);
}

// If 语句
void SemanticAnalyzer::visit(IfStmt &node)
{
    node.cond_->accept(*this);
    node.thenBranch_->accept(*this);
    if (node.elseBranch_)
    {
        node.elseBranch_->accept(*this);
    }
}

// while 语句
void SemanticAnalyzer::visit(WhileStmt &node)
{
    node.cond_->accept(*this);
    node.body_->accept(*this);
}

// return 语句
void SemanticAnalyzer::visit(ReturnStmt &node)
{
    if (node.exp_)
    {
        node.exp_->accept(*this);
    }
    // 这里还可以检查返回值类型是否与函数声明匹配
}

// 输入输出语句
void SemanticAnalyzer::visit(IOStmt &node)
{
    if (node.kind == IOStmt::IOKind::Getint)
    {
        node.target_->accept(*this);
        Symbol *symbol = symbolTable.lookup(node.target_->name_);
        if (symbol == nullptr)
        {
            errorManager.addError(ErrorLevel::ERROR, 'b', 0, "getint中未定义的变量：" + node.target_->name_, ErrorType::SemanticError);
        }
        if (symbol && symbol->symbolType_ == CONSTANT)
        {
            errorManager.addError(ErrorLevel::ERROR, 'b', 0, "不能使用getint修改常量：" + node.target_->name_, ErrorType::SemanticError);
        }
    }
    else
    { // Printf
        for (auto &arg : node.args_)
        {
            arg->accept(*this);
        }
        // 还可以检查格式字符串与参数个数是否匹配
    }
}

// --- 表达式 ---
// 数字字面量
void SemanticAnalyzer::visit(Number &node)
{
    // 数值直接通过，不用做检查
}

// 左值
void SemanticAnalyzer::visit(LVal &node)
{
    Symbol *symbol = symbolTable.lookup(node.name_);
    if (symbol == nullptr)
    {
        errorManager.addError(ErrorLevel::ERROR, 'g', 0,
                              "使用了未定义的变量：" + node.name_,
                              ErrorType::SemanticError);
    }
    else
    {
        // 如果符号为数组，则检查下标数量
        if (symbol->symbolType_ == ARRAY)
        {
            // 假设 ArraySymbol 在符号表中，需向下转型
            auto arraySymbol = dynamic_cast<ArraySymbol *>(symbol);
            if (node.indices_.size() != arraySymbol->dimensions_.size())
            {
                errorManager.addError(ErrorLevel::ERROR, 'h', 0,
                                      "数组下标个数与声明不匹配：" + node.name_,
                                      ErrorType::SemanticError);
            }
            else
            {
                // 尝试对每个下标进行常量求值检查，并判断是否越界
                EvalConstant evaluator;
                for (size_t i = 0; i < node.indices_.size(); ++i)
                {
                    int indexValue = 0;
                    try
                    {
                        indexValue = evaluator.Eval(node.indices_[i].get());
                    }
                    catch (std::runtime_error &e)
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'i', 0,
                                              "数组下标不能求值为常量：" + node.name_ + ", " + e.what(),
                                              ErrorType::SemanticError);
                    }
                    if (indexValue < 0 || indexValue >= arraySymbol->dimensions_[i])
                    {
                        errorManager.addError(ErrorLevel::ERROR, 'j', 0,
                                              "数组下标越界：" + node.name_,
                                              ErrorType::SemanticError);
                    }
                }
            }
        }
        else
        {
            // 对于标量变量，检查是否为常量
            if (symbol->symbolType_ == CONSTANT)
            {
                return;
                errorManager.addError(ErrorLevel::ERROR, 'k', 0,
                                      "不能修改常量：" + node.name_,
                                      ErrorType::SemanticError);
            }
        }
        // 对于标量变量的访问，仅需检查变量是否存在即可
        for (auto &index : node.indices_)
        {
            index->accept(*this);
        }
    }
}

// 基本表达式
void SemanticAnalyzer::visit(PrimaryExp &node)
{
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.operand_))
    {
        auto &child = std::get<std::unique_ptr<Exp>>(node.operand_);
        if (child)
            child->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<LVal>>(node.operand_))
    {
        auto &child = std::get<std::unique_ptr<LVal>>(node.operand_);
        if (child)
            child->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<Number>>(node.operand_))
    {
        auto &child = std::get<std::unique_ptr<Number>>(node.operand_);
        if (child)
            child->accept(*this);
    }
}

// 一元表达式
void SemanticAnalyzer::visit(UnaryExp &node)
{
    // 1. 根据 variant 的实际类型，分发到对应的子节点
    node.operand_->accept(*this);
}

// 加法表达式（同理适用于其他二元表达式）
void SemanticAnalyzer::visit(AddExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
        // 运算符的部分一般只作为标记，无需遍历
    }
}

void SemanticAnalyzer::visit(MulExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(LOrExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(LAndExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(EqExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(RelExp &node)
{
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            if (child)
                child->accept(*this);
        }
    }
}

// 函数调用表达式
void SemanticAnalyzer::visit(CallExp &node)
{
    Symbol *symbol = symbolTable.lookup(node.funcName);
    if (symbol == nullptr)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "调用未定义的函数：" + node.funcName, ErrorType::SemanticError);
    }
    else if (symbol->symbolType_ != FUNCTION)
    {
        errorManager.addError(ErrorLevel::ERROR, 'b', 0, "标识符不是函数：" + node.funcName, ErrorType::SemanticError);
    }
    for (auto &arg : node.args_)
    {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(FuncType &)
{
}

// --- 辅助函数 ---
// 这里给出简单的常量表达式求值示例（实际情况可能复杂得多）
int SemanticAnalyzer::evaluateConstExp(ConstInitVal *initVal)
{
    // 如果是单个表达式，则尝试递归求值
    if (std::holds_alternative<std::unique_ptr<Exp>>(initVal->value_))
    {
        // 例如：如果该表达式仅为数字字面量，则返回其值
        auto &expPtr = std::get<std::unique_ptr<Exp>>(initVal->value_);
        if (expPtr)
        {
            // 此处仅作演示，假定 Exp 最终能返回 Number 的值
            // 你可以实现一个专门的常量求值器
            return evaluateExp(expPtr.get());
        }
    }
    // 数组初始化或其他情况暂返回0
    return 0;
}

// 简单表达式求值（仅作示例，实际需完整支持运算）
int SemanticAnalyzer::evaluateExp(Node *node)
{
    return evalConstant.Eval(node);
}

bool SemanticAnalyzer::checkAndEvaluateInitList(const std::vector<std::unique_ptr<InitVal>> &initList,
                                                const std::vector<int> &dimensions,
                                                size_t currentDim,
                                                EvalConstant &evaluator,
                                                std::vector<int> &evaluatedValues,
                                                ErrorManager &errorManager,
                                                const std::string &varName)
{
    // 当前层次应有 dimensions[currentDim] 个元素
    if (initList.size() != dimensions[currentDim])
    {
        errorManager.addError(ErrorLevel::ERROR, 'k', 0,
                              "数组初始化列表第" + std::to_string(currentDim + 1) +
                                  "维元素数量与声明不匹配：" + varName,
                              ErrorType::SemanticError);
        return false;
    }
    // 如果是最后一维，则列表中每个元素都应为单个表达式，可以求值
    if (currentDim == dimensions.size() - 1)
    {
        for (auto &elem : initList)
        {
            try
            {
                int value = evaluator.Eval(elem.get());
                evaluatedValues.push_back(value);
            }
            catch (std::runtime_error &e)
            {
                errorManager.addError(ErrorLevel::ERROR, 'l', 0,
                                      "数组初始化元素求值失败：" + varName + ", " + e.what(),
                                      ErrorType::SemanticError);
                return false;
            }
        }
    }
    else
    {
        // 对于非最后一维，期望每个元素为嵌套的初始化列表
        for (auto &elem : initList)
        {
            // 判断是否为列表
            if (std::holds_alternative<std::vector<std::unique_ptr<InitVal>>>(elem->value_))
            {
                auto &subList = std::get<std::vector<std::unique_ptr<InitVal>>>(elem->value_);
                if (!checkAndEvaluateInitList(subList, dimensions, currentDim + 1,
                                              evaluator, evaluatedValues, errorManager, varName))
                {
                    return false;
                }
            }
            else
            {
                errorManager.addError(ErrorLevel::ERROR, 'm', 0,
                                      "数组初始化列表嵌套错误：" + varName,
                                      ErrorType::SemanticError);
                return false;
            }
        }
    }
    return true;
}

bool SemanticAnalyzer::checkAndEvaluateConstInitList(const std::vector<std::unique_ptr<ConstInitVal>> &initList,
                                                     const std::vector<int> &dimensions,
                                                     size_t currentDim,
                                                     EvalConstant &evaluator,
                                                     std::vector<int> &evaluatedValues,
                                                     ErrorManager &errorManager,
                                                     const std::string &varName)
{
    // 当前层次应有 dimensions[currentDim] 个元素
    if (initList.size() != dimensions[currentDim])
    {
        errorManager.addError(ErrorLevel::ERROR, 'k', 0,
                              "常量数组初始化列表第" + std::to_string(currentDim + 1) +
                                  "维元素数量与声明不匹配：" + varName,
                              ErrorType::SemanticError);
        return false;
    }
    // 如果是最后一维，则列表中每个元素都应为单个表达式，可以求值
    if (currentDim == dimensions.size() - 1)
    {
        for (auto &elem : initList)
        {
            try
            {
                int value = evaluator.Eval(elem.get());
                evaluatedValues.push_back(value);
            }
            catch (std::runtime_error &e)
            {
                errorManager.addError(ErrorLevel::ERROR, 'l', 0,
                                      "常量数组初始化元素求值失败：" + varName + ", " + e.what(),
                                      ErrorType::SemanticError);
                return false;
            }
        }
    }
    else
    {
        // 对于非最后一维，期望每个元素为嵌套的初始化列表
        for (auto &elem : initList)
        {
            // 判断是否为列表
            if (std::holds_alternative<std::vector<std::unique_ptr<ConstInitVal>>>(elem->value_))
            {
                auto &subList = std::get<std::vector<std::unique_ptr<ConstInitVal>>>(elem->value_);
                if (!checkAndEvaluateConstInitList(subList, dimensions, currentDim + 1,
                                                   evaluator, evaluatedValues, errorManager, varName))
                {
                    return false;
                }
            }
            else
            {
                errorManager.addError(ErrorLevel::ERROR, 'm', 0,
                                      "常量数组初始化列表嵌套错误：" + varName,
                                      ErrorType::SemanticError);
                return false;
            }
        }
    }
    return true;
}
