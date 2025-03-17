#include "codeGenerator.h"
#include "codeGenerator.h"

using namespace llvm;

CodeGenerator::CodeGenerator(SymbolTable &symtab)
    : symtab_(symtab), builder_(context_), module_(std::make_unique<Module>("SysY_module", context_))
{
    createGetintFunction(module_.get(), context_);
}

CodeGenerator::~CodeGenerator()
{
}

void CodeGenerator::generateCode(CompUnit &compUnit)
{
    compUnit.accept(*this);
    // 模块验证
    std::string errStr;
    raw_string_ostream errStream(errStr);
}

//===----------------------------------------------------------------------===//
// 声明处理
//===----------------------------------------------------------------------===//

void CodeGenerator::visit(CompUnit &node)
{
    // 处理全局声明
    for (auto &decl : node.decls_)
    {
        decl->accept(*this);
    }

    // 处理函数定义
    for (auto &func : node.funcDefs_)
    {
        func->accept(*this);
    }

    // 处理主函数
    if (node.mainfuncDef_)
    {
        node.mainfuncDef_->accept(*this);
    }
}

void CodeGenerator::visit(VarDecl &node)
{
    node.bType_->accept(*this);
    for (auto &def : node.varDefs_)
    {
        def->accept(*this);
    }
}

void CodeGenerator::visit(ConstDef &node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(context_);
    // 生成初始化值（此处假设 initVal_ 为单个数值表达式）
    node.initVal_->accept(*this);
    llvm::Constant *initConst = dyn_cast<llvm::Constant>(currentValue_);
    if (!initConst)
    {
        initConst = llvm::ConstantInt::get(intType, 0);
    }
    // 创建全局常量变量
    new GlobalVariable(
        *module_,
        intType,
        true, // 常量
        GlobalValue::InternalLinkage,
        initConst,
        node.name_);
}

void CodeGenerator::visit(ConstDecl &node)
{
    // 目前只支持 int 类型
    for (auto &constDef : node.constDefs_)
    {
        constDef->accept(*this);
    }
}

