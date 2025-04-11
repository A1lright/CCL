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
    // 如果没有维度信息，则是标量变量
    if (node.constExps_.empty())
    {
        llvm::Constant *initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
        if (node.hasInit)
        {
            // 使用 evalConstant 来计算初始值
            // TODO: 这里需要处理更复杂的初始化表达式
            // 遍历 node.constExps_，计算初始值
            initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), evalConstant.Eval(node.initVal_.get()));
        }

        // 如果是在当前函数作用域内定义的局部变量，则用 Alloca
        if (currentFunc_)
        {
            AllocaInst *allocaInst = builder_.CreateAlloca(builder_.getInt32Ty(), nullptr, node.name_);
            builder_.CreateStore(initVal, allocaInst);
            AddLocalVarToMap(allocaInst, builder_.getInt32Ty(), node.name_);
        }
        else
        {
            // 如果是全局变量，创建全局变量
            llvm::GlobalVariable *gVar = new llvm::GlobalVariable(
                *module_,
                builder_.getInt32Ty(),
                false, llvm::GlobalValue::ExternalLinkage,
                initVal,
                node.name_);
            AddGlobalVarToMap(gVar, builder_.getInt32Ty(), node.name_);
        }
    }
    else
    {
        // 如果是数组类型，可以在这里处理
        // TODO: 数组初始化
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
    // --- 1.1 构造函数类型 ---
    // 返回类型
    llvm::Type *retTy = (node.returnType_->typeName_ == "void")
                            ? builder_.getVoidTy()
                            : builder_.getInt32Ty();
    // 参数类型列表
    std::vector<llvm::Type *> paramTys;
    for (auto &param : node.params_)
    {
        // SysY 只支持 int 和 int[] 形参，均映射为 i32*
        paramTys.push_back(builder_.getInt32Ty());
    }
    llvm::FunctionType *funcTy = llvm::FunctionType::get(retTy, paramTys, false);

    // --- 1.2 在模块中创建函数 ---
    llvm::Function *func = llvm::Function::Create(
        funcTy,
        llvm::Function::ExternalLinkage,
        node.name_,
        module_.get());
    currentFunc_ = func;

    // --- 1.3 为形参命名并分配空间 ---
    // 在入口块开头插入 alloca
    llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entryBB);

    PushScope(); // 新作用域

    unsigned idx = 0;
    for (auto &arg : func->args())
    {
        // 为每个参数创建 alloca
        std::string pname = node.params_[idx]->name_;
        llvm::AllocaInst *alloc = builder_.CreateAlloca(
            builder_.getInt32Ty(), nullptr, pname);
        // 将调用时传入的参数存入 alloc
        builder_.CreateStore(&arg, alloc);
        // 记录到符号映射
        AddLocalVarToMap(alloc, builder_.getInt32Ty(), pname);
        idx++;
    }

    // --- 1.4 生成函数体 ---
    node.body_->accept(*this);

    // --- 1.5 如果函数末尾没有 return，补充默认 return ---
    if (!entryBB->getTerminator())
    {
        if (retTy->isVoidTy())
        {
            builder_.CreateRetVoid();
        }
        else
        {
            builder_.CreateRet(llvm::ConstantInt::get(builder_.getInt32Ty(), 0));
        }
    }

    // 验证
    llvm::verifyFunction(*func);

    PopScope();
}

