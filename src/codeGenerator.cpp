#include "codeGenerator.h"
#include "codeGenerator.h"
#include "llvm/Support/Host.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

CodeGenerator::CodeGenerator()
    : builder_(context_), module_(std::make_unique<Module>("SysY_module", context_))
{
    createGetintFunction(module_.get(), context_);
    PushScope();
}

CodeGenerator::~CodeGenerator()
{
    // 清空所有局部作用域
    while (!localVarMap.empty())
        PopScope();
}

void CodeGenerator::generateCode(CompUnit &compUnit)
{
    compUnit.accept(*this);
    // 模块验证
    std::string errStr;
    raw_string_ostream errStream(errStr);
    if (verifyModule(*module_, &errStream))
    {
        errs() << "生成的模块验证失败:\n"
               << errStream.str();
    }
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
    // 如果是标量常量（没有数组维度信息）
    if (node.dimensions_.empty())
    {
        // 先生成常量的初始值
        llvm::Constant *initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
        // 如果有初始化表达式，则计算其值
        // 这里可以调用类似 EvalConstant 的工具函数直接求值，
        // 但为了简单示例，假定初始值已经通过语义分析保存在 node 内部
        if (node.hasInit)
        {
            // 假定 node.initVal_ 已经是求值后的常量值
            // 实际中可能需要更复杂的表达式求值过程
            initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), evalConstant.Eval(node.initVal_.get()));
        }
        // 创建全局常量变量，标记为 constant，存放在 module_ 中
        llvm::GlobalVariable *gVar = new llvm::GlobalVariable(
            *module_,
            builder_.getInt32Ty(),
            /* isConstant */ true,
            llvm::GlobalValue::ExternalLinkage,
            initVal,
            node.name_);
        // 将生成的变量添加到全局符号映射中
        AddGlobalVarToMap(gVar, builder_.getInt32Ty(), node.name_);
    }
    else
    {
    }
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
    if (node.constExps_.empty())
    {
        Constant *initVal = ConstantInt::get(builder_.getInt32Ty(), 0);
        if (node.hasInit)
        {
            // 这里可插入对 initVal 表达式的求值，本示例直接用 0
            initVal = ConstantInt::get(builder_.getInt32Ty(), 0);
        }
        GlobalVariable *gVar = new GlobalVariable(*module_, builder_.getInt32Ty(),
                                                  false, GlobalValue::ExternalLinkage, initVal, node.name_);
        // TODO: 添加进局部变量表还是全局表？通过当前函数判定？全局变量整个生命周期都在
        AddGlobalVarToMap(gVar, builder_.getInt32Ty(), node.name_);
    }
    else
    {
    }
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
}