// 变量定义：生成局部变量的 alloca，并处理初始化
void CodeGenerator::visit(VarDef &node)
{
    Type *ty = Type::getInt32Ty(context_);

    // 在函数入口创建 alloca（更高效的方法是使用一个专门的 alloca 插入点）
    IRBuilder<> tmpBuilder(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    llvm::Value *allocaInst = tmpBuilder.CreateAlloca(ty, nullptr, node.name_);
    //// 更新符号表中变量对应的地址信息
    Symbol *sym = symtab_.lookup(node.name_);
    if (!sym)
    {
        // 如果在活动作用域中找不到，则尝试在当前函数的持久化局部符号中查找
        std::string funcName = currentFunction->getName().str();
        sym = symtab_.lookupFunctionSymbol(funcName, node.name_);
    }
    if (!sym)
    {
        reportError("Undefined variable: " + node.name_, 0);
        currentValue_ = nullptr;
        return;
    }

    VariableSymbol *varSym = static_cast<VariableSymbol *>(sym);
    // varSym->initValue_.intValue = 0; // 默认值
    //  假设我们用一个额外字段存储 alloca 指针
    varSym->allocaInst_ = allocaInst;

    // 如果有初始化表达式，则生成初始化代码
    if (node.hasInit && node.initVal_)
    {
        node.initVal_->accept(*this);
        llvm::Value *initVal = currentValue_;
        builder_.CreateStore(initVal, allocaInst);
    }

    // // 处理数组维度
    // for (auto &dim : node.constExps_)
    // {
    //     dim->accept(*this);
    //     // Value *dimVal = getLastValue();
    //     // if (auto *CI = dyn_cast<ConstantInt>(dimVal)) {
    //     //     ty = ArrayType::get(ty, CI->getSExtValue());
    //     // }
    // }
    // // 创建变量
    // if (currentFunction == nullptr)
    // { // 全局变量
    //     GlobalVariable *gvar = new GlobalVariable(
    //         *module_, ty, false, GlobalValue::CommonLinkage,
    //         Constant::getNullValue(ty), node.name_);
    //     if (node.hasInit)
    //     {
    //         node.initVal_->accept(*this);
    //         handleGlobalInit(gvar, *node.initVal_);
    //     }
    // }
    // else
    // { // 局部变量
    //     AllocaInst *alloca = builder_.CreateAlloca(ty, nullptr, node.name_);
    //     if (node.hasInit)
    //     {
    //         Value *initVal = generateInitValue(*node.initVal_);
    //         builder_.CreateStore(initVal, alloca);
    //     }
    // }
}

//===----------------------------------------------------------------------===//
// 函数处理
//===----------------------------------------------------------------------===//

void CodeGenerator::visit(FuncParam &node)
{
    // 无需处理
}

void CodeGenerator::visit(FuncDef &node)
{
    // 生成函数原型和入口基本块
    std::vector<Type *> paramTypes;
    for (auto &param : node.params_)
    {
        if (param->isArray_)
            paramTypes.push_back(PointerType::getUnqual(Type::getInt32Ty(context_)));
        else
            paramTypes.push_back(Type::getInt32Ty(context_));
    }

    FunctionType *funcType = FunctionType::get(
        node.returnType_->typeName_ == "void" ? Type::getVoidTy(context_) : Type::getInt32Ty(context_),
        paramTypes, false);

    Function *func = Function::Create(
        funcType, Function::ExternalLinkage,
        node.name_, module_.get());

    // 设置当前函数上下文
    currentFunction = func;
    BasicBlock *entryBB = BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entryBB);

    // 处理形参
    auto argIter = func->arg_begin();
    for (auto &param : node.params_)
    {
        AllocaInst *alloc = builder_.CreateAlloca(
            argIter->getType(), nullptr, param->name_);
        builder_.CreateStore(argIter, alloc);

        Symbol *sym = symtab_.lookup(param->name_);
        if (sym && sym->symbolType_ == PARAM)
        {
            VariableSymbol *varSym = static_cast<VariableSymbol *>(sym);
            varSym->allocaInst_ = alloc;
        }

        ++argIter;
    }

    // 生成函数体
    node.body_->accept(*this);

    // 处理无return语句的情况
    if (!verifyTerminator())
    {
        if (node.returnType_->typeName_ == "void")
        {
            builder_.CreateRetVoid();
        }
        else
        {
            // reportError("Missing return statement in non-void function", node.body_->line_);
        }
    }

    currentFunction = nullptr;
}

void CodeGenerator::visit(MainFuncDef &node)
{
    FunctionType *mainFuncType = FunctionType::get(Type::getInt32Ty(context_), false);
    Function *mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage, "main", module_.get());
    currentFunction = mainFunc;

    BasicBlock *entryBB = BasicBlock::Create(context_, "entry", currentFunction);
    builder_.SetInsertPoint(entryBB);

    // 生成函数体代码
    node.body_->accept(*this);

    if (!verifyTerminator())
    {
        builder_.CreateRet(ConstantInt::get(Type::getInt32Ty(context_), 0));
    }

    currentFunction = nullptr;
}

void CodeGenerator::visit(BType &node)
{
}

void CodeGenerator::visit(InitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.value_))
    {
        std::get<std::unique_ptr<Exp>>(node.value_)->accept(*this);
    }
    else
    {
        if (!std::get<std::vector<std::unique_ptr<InitVal>>>(node.value_).empty())
        {
            std::get<std::vector<std::unique_ptr<InitVal>>>(node.value_)[0]->accept(*this);
        }
    }
}