void CodeGenerator::visit(MainFuncDef &node)
{
    // 创建 main 函数：i32 @main()
    FunctionType *mainTy = FunctionType::get(builder_.getInt32Ty(), {}, false);
    Function *mainFunc = Function::Create(
        mainTy, Function::ExternalLinkage, "main", module_.get());

    // 创建入口基本块
    BasicBlock *entryBB = BasicBlock::Create(context_, "entry", mainFunc);
    builder_.SetInsertPoint(entryBB);

    currentFunc_ = mainFunc;

    // 处理 main 函数体
    node.body_->accept(*this);

    // 如果没有终结指令，则添加 `ret i32 0`
    if (!entryBB->getTerminator())
    {
        builder_.CreateRet(ConstantInt::get(builder_.getInt32Ty(), 0));
    }

    // 验证函数
    if (verifyFunction(*mainFunc, &errs()))
    {
        errs() << "main 函数验证失败！\n";
    }

    // 将 main 函数记录到全局符号映射（以便可能的递归或外部引用）
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
    // 处理条件表达式
    node.cond_->accept(*this);
    llvm::Value *condValue = currentValue_;

    // 转换条件值为布尔值（0 为 false，非 0 为 true）
    condValue = builder_.CreateICmpNE(condValue,
                                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                      "ifcond");

    // 获取当前函数
    llvm::Function *function = builder_.GetInsertBlock()->getParent();

    // 创建基本块
    BasicBlock *thenBB = BasicBlock::Create(context_, "then", function);
    BasicBlock *elseBB = nullptr; // elseBB 仅在有 else 分支时创建
    BasicBlock *mergeBB = BasicBlock::Create(context_, "ifcont", function);

    // 如果有 else 分支，创建 elseBB 基本块
    if (node.elseBranch_)
    {
        elseBB = BasicBlock::Create(context_, "else", function);
    }

    // 创建条件跳转指令
    builder_.CreateCondBr(condValue, thenBB, elseBB ? elseBB : mergeBB);

    // 处理 then 分支
    builder_.SetInsertPoint(thenBB);
    node.thenBranch_->accept(*this);

    // 如果没有跳转到 else 部分，直接跳转到 mergeBB
    builder_.CreateBr(mergeBB);

    // 设置当前的基本块为 thenBB
    thenBB = builder_.GetInsertBlock();

    // 处理 else 分支（如果有的话）
    if (elseBB)
    {
        builder_.SetInsertPoint(elseBB);
        node.elseBranch_->accept(*this);
        builder_.CreateBr(mergeBB);
        elseBB = builder_.GetInsertBlock();
    }

    // 合并分支，设置插入点到 mergeBB
    builder_.SetInsertPoint(mergeBB);
}

void CodeGenerator::visit(WhileStmt &node)
{
    // 获取当前函数
    llvm::Function *function = builder_.GetInsertBlock()->getParent();

    // 创建循环的基本块
    BasicBlock *condBB = BasicBlock::Create(context_, "while.cond", function); // 条件判断基本块
    BasicBlock *loopBB = BasicBlock::Create(context_, "while.body", function); // 循环体基本块
    BasicBlock *exitBB = BasicBlock::Create(context_, "while.exit", function); // 循环退出基本块

    // 跳转到条件判断基本块
    builder_.CreateBr(condBB);

    // 设置插入点到条件判断基本块
    builder_.SetInsertPoint(condBB);

    // 处理条件表达式（检查条件是否为真）
    node.cond_->accept(*this);
    llvm::Value *condValue = currentValue_;

    // 转换条件值为布尔值（0 为 false，非 0 为 true）
    condValue = builder_.CreateICmpNE(condValue,
                                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                      "whilecond");

    // 根据条件判断，决定是否跳转到循环体或者退出循环
    builder_.CreateCondBr(condValue, loopBB, exitBB);

    // 设置插入点到循环体基本块
    builder_.SetInsertPoint(loopBB);

    // 处理循环体
    node.body_->accept(*this);

    // 循环体执行完后跳转回条件判断基本块
    builder_.CreateBr(condBB);

    // 设置插入点到循环退出基本块
    builder_.SetInsertPoint(exitBB);
}

void CodeGenerator::visit(ReturnStmt &node)
{
    if (node.exp_)
    {
        node.exp_->accept(*this);
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
            // currentValue_ = builder_.CreateLoad(llvm::Type::getInt32Ty(context_), currentValue_);
            printfArgs.push_back(currentValue_);
        }
        builder_.CreateCall(printfFunc, printfArgs, "printfCall");
    }
}