void CodeGenerator::visit(MainFuncDef &node)
{
    // 1. 创建 main 函数：i32 @main()
    FunctionType *mainTy = FunctionType::get(builder_.getInt32Ty(), {}, false);
    Function *mainFunc = Function::Create(
        mainTy, Function::ExternalLinkage, "main", module_.get());

    // 2. 创建入口基本块
    BasicBlock *entryBB = BasicBlock::Create(context_, "entry", mainFunc);
    builder_.SetInsertPoint(entryBB);

    currentFunc_ = mainFunc;
    // 3. 进入新的局部作用域
    PushScope();

    // 4. 处理 main 函数体
    node.body_->accept(*this);

    // 5. 如果没有终结指令，则添加 `ret i32 0`
    if (!entryBB->getTerminator())
    {
        builder_.CreateRet(ConstantInt::get(builder_.getInt32Ty(), 0));
    }

    // 6. 验证函数
    if (verifyFunction(*mainFunc, &errs()))
    {
        errs() << "main 函数验证失败！\n";
    }

    // 7. 弹出作用域
    PopScope();

    // 8. 将 main 函数记录到全局符号映射（以便可能的递归或外部引用）
    AddGlobalVarToMap(mainFunc, mainFunc->getType(), "main");
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
        std::move(std::get<std::unique_ptr<Exp>>(node.value_))->accept(*this);
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

void CodeGenerator::visit(FuncType &node)
{
}

//===----------------------------------------------------------------------===//
// 控制流语句
//===----------------------------------------------------------------------===//

void CodeGenerator::visit(ExpStmt &node)
{
    node.exp_->accept(*this);
}

void CodeGenerator::visit(Block &node)
{
    PushScope();
    for (auto &item : node.items_)
    {
        item->accept(*this);
    }
    PopScope();
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
        currentValue_ = rhs;
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
    BasicBlock *elseBB = BasicBlock::Create(context_, "else", function);
    BasicBlock *mergeBB = BasicBlock::Create(context_, "ifcont", function);
    builder_.CreateCondBr(condValue, thenBB, elseBB);

    // then 分支
    builder_.SetInsertPoint(thenBB);
    node.thenBranch_->accept(*this);
    builder_.CreateBr(mergeBB);
    thenBB = builder_.GetInsertBlock();

    // else 分支

    builder_.SetInsertPoint(elseBB);
    if (node.elseBranch_)
        node.elseBranch_->accept(*this);
    builder_.CreateBr(mergeBB);
    elseBB = builder_.GetInsertBlock();

    // 合并分支

    builder_.SetInsertPoint(mergeBB);
}

void CodeGenerator::visit(WhileStmt &node)
{
    Function *function = builder_.GetInsertBlock()->getParent();
    BasicBlock *condBB = BasicBlock::Create(context_, "while.cond", function);
    BasicBlock *loopBB = BasicBlock::Create(context_, "while.body", function);
    BasicBlock *exitBB = BasicBlock::Create(context_, "while.exit", function);

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
    node.body_->accept(*this);
    builder_.CreateBr(condBB);

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
        node.target_->accept(*this);             // 生成变量指针
        llvm::Value *targetAddr = currentValue_; // 获取目标地址值
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
    auto varPair = GetVarByName(node.name_);
    if (!varPair.first)
    {
        errs() << "未找到变量 " << node.name_ << "\n";
        currentValue_ = nullptr;
        return;
    }
    // 假定变量存放的是指针，需要生成 load
    currentValue_ = varPair.first;
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

        builder_.SetInsertPoint(falseBB);
        builder_.CreateBr(mergeBB);

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

        builder_.SetInsertPoint(trueBB);
        builder_.CreateBr(mergeBB);

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
    // std::error_code EC;
    // raw_fd_ostream out(outputFilename, EC, sys::fs::OF_None);
    // if (EC)
    // {
    //     errs() << "无法打开文件: " << EC.message() << "\n";
    //     return;
    // }
    // module_->print(out, nullptr);
    // outs() << "LLVM IR 已写入 " << outputFilename << "\n";
    // 初始化LLVM目标支持
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    std::string targetTriple = sys::getDefaultTargetTriple();
    module_->setTargetTriple(targetTriple);

    std::string error;
    const Target *target = TargetRegistry::lookupTarget(targetTriple, error);
    if (!target)
    {
        errs() << error;
        return;
    }

    TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    TargetMachine *targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, RM);

    module_->setDataLayout(targetMachine->createDataLayout());

    std::error_code EC;
    raw_fd_ostream dest(outputFilename, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return;
    }

    legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, CodeGenFileType::CGFT_AssemblyFile))
    {
        errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module_);
    dest.flush();
}

void CodeGenerator::AddLocalVarToMap(llvm::Value *addr, llvm::Type *ty, llvm::StringRef name)
{
    localVarMap.back().insert({name, {addr, ty}});
}

void CodeGenerator::AddGlobalVarToMap(llvm::Value *addr, llvm::Type *ty, llvm::StringRef name)
{
    globalVarMap.insert({name, {addr, ty}});
}

std::pair<llvm::Value *, llvm::Type *> CodeGenerator::GetVarByName(llvm::StringRef name)
{
    for (auto it = localVarMap.rbegin(); it != localVarMap.rend(); ++it)
    {
        if (it->find(name) != it->end())
        {
            return (*it)[name];
        }
    }
    assert(globalVarMap.find(name) != globalVarMap.end());
    return globalVarMap[name];
}

void CodeGenerator::PushScope()
{
    localVarMap.emplace_back();
}

void CodeGenerator::PopScope()
{
    localVarMap.pop_back();
}

void CodeGenerator::ClearVarScope()
{
    localVarMap.clear();
}