void CodeGenerator::visit(ConstInitVal &node)
{
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.value_))
    {
        std::get<std::unique_ptr<Exp>>(node.value_)->accept(*this);
    }
    else
    {
        if (!std::get<std::vector<std::unique_ptr<ConstInitVal>>>(node.value_).empty())
        {
            std::get<std::vector<std::unique_ptr<ConstInitVal>>>(node.value_)[0]->accept(*this);
        }
    }
}

void CodeGenerator::visit(BlockItem &node)
{
    node.item_->accept(*this);
}

//===----------------------------------------------------------------------===//
// 控制流语句
//===----------------------------------------------------------------------===//

void CodeGenerator::visit(Block &node)
{
    for (auto &item : node.items_)
    {
        item->accept(*this);
    }
}

void CodeGenerator::visit(AssignStmt &node)
{
    // 生成右侧表达式的值
    node.exp_->accept(*this);
    llvm::Value *rhs = currentValue_;
    // 生成左值（得到变量地址）
    node.lval_->accept(*this);
    llvm::Value *lvalAddr = currentValue_;
    if (lvalAddr)
    {
        builder_.CreateStore(rhs, lvalAddr);
    }
    else
    {
        reportError("Invalid assignment target", 0);
    }
}

void CodeGenerator::visit(IfStmt &node)
{
    node.cond_->accept(*this);
    llvm::Value *condValue = currentValue_;
    // 转换成 bool 型（不等于0即为 true）
    condValue = builder_.CreateICmpNE(condValue,
                                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                      "ifcond");

    llvm::Function *function = builder_.GetInsertBlock()->getParent();
    BasicBlock *thenBB = BasicBlock::Create(context_, "then", function);
    // BasicBlock *elseBB = BasicBlock::Create(context_, "else");
    BasicBlock *elseBB = BasicBlock::Create(context_, "else", function);
    // BasicBlock *mergeBB = BasicBlock::Create(context_, "ifcont");
    BasicBlock *mergeBB = BasicBlock::Create(context_, "ifcont", function);
    builder_.CreateCondBr(condValue, thenBB, elseBB);

    // then 分支
    builder_.SetInsertPoint(thenBB);
    node.thenBranch_->accept(*this);
    if (!verifyTerminator())
        builder_.CreateBr(mergeBB);
    thenBB = builder_.GetInsertBlock();

    // else 分支

    builder_.SetInsertPoint(elseBB);
    if (node.elseBranch_)
        node.elseBranch_->accept(*this);
    if (!verifyTerminator())
        builder_.CreateBr(mergeBB);
    elseBB = builder_.GetInsertBlock();

    // 合并分支

    builder_.SetInsertPoint(mergeBB);
}

void CodeGenerator::visit(WhileStmt &node)
{
    Function *function = builder_.GetInsertBlock()->getParent();
    BasicBlock *condBB = BasicBlock::Create(context_, "while.cond", function);
    // BasicBlock *loopBB = BasicBlock::Create(context_, "while.body");
    BasicBlock *loopBB = BasicBlock::Create(context_, "while.body", function);
    BasicBlock *exitBB = BasicBlock::Create(context_, "while.exit", function);
    // BasicBlock *exitBB = BasicBlock::Create(context_, "while.exit");

    builder_.CreateBr(condBB);
    // 循环条件块
    builder_.SetInsertPoint(condBB);
    node.cond_->accept(*this);
    llvm::Value *condValue = currentValue_;
    condValue = builder_.CreateICmpNE(condValue,
                                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                      "whilecond");
    builder_.CreateCondBr(condValue, loopBB, exitBB);

    // 循环体块

    builder_.SetInsertPoint(loopBB);
    builder_.SetInsertPoint(loopBB);
    // 压入循环上下文（用于 break/continue 支持，此处简化）
    loopStack.push_back({condBB, exitBB});
    node.body_->accept(*this);
    if (!verifyTerminator())
        builder_.CreateBr(condBB);
    loopStack.pop_back();

    // 循环退出块

    builder_.SetInsertPoint(exitBB);
}