void CodeGenerator::visit(LVal &node)
{
    // 查找符号表中的变量
    auto varPair = GetVarByName(node.name_);
    if (!varPair.first)
    {
        errs() << "未找到变量 " << node.name_ << "\n";
        currentValue_ = nullptr;
        return;
    }
    // 直接加载变量值
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
    // 如果当前操作数是一个地址，需要从地址中加载值
    if (currentValue_->getType()->isPointerTy())
    {
        // 当前值是一个指针，使用 CreateLoad 加载值
        auto ptrType = llvm::dyn_cast<llvm::PointerType>(currentValue_->getType());
        if (ptrType)
        {
            currentValue_ = builder_.CreateLoad(llvm::Type::getInt32Ty(context_), currentValue_, "loadtmp");
        }
    }
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
    // 如果只有一个子表达式，直接计算
    if (node.elements_.size() == 1)
    {
        std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
        // 将结果转换为 0/1
        llvm::Value *zero = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
        llvm::Value *cmp = builder_.CreateICmpNE(currentValue_, zero, "lor_single");
        currentValue_ = builder_.CreateZExt(cmp, builder_.getInt32Ty(), "lor_single_ext");
        return;
    }

    llvm::Function *function = builder_.GetInsertBlock()->getParent();

    // 创建 true, false, merge 三个基本块
    BasicBlock *condBB = BasicBlock::Create(context_, "lor.cond", function);
    BasicBlock *trueBB = BasicBlock::Create(context_, "lor.true", function);
    BasicBlock *falseBB = BasicBlock::Create(context_, "lor.false", function);
    BasicBlock *mergeBB = BasicBlock::Create(context_, "lor.merge", function);

    // 1. 计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *val = currentValue_;
    // 检查是否非零
    llvm::Value *zero = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    llvm::Value *cond = builder_.CreateICmpNE(val, zero, "lor.cond0");
    // 如果为真，跳到 trueBB；否则跳到下一个子表达式的块（我们用 falseBB 临时承载）
    builder_.CreateCondBr(cond, trueBB, condBB);

    // 2. 处理后续子表达式
    builder_.SetInsertPoint(condBB);
    // 从第二个元素开始，每隔一个元素是子表达式
    for (size_t i = 2; i < node.elements_.size(); i += 2)
    {
        // 计算当前子表达式
        std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
        val = currentValue_;
        cond = builder_.CreateICmpNE(val, zero, "lor.cond");
        // 如果为真，跳 trueBB，否则继续到下一个子表达式或最终 falseBB
        BasicBlock *nextBB = (i + 2 < node.elements_.size())
                                 ? BasicBlock::Create(context_, "lor.next", function)
                                 : falseBB; // 最后一个子表达式假时留在 falseBB
        builder_.CreateCondBr(cond, trueBB, nextBB);
        // 移动插入点到 nextBB 以处理下一轮
        if (nextBB != falseBB)
            builder_.SetInsertPoint(nextBB);
    }

    // 3. 在 trueBB 中，生成返回 1，然后跳转到 mergeBB
    builder_.SetInsertPoint(trueBB);
    llvm::Value *one = llvm::ConstantInt::get(builder_.getInt32Ty(), 1);
    builder_.CreateBr(mergeBB);

    // 4. 在 falseBB 中，生成返回 0，然后跳转到 mergeBB
    builder_.SetInsertPoint(falseBB);
    llvm::Value *zeroRet = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    builder_.CreateBr(mergeBB);

    // 5. 在 mergeBB 中，创建 PHI 节点合并结果
    builder_.SetInsertPoint(mergeBB);
    llvm::PHINode *phi = builder_.CreatePHI(builder_.getInt32Ty(), 2, "lor.result");
    phi->addIncoming(one, trueBB);
    phi->addIncoming(zeroRet, falseBB);

    currentValue_ = phi;
}

void CodeGenerator::visit(LAndExp &node)
{
    // 如果只有一个子表达式，直接计算并转换为 0/1
    if (node.elements_.size() == 1)
    {
        std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
        llvm::Value *zero = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
        llvm::Value *cmp = builder_.CreateICmpNE(currentValue_, zero, "land_single");
        currentValue_ = builder_.CreateZExt(cmp, builder_.getInt32Ty(), "land_single_ext");
        return;
    }

    llvm::Function *function = builder_.GetInsertBlock()->getParent();

    // 创建 true, false, merge 三个基本块
    BasicBlock *condBB = BasicBlock::Create(context_, "land.cond", function);
    BasicBlock *trueBB = BasicBlock::Create(context_, "land.true", function);
    BasicBlock *falseBB = BasicBlock::Create(context_, "land.false", function);
    BasicBlock *mergeBB = BasicBlock::Create(context_, "land.merge", function);

    // 1. 计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *val = currentValue_;
    llvm::Value *zero = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    llvm::Value *cond = builder_.CreateICmpNE(val, zero, "land.cond0");
    // 如果为真，继续到下一个；否则跳到 falseBB
    builder_.CreateCondBr(cond, condBB, falseBB);

    // 2. 处理后续子表达式
    builder_.SetInsertPoint(condBB);
    for (size_t i = 2; i < node.elements_.size(); i += 2)
    {
        // 计算当前子表达式
        std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
        val = currentValue_;
        cond = builder_.CreateICmpNE(val, zero, "land.cond");

        // 如果为真，跳到下一个或 trueBB；否则跳到 falseBB
        BasicBlock *nextBB = (i + 2 < node.elements_.size())
                                 ? BasicBlock::Create(context_, "land.next", function)
                                 : trueBB; // 最后一个真时留在 trueBB
        builder_.CreateCondBr(cond, nextBB, falseBB);
        if (nextBB != trueBB)
            builder_.SetInsertPoint(nextBB);
    }

    // 3. 在 trueBB 中，生成返回 1，然后跳转到 mergeBB
    builder_.SetInsertPoint(trueBB);
    llvm::Value *one = llvm::ConstantInt::get(builder_.getInt32Ty(), 1);
    builder_.CreateBr(mergeBB);

    // 4. 在 falseBB 中，生成返回 0，然后跳转到 mergeBB
    builder_.SetInsertPoint(falseBB);
    llvm::Value *zeroRet = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    builder_.CreateBr(mergeBB);

    // 5. 在 mergeBB 中，创建 PHI 节点合并结果
    builder_.SetInsertPoint(mergeBB);
    llvm::PHINode *phi = builder_.CreatePHI(builder_.getInt32Ty(), 2, "land.result");
    phi->addIncoming(one, trueBB);
    phi->addIncoming(zeroRet, falseBB);

    currentValue_ = phi;
}