void CodeGenerator::visit(ReturnStmt &node)
{
    if (node.exp_)
    {
        node.exp_->accept(*this);
        currentValue_ = builder_.CreateLoad(Type::getInt32Ty(context_), currentValue_);
        builder_.CreateRet(currentValue_);
    }
    else
    {
        builder_.CreateRetVoid();
    }
}

void CodeGenerator::visit(IOStmt &node)
{
    if (node.kind == IOStmt::IOKind::Getint)
    {
        Function *getintFunc = module_->getFunction("getint");
        if (!getintFunc)
        {
            // 声明：int getint();
            FunctionType *funcType = FunctionType::get(llvm::Type::getInt32Ty(context_), false);
            getintFunc = Function::Create(funcType, Function::ExternalLinkage, "getint", module_.get());
        }
        llvm::Value *retVal = builder_.CreateCall(getintFunc, {}, "getintCall");
        // 将读入的值存入目标变量
        node.target_->accept(*this); // 生成变量指针
        llvm::Value *targetAddr = currentValue_;
        builder_.CreateStore(retVal, static_cast<AllocaInst *>(targetAddr));
        currentValue_ = retVal;
    }
    else if (node.kind == IOStmt::IOKind::Printf)
    {
        Function *printfFunc = module_->getFunction("printf");
        if (!printfFunc)
        {
            std::vector<Type *> printfArgs;
            printfArgs.push_back(Type::getInt8PtrTy(context_));
            FunctionType *printfType = FunctionType::get(
                Type::getInt32Ty(context_), printfArgs, true);
            printfFunc = Function::Create(printfType, Function::ExternalLinkage, "printf", module_.get());
        }
        llvm::Value *formatStr = builder_.CreateGlobalStringPtr(node.formatString_, "fmt");
        std::vector<llvm::Value *> printfArgs;
        printfArgs.push_back(formatStr);
        for (auto &arg : node.args_)
        {
            arg->accept(*this);
            currentValue_ = builder_.CreateLoad(Type::getInt32Ty(context_), currentValue_);
            printfArgs.push_back(currentValue_);
        }
        builder_.CreateCall(printfFunc, printfArgs, "printfCall");
    }
}

void CodeGenerator::visit(LVal &node)
{
    Symbol *sym = symtab_.lookup(node.name_);
    if (!sym)
    {
        // 如果在活动作用域中找不到，则尝试在当前函数的持久化局部符号中查找
        std::string funcName = currentFunction->getName().str();
        sym = symtab_.lookupFunctionSymbol(funcName, node.name_);
    }
    if (!sym)
    {
        reportError("Undefined variable: " + node.name_, 0);
        currentValue_ = nullptr;
        return;
    }
    VariableSymbol *varSym = static_cast<VariableSymbol *>(sym);

    // 获取变量地址（假设 varSym->allocaInst_ 已在代码生成阶段创建并保存）
    llvm::Value *varAddr = varSym->allocaInst_;
    // 如果有数组下标则调用 handleArraySubscript 生成 GEP 指令
    if (!node.indices_.empty())
    {
        varAddr = handleArraySubscript(varAddr, node.indices_);
    }
    // 生成 load 指令
    // currentValue_ = builder_.CreateLoad(Type::getInt32Ty(context_), varAddr, node.name_ + ".load");
    currentValue_ = varAddr;
}

void CodeGenerator::visit(PrimaryExp &node)
{
    if (std::holds_alternative<std::unique_ptr<Exp>>(node.operand_))
    {
        std::get<std::unique_ptr<Exp>>(node.operand_)->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<LVal>>(node.operand_))
    {
        std::get<std::unique_ptr<LVal>>(node.operand_)->accept(*this);
    }
    else if (std::holds_alternative<std::unique_ptr<Number>>(node.operand_))
    {
        std::get<std::unique_ptr<Number>>(node.operand_)->accept(*this);
    }
}

void CodeGenerator::visit(UnaryExp &node)
{
    node.operand_->accept(*this);
    llvm::Value *operandVal = currentValue_;
    switch (node.op)
    {
    case UnaryExp::Op::Plus:
        currentValue_ = operandVal;
        break;
    case UnaryExp::Op::Minus:
        currentValue_ = builder_.CreateNeg(operandVal, "negtmp");
        break;
    case UnaryExp::Op::Not:
    {
        llvm::Value *zero = ConstantInt::get(Type::getInt32Ty(context_), 0);
        currentValue_ = builder_.CreateICmpEQ(operandVal, zero, "nottmp");
        currentValue_ = builder_.CreateZExt(currentValue_, Type::getInt32Ty(context_), "booltmp");
        break;
    }
    }
}

void CodeGenerator::visit(AddExp &node)
{
    bool first = true;
    llvm::Value *result = nullptr;
    bool doAdd = true; // 当前运算符，true 表示加，false 表示减
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            std::get<std::unique_ptr<Exp>>(elem)->accept(*this);
            if (first)
            {
                result = currentValue_;
                first = false;
            }
            else
            {
                if (doAdd)
                    result = builder_.CreateAdd(result, currentValue_, "addtmp");
                else
                    result = builder_.CreateSub(result, currentValue_, "subtmp");
            }
        }
        else
        {
            TokenType op = std::get<TokenType>(elem);
            if (op == TokenType::OPERATOR_PLUS)
                doAdd = true;
            else if (op == TokenType::OPERATOR_MINUS)
                doAdd = false;
        }
    }
    currentValue_ = result;
}

void CodeGenerator::visit(MulExp &node)
{
    bool first = true;
    llvm::Value *result = nullptr;
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            std::get<std::unique_ptr<Exp>>(elem)->accept(*this);
            if (first)
            {
                result = currentValue_;
                first = false;
            }
            else
            {
                result = builder_.CreateMul(result, currentValue_, "multmp");
            }
        }
        else
        {
            // 此处可根据 op 类型选择除法或取模，目前仅支持乘法
        }
    }
    currentValue_ = result;
}

void CodeGenerator::visit(LOrExp &node)
{
    Function *function = builder_.GetInsertBlock()->getParent();
    BasicBlock *trueBB = BasicBlock::Create(context_, "lor.true", function);
    BasicBlock *falseBB = BasicBlock::Create(context_, "lor.false", function);
    BasicBlock *mergeBB = BasicBlock::Create(context_, "lor.merge", function);

    llvm::Value *resultAll = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0);
    // 此处遍历各个子表达式，简化起见只处理一个
    if (!node.elements_.empty() && std::holds_alternative<std::unique_ptr<Exp>>(node.elements_[0]))
    {
        std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
        llvm::Value *val = currentValue_;
        llvm::Value *cond = builder_.CreateICmpNE(val,
                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                                  "lorcond");
        builder_.CreateCondBr(cond, trueBB, falseBB);

        builder_.SetInsertPoint(trueBB);
        resultAll = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 1);
        builder_.CreateBr(mergeBB);

        // function->getBasicBlockList().push_back(falseBB);
        builder_.SetInsertPoint(falseBB);
        builder_.CreateBr(mergeBB);

        // function->getBasicBlockList().push_back(mergeBB);
        builder_.SetInsertPoint(mergeBB);
    }
    currentValue_ = resultAll;
}