void CodeGenerator::visit(EqExp &node)
{
    // 1. 先计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *result = currentValue_;

    // 2. 遍历后续元素
    TokenType op = TokenType::UNKNOW; // 记录当前操作符
    for (size_t i = 1; i < node.elements_.size(); ++i)
    {
        if (std::holds_alternative<TokenType>(node.elements_[i]))
        {
            // 操作符：== 或 !=
            op = std::get<TokenType>(node.elements_[i]);
        }
        else
        {
            // 子表达式
            std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
            llvm::Value *rhs = currentValue_;

            // 生成比较指令
            llvm::Value *cmp = nullptr;
            if (op == TokenType::OPERATOR_EQUAL)
            {
                cmp = builder_.CreateICmpEQ(result, rhs, "eqtmp");
            }
            else if (op == TokenType::OPERATOR_NOT_EQUAL)
            {
                cmp = builder_.CreateICmpNE(result, rhs, "netmp");
            }

            // 将布尔值 (i1) 扩展为 i32 (0 或 1)
            result = builder_.CreateZExt(cmp, builder_.getInt32Ty(), "eqext");
        }
    }

    // 3. 最终结果
    currentValue_ = result;
}

void CodeGenerator::visit(RelExp &node)
{
    // 1. 先计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *result = currentValue_;

    // 2. 遍历后续元素
    TokenType relOp = TokenType::UNKNOW;
    for (size_t i = 1; i < node.elements_.size(); ++i)
    {
        if (std::holds_alternative<TokenType>(node.elements_[i]))
        {
            // 操作符：<, <=, >, >=
            relOp = std::get<TokenType>(node.elements_[i]);
        }
        else
        {
            // 子表达式
            std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
            llvm::Value *rhs = currentValue_;

            // 根据 relOp 生成相应的比较
            llvm::Value *cmp = nullptr;
            switch (relOp)
            {
            case TokenType::OPERATOR_LESS:
                cmp = builder_.CreateICmpSLT(result, rhs, "lttmp");
                break;
            case TokenType::OPERATOR_LESS_EQUAL:
                cmp = builder_.CreateICmpSLE(result, rhs, "letmp");
                break;
            case TokenType::OPERATOR_GREATER:
                cmp = builder_.CreateICmpSGT(result, rhs, "gttmp");
                break;
            case TokenType::OPERATOR_GREATER_EQUAL:
                cmp = builder_.CreateICmpSGE(result, rhs, "getmp");
                break;
            default:
                // 不应该到这里
                break;
            }

            // 将 i1 扩展为 i32
            result = builder_.CreateZExt(cmp, builder_.getInt32Ty(), "rel_ext");
        }
    }

    // 3. 最终结果
    currentValue_ = result;
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
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::CGFT_AssemblyFile))
    {
        errs() << "TargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module_);
    dest.flush();
}

void CodeGenerator::emitIRToFile(const std::string &outputFilename)
{

    // 确保文件可以正确打开
    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFilename, EC, llvm::sys::fs::OF_None);

    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message() << "\n";
        return;
    }

    // 输出模块的 IR 到文件
    module_->print(dest, nullptr);

    llvm::outs() << "IR code has been written to: " << outputFilename << "\n";
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