void CodeGenerator::visit(LAndExp &node)
{
    Function *function = builder_.GetInsertBlock()->getParent();
    BasicBlock *trueBB = BasicBlock::Create(context_, "land.true", function);
    BasicBlock *falseBB = BasicBlock::Create(context_, "land.false");
    BasicBlock *mergeBB = BasicBlock::Create(context_, "land.merge", function);

    llvm::Value *resultAll = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 1);
    if (!node.elements_.empty() && std::holds_alternative<std::unique_ptr<Exp>>(node.elements_[0]))
    {
        std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
        llvm::Value *val = currentValue_;
        llvm::Value *cond = builder_.CreateICmpNE(val,
                                                  llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                                  "landcond");
        builder_.CreateCondBr(cond, trueBB, falseBB);

        builder_.SetInsertPoint(falseBB);
        resultAll = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0);
        builder_.CreateBr(mergeBB);

        // function->getBasicBlockList().push_back(trueBB);
        builder_.SetInsertPoint(trueBB);
        builder_.CreateBr(mergeBB);

        // function->getBasicBlockList().push_back(mergeBB);
        builder_.SetInsertPoint(mergeBB);
    }
    currentValue_ = resultAll;
}

void CodeGenerator::visit(EqExp &node)
{
    bool first = true;
    llvm::Value *result = nullptr;
    bool equalOp = true;
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            std::get<std::unique_ptr<Exp>>(elem)->accept(*this);
            if (first)
            {
                result = currentValue_;
                first = false;
            }
            else
            {
                if (equalOp)
                    result = builder_.CreateICmpEQ(result, currentValue_, "eqtmp");
                else
                    result = builder_.CreateICmpNE(result, currentValue_, "neqtmp");
            }
        }
        else
        {
            TokenType op = std::get<TokenType>(elem);
            equalOp = (op == TokenType::OPERATOR_EQUAL);
        }
    }
    currentValue_ = builder_.CreateZExt(result, llvm::Type::getInt32Ty(context_), "booltmp");
}

void CodeGenerator::visit(RelExp &node)
{
    bool first = true;
    llvm::Value *result = nullptr;
    TokenType relOp = TokenType::UNKNOW;
    for (auto &elem : node.elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            std::get<std::unique_ptr<Exp>>(elem)->accept(*this);
            if (first)
            {
                result = currentValue_;
                first = false;
            }
            else
            {
                switch (relOp)
                {
                case TokenType::OPERATOR_LESS:
                    result = builder_.CreateICmpSLT(result, currentValue_, "lttmp");
                    break;
                case TokenType::OPERATOR_LESS_EQUAL:
                    result = builder_.CreateICmpSLE(result, currentValue_, "letmp");
                    break;
                case TokenType::OPERATOR_GREATER:
                    result = builder_.CreateICmpSGT(result, currentValue_, "gttmp");
                    break;
                case TokenType::OPERATOR_GREATER_EQUAL:
                    result = builder_.CreateICmpSGE(result, currentValue_, "getmp");
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            relOp = std::get<TokenType>(elem);
        }
    }
    currentValue_ = builder_.CreateZExt(result, llvm::Type::getInt32Ty(context_), "booltmp");
}

void CodeGenerator::visit(CallExp &node)
{
    Function *callee = module_->getFunction(node.funcName);
    if (!callee)
    {
        reportError("Undefined function: " + node.funcName, 0);
        currentValue_ = nullptr;
        return;
    }
    std::vector<llvm::Value *> args;
    for (auto &arg : node.args_)
    {
        arg->accept(*this);
        args.push_back(currentValue_);
    }
    currentValue_ = builder_.CreateCall(callee, args, "calltmp");
}

void CodeGenerator::visit(Number &node)
{
    currentValue_ = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), node.value_);
}

llvm::Value *CodeGenerator::handleArraySubscript(llvm::Value *base, const std::vector<std::unique_ptr<AST::Exp>> &indices)
{
    return nullptr;
}

void CodeGenerator::handleGlobalInit(GlobalVariable *gvar, const InitVal &initVal)
{
    if (auto *list = std::get_if<std::vector<std::unique_ptr<InitVal>>>(&initVal.value_))
    {
        std::vector<Constant *> initValues;
        for (auto &item : *list)
        {
            if (auto *subList = std::get_if<std::vector<std::unique_ptr<InitVal>>>(&item->value_))
            {
                // 处理嵌套初始化
            }
            else
            {
                // auto *val = cast<ConstantInt>(std::get<std::unique_ptr<Exp>>(item->value_)->accept(*this));
                // initValues.push_back(val);
            }
        }
        // gvar->setInitializer(ConstantArray::get(
        // cast<ArrayType>(gvar->getType()->getPointerElementType()),
        // initValues));
    }
    else
    {
        // auto *val = cast<ConstantInt>(std::get<std::unique_ptr<Exp>>(initVal.value_)->accept(*this));
        // gvar->setInitializer(val);
    }
}

llvm::Value *CodeGenerator::generateInitValue(const AST::InitVal &initVal)
{
    return nullptr;
}

llvm::BasicBlock *CodeGenerator::createMergeBlock(const std::string &name)
{
    return nullptr;
}

bool CodeGenerator::verifyTerminator()
{
    BasicBlock *bb = builder_.GetInsertBlock();
    return bb->getTerminator() != nullptr;
}

void CodeGenerator::reportError(const std::string &msg, int line)
{
    errs() << "Error at line " << line << ": " << msg << "\n";
}

// 格式字符串处理辅助函数
std::string processFormatString(const std::string &orig)
{
    std::string processed;
    for (size_t i = 0; i < orig.size(); ++i)
    {
        if (orig[i] == '\\' && i + 1 < orig.size())
        {
            if (orig[i + 1] == 'n')
            {
                processed += '\n';
                ++i;
                continue;
            }
            // 其他转义字符处理（根据规范只有\n合法）
        }
        processed += orig[i];
    }
    return processed;
}

// 实现 getint 函数，返回 i32 类型整数，且无参数
Function *CodeGenerator::createGetintFunction(Module *module, LLVMContext &context)
{
    // 函数类型：i32 getint(void)
    FunctionType *funcType = FunctionType::get(Type::getInt32Ty(context), false);
    Function *getintFunc = Function::Create(funcType, GlobalValue::ExternalLinkage, "getint", module);

    // 创建函数入口基本块
    BasicBlock *entryBB = BasicBlock::Create(context, "entry", getintFunc);
    IRBuilder<> builder(entryBB);

    // 创建一个全局常量格式字符串 "%d"
    // 注意：字符串内容中需要加上结尾的 '\0'
    Constant *formatStr = builder.CreateGlobalStringPtr("%d", "fmt");

    // 在函数入口分配一个局部变量用于存储输入整数
    AllocaInst *nVar = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "n");

    // 声明 scanf 函数：原型为 int scanf(i8*, ...);
    // 构造函数类型：返回 i32，参数类型为 i8*，可变参数
    FunctionType *scanfType = FunctionType::get(
        Type::getInt32Ty(context),
        {Type::getInt8PtrTy(context)},
        true);
    // 使用 getOrInsertFunction 如果模块中已存在 scanf 则返回，否则创建一个新的声明
    Function *scanfFunc = cast<Function>(
        module->getOrInsertFunction("scanf", scanfType).getCallee());

    // 调用 scanf，传入格式字符串和 nVar 的地址
    builder.CreateCall(scanfFunc, {formatStr, nVar}, "scanfCall");

    // 加载 nVar 的值
    Value *nVal = builder.CreateLoad(Type::getInt32Ty(context), nVar, "n.load");

    // 返回读取到的整数
    builder.CreateRet(nVal);

    // 验证生成的函数是否正确
    verifyFunction(*getintFunc);
    return getintFunc;
}

void CodeGenerator::emitMIPSAssembly(const std::string &outputFilename)
{
    // //   1. 只初始化 MIPS 相关的目标，防止 X86 依赖
    // LLVMInitializeMipsTargetInfo();
    // LLVMInitializeMipsTarget();
    // LLVMInitializeMipsTargetMC();
    // LLVMInitializeMipsAsmParser();
    // LLVMInitializeMipsAsmPrinter();

    // //   2. 设置 MIPS 目标三元组
    // std::string targetTriple = "mips-unknown-linux-gnu";
    // std::string error;
    // const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    // if (!target)
    // {
    //     llvm::errs() << "  无法找到 MIPS 目标: " << error << "\n";
    //     return;
    // }

    // //   3. 创建 MIPS 目标机器
    // llvm::TargetOptions options;
    // std::unique_ptr<llvm::TargetMachine> targetMachine(
    //     target->createTargetMachine(targetTriple, "mips32", "", options, std::nullopt)); // ✅ 代替 llvm::Reloc::Model::PIC_

    // if (!targetMachine)
    // {
    //     llvm::errs() << "  无法创建 MIPS 目标机器！\n";
    //     return;
    // }

    // //   4. 确保 module_ 存在
    // if (!module_)
    // {
    //     llvm::errs() << "  错误：LLVM 模块未初始化！\n";
    //     return;
    // }

    // //   5. 设置模块的目标三元组和数据布局
    // module_->setTargetTriple(targetTriple);
    // module_->setDataLayout(targetMachine->createDataLayout());

    // //   6. 设置输出文件
    // std::error_code ec;
    // llvm::raw_fd_ostream dest(outputFilename, ec, llvm::sys::fs::OF_None);
    // if (ec)
    // {
    //     llvm::errs() << "  无法打开输出文件 " << outputFilename << "：" << ec.message() << "\n";
    //     return;
    // }

    // //   7. 创建 PassManager 生成 MIPS 汇编
    // llvm::legacy::PassManager passManager;
    // if (targetMachine->addPassesToEmitFile(passManager, dest, nullptr, llvm::CodeGenFileType::CGFT_AssemblyFile))
    // {
    //     llvm::errs() << "  目标机器无法生成 MIPS 汇编文件！\n";
    //     return;
    // }

    // //   8. 运行 PassManager，生成汇编代码
    // passManager.run(*module_);
    // dest.flush();

    // llvm::outs() << "✅ MIPS 汇编代码已成功写入：" << outputFilename << "\n";
    //   1. 只初始化 X86 目标，避免多余依赖
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmParser();
    LLVMInitializeX86AsmPrinter();

    //   2. 设置 X86 目标三元组
    std::string targetTriple = "x86_64-pc-linux-gnu";
    std::string error;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target)
    {
        llvm::errs() << "  无法找到 X86 目标: " << error << "\n";
        return;
    }

    //   3. 创建 X86 目标机器
    llvm::TargetOptions options;
    std::unique_ptr<llvm::TargetMachine> targetMachine(
        target->createTargetMachine(targetTriple, "x86-64", "", options, std::nullopt));

    if (!targetMachine)
    {
        llvm::errs() << "  无法创建 X86 目标机器！\n";
        return;
    }

    //   4. 确保 module_ 存在
    if (!module_)
    {
        llvm::errs() << "  错误：LLVM 模块未初始化！\n";
        return;
    }

    //   5. 设置模块的目标三元组和数据布局
    module_->setTargetTriple(targetTriple);
    module_->setDataLayout(targetMachine->createDataLayout());

    //   6. 设置输出文件
    std::error_code ec;
    llvm::raw_fd_ostream dest(outputFilename, ec, llvm::sys::fs::OF_None);
    if (ec)
    {
        llvm::errs() << "  无法打开输出文件 " << outputFilename << "：" << ec.message() << "\n";
        return;
    }

    //   7. 创建 PassManager 生成 X86 汇编
    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(passManager, dest, nullptr, llvm::CodeGenFileType::CGFT_AssemblyFile))
    {
        llvm::errs() << "  目标机器无法生成 X86 汇编文件！\n";
        return;
    }

    //   8. 运行 PassManager，生成汇编代码
    passManager.run(*module_);
    dest.flush();

    llvm::outs() << "✅ X86 汇编代码已成功写入：" << outputFilename << "\n";
}