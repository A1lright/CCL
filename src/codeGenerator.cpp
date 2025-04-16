#include "codeGenerator.h"
#include "codeGenerator.h"
#include "llvm/Support/Host.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/Type.h"

using namespace llvm;

CodeGenerator::CodeGenerator() : builder_(context_), module_(std::make_unique<Module>("SysY_module", context_))
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
        if (node.hasInit)
        {
            initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), evalConstant.Eval(node.initVal_.get()));
        }
        // 创建全局常量变量，标记为constant，存放在module_中
        llvm::GlobalVariable *gVar = new llvm::GlobalVariable(*module_, builder_.getInt32Ty(), true, llvm::GlobalValue::ExternalLinkage, initVal, node.name_);
        // 添加到全局符号
        AddGlobalVarToMap(gVar, builder_.getInt32Ty(), node.name_);
        return;
    }
    else
    {
        // 带维度信息，为数组常量
        std::vector<uint64_t> dims;
        for (auto &dimExp : node.dimensions_)
        {
            int d = evalConstant.Eval(dimExp.get());
            dims.push_back(d);
        }
        // 支持一维或二维
        if (dims.size() > 2)
        {
            errs() << "仅支持最多二维数组初始化: " << node.name_ << "\n";
            return;
        }

        // 扁平化ConstInitVal
        std::vector<Constant *> flat;
        std::function<void(ConstInitVal *)> flatten = [&](ConstInitVal *cv)
        {
            if (auto pe = std::get_if<std::unique_ptr<Exp>>(&cv->value_))
            {
                int v = evalConstant.Eval(pe->get());
                flat.push_back(ConstantInt::get(builder_.getInt32Ty(), v));
            }
            else
            {
                auto &vec = std::get<std::vector<std::unique_ptr<ConstInitVal>>>(cv->value_);
                for (auto &child : vec)
                    flatten(child.get());
            }
        };
        flatten(node.initVal_.get());

        // 构造ConstantArray嵌套结构
        // 一维：[N x i32]
        // 二维：[M x [N x i32]]
        Type *elemTy = builder_.getInt32Ty();
        Constant *initConst = nullptr;

        if (dims.size() == 1)
        {
            // 一维数组
            uint64_t N = dims[0];
            std::vector<Constant *> elems;
            elems.reserve(N);
            for (uint64_t i = 0; i < N; ++i)
            {
                Constant *c = (i < flat.size()) ? flat[i]
                                                : ConstantInt::get(elemTy, 0);
                elems.push_back(c);
            }
            ArrayType *arrTy = ArrayType::get(elemTy, N);
            initConst = ConstantArray::get(arrTy, elems);
            // 创建全局变量
            GlobalVariable *gVar = new GlobalVariable(*module_, arrTy, true, GlobalValue::ExternalLinkage, initConst, node.name_);
            AddGlobalVarToMap(gVar, arrTy, node.name_);
        }
        else
        {
            // 二维数组
            uint64_t M = dims[0], N = dims[1];
            ArrayType *innerTy = ArrayType::get(elemTy, N);
            ArrayType *outerTy = ArrayType::get(innerTy, M);

            std::vector<Constant *> rows;
            rows.reserve(M);
            for (uint64_t i = 0; i < M; ++i)
            {
                std::vector<Constant *> row;
                row.reserve(N);
                for (uint64_t j = 0; j < N; ++j)
                {
                    uint64_t idx = i * N + j;
                    Constant *c = (idx < flat.size())
                                      ? flat[idx]
                                      : ConstantInt::get(elemTy, 0);
                    row.push_back(c);
                }
                rows.push_back(ConstantArray::get(innerTy, row));
            }
            initConst = ConstantArray::get(outerTy, rows);
            GlobalVariable *gVar = new GlobalVariable(*module_, outerTy, true, GlobalValue::ExternalLinkage, initConst, node.name_);
            AddGlobalVarToMap(gVar, outerTy, node.name_);
        }
    }
}

void CodeGenerator::visit(ConstDecl &node)
{
    // 只支持int类型，无需遍历type获取信息
    for (auto &constDef : node.constDefs_)
    {
        constDef->accept(*this);
    }
}

void CodeGenerator::visit(VarDef &node)
{
    // 如果没有维度信息，则是标量变量
    if (node.constExps_.empty())
    {
        llvm::Constant *initVal = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
        if (node.hasInit)
        {
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
            llvm::GlobalVariable *gVar = new llvm::GlobalVariable(*module_, builder_.getInt32Ty(), false, llvm::GlobalValue::ExternalLinkage, initVal, node.name_);
            AddGlobalVarToMap(gVar, builder_.getInt32Ty(), node.name_);
        }
    }
    else
    {
        // 数组变量一维或二维
        std::vector<uint64_t> dims;
        for (auto &exp : node.constExps_)
        {
            dims.push_back(evalConstant.Eval(exp.get()));
        }
        if (dims.size() > 2)
        {
            errs() << "仅支持最多二维数组：" << node.name_ << "\n";
            return;
        }

        // 扁平化InitVal
        std::vector<Constant *> flat;
        std::function<void(InitVal *)> flatten = [&](InitVal *iv)
        {
            if (auto pe = std::get_if<std::unique_ptr<Exp>>(&iv->value_))
            {
                int v = evalConstant.Eval(pe->get());
                flat.push_back(ConstantInt::get(builder_.getInt32Ty(), v));
            }
            else
            {
                auto &vec = std::get<std::vector<std::unique_ptr<InitVal>>>(iv->value_);
                for (auto &child : vec)
                    flatten(child.get());
            }
        };
        if (node.hasInit)
            flatten(node.initVal_.get());

        // 4) 构造 LLVM 数组类型
        Type *i32 = builder_.getInt32Ty();
        ArrayType *arrayTy = nullptr;
        if (dims.size() == 1)
        {
            arrayTy = ArrayType::get(i32, dims[0]);
        }
        else
        {
            ArrayType *inner = ArrayType::get(i32, dims[1]);
            arrayTy = ArrayType::get(inner, dims[0]);
        }

        // 局部 alloca 或 全局 GlobalVariable
        Value *basePtr = nullptr;
        if (currentFunc_)
        {
            basePtr = builder_.CreateAlloca(arrayTy, nullptr, node.name_);
            AddLocalVarToMap(basePtr, arrayTy, node.name_);
            //  索引列表：[0, i] 或 [0, i, j]
            Constant *zero = ConstantInt::get(i32, 0);
            if (dims.size() == 1) // 一维数组
            {
                uint64_t N = dims[0];
                for (uint64_t i = 0; i < N; ++i)
                {
                    Constant *idx1 = ConstantInt::get(i32, i);
                    Value *elemPtr = builder_.CreateGEP(
                        arrayTy, basePtr,
                        {zero, idx1},
                        node.name_ + ".idx");
                    Constant *c = (i < flat.size() ? flat[i]
                                                   : ConstantInt::get(i32, 0));
                    builder_.CreateStore(c, elemPtr);
                }
            }
            else // 二维数组
            {
                uint64_t M = dims[0], N = dims[1];
                for (uint64_t i = 0; i < M; ++i)
                {
                    for (uint64_t j = 0; j < N; ++j)
                    {
                        Constant *idx1 = ConstantInt::get(i32, i);
                        Constant *idx2 = ConstantInt::get(i32, j);
                        Value *elemPtr = builder_.CreateGEP(arrayTy, basePtr, {zero, idx1, idx2}, node.name_ + ".idx");
                        uint64_t flatIdx = i * N + j;
                        Constant *c = (flatIdx < flat.size() ? flat[flatIdx] : ConstantInt::get(i32, 0));
                        builder_.CreateStore(c, elemPtr);
                    }
                }
            }
        }
        else
        {
            // 全局数组：使用ConstantArray::get初始化数组
            // 创建内部数组的常量（每个元素都是ConstantInt）
            llvm::SmallVector<llvm::Constant *, 64> InnerConstants;
            for (size_t i = 0; i < flat.size(); ++i)
            {
                InnerConstants.push_back(flat[i]);
            }

            // 创建外部数组的常量
            llvm::ArrayType *InnerArrayTy = llvm::ArrayType::get(i32, dims[1]);          // 内部数组类型
            llvm::ArrayType *OuterArrayTy = llvm::ArrayType::get(InnerArrayTy, dims[0]); // 外部数组类型

            // 使用ConstantArray创建外部数组的初始化值
            llvm::Constant *GlobalArrayInit = llvm::ConstantArray::get(OuterArrayTy, llvm::ArrayRef<llvm::Constant *>(InnerConstants));

            // 创建全局变量并初始化
            llvm::GlobalVariable *gVar = new llvm::GlobalVariable(*module_, OuterArrayTy, true, llvm::GlobalValue::InternalLinkage, GlobalArrayInit, node.name_);
            basePtr = gVar;
            AddGlobalVarToMap(gVar, arrayTy, node.name_);
        }
    }
}

// 函数处理
void CodeGenerator::visit(FuncParam &node)
{
    // 无需处理
}

void CodeGenerator::visit(FuncDef &node)
{
    // 返回类型
    llvm::Type *retTy = (node.returnType_->typeName_ == "void") ? builder_.getVoidTy() : builder_.getInt32Ty();
    // 参数类型列表
    std::vector<llvm::Type *> paramTys;
    for (auto &param : node.params_)
    {
        llvm::Type *paramType = nullptr;
        if (param->isArray_)
        {
            // 处理多维数组参数
            std::vector<uint64_t> innerDims;

            // 跳过第一维（允许空缺），收集后续维度（必须为常量）
            for (size_t i = 1; i < param->dimSizes_.size(); ++i)
            {
                auto &dim = param->dimSizes_[i];
                innerDims.push_back(evalConstant.Eval(dim.get()));
            }

            // 从内到外构建嵌套数组类型
            Type *currentType = builder_.getInt32Ty();
            for (auto it = innerDims.rbegin(); it != innerDims.rend(); ++it)
            {
                currentType = ArrayType::get(currentType, *it);
            }
            // 内层数组类型
            // 参数类型为指向最内层数组的指针
            paramType = PointerType::get(currentType, 0);
            pointerTypeToArray_.insert(std::make_pair(paramType, currentType));
        }
        else
        {
            paramType = builder_.getInt32Ty();
        }
        paramTys.push_back(paramType);
    }

    llvm::FunctionType *funcTy = llvm::FunctionType::get(retTy, paramTys, false);

    // 在模块中创建函数
    llvm::Function *func = llvm::Function::Create(funcTy, llvm::Function::ExternalLinkage, node.name_, module_.get());
    currentFunc_ = func;

    // 在入口块开头插入 alloca
    llvm::BasicBlock *entryBB = llvm::BasicBlock::Create(context_, "entry", func);
    builder_.SetInsertPoint(entryBB);

    PushScope(); // 新作用域

    unsigned idx = 0;
    for (auto &arg : func->args())
    {
        // 获取参数元数据
        auto &param = node.params_[idx]; // FuncParam 节点
        std::string pname = param->name_;

        Type *paramTy = arg.getType(); // 从 LLVM 函数参数获取类型

        AllocaInst *alloc = builder_.CreateAlloca(
            paramTy, nullptr, pname + ".addr");
        // 将调用时传入的参数存入 alloc
        builder_.CreateStore(&arg, alloc);
        // 记录到符号映射
        // 假设 paramType 是一个 Type* 类型
        // AddLocalVarToMap(alloc, paramTy, pname);
        // 插入 load 指令
        if (paramTy->isPointerTy())
        {
            llvm::Value *loadedPtr = builder_.CreateLoad(paramTy, alloc, pname + ".load");
            // // 更新符号表中该变量的指针值为加载后的指针
            // auto var = GetVarByName(pname);
            // var.first = loadedPtr;
            AddLocalVarToMap(loadedPtr, paramTy, pname);
        }
        else
        {
            // 如果是标量参数，直接存入符号表
            AddLocalVarToMap(alloc, paramTy, pname);
        }

        idx++;
    }

    node.body_->accept(*this);

    // 函数末尾没有return，补充默认return
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
    // currentType_ = nullptr;
}

void CodeGenerator::visit(MainFuncDef &node)
{
    // 创建main函数
    FunctionType *mainTy = FunctionType::get(builder_.getInt32Ty(), {}, false);
    Function *mainFunc = Function::Create(
        mainTy, Function::ExternalLinkage, "main", module_.get());

    // 创建入口基本块
    BasicBlock *entryBB = BasicBlock::Create(context_, "entry", mainFunc);
    builder_.SetInsertPoint(entryBB);

    currentFunc_ = mainFunc;

    // 处理main函数体
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

// 控制流语句
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
    rhs = loadIfPointer(currentValue_);
    // 生成左值，得到变量地址
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

    // 转换条件值为布尔值
    condValue = builder_.CreateICmpNE(condValue, llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0), "ifcond");

    // 获取当前函数
    llvm::Function *function = builder_.GetInsertBlock()->getParent();

    // 创建基本块
    BasicBlock *thenBB = BasicBlock::Create(context_, "then", function);
    BasicBlock *elseBB = nullptr; // elseBB 仅在有 else 分支时创建
    BasicBlock *mergeBB = BasicBlock::Create(context_, "ifcont", function);

    // 如果有else分支，创建elseBB基本块
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

    BasicBlock *condBB = BasicBlock::Create(context_, "while.cond", function); // 条件判断基本块
    BasicBlock *loopBB = BasicBlock::Create(context_, "while.body", function); // 循环体基本块
    BasicBlock *exitBB = BasicBlock::Create(context_, "while.exit", function); // 循环退出基本块

    builder_.CreateBr(condBB);

    // 设置插入点到条件判断基本块
    builder_.SetInsertPoint(condBB);

    // 检查条件是否为真
    node.cond_->accept(*this);
    llvm::Value *condValue = currentValue_;

    // 转换条件值为布尔值
    condValue = builder_.CreateICmpNE(condValue,
                                      llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), 0),
                                      "whilecond");

    // 决定是否跳转到循环体或者退出循环
    builder_.CreateCondBr(condValue, loopBB, exitBB);

    // 设置插入点到循环体基本块
    builder_.SetInsertPoint(loopBB);

    // 处理循环体
    node.body_->accept(*this);

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
            currentValue_ = builder_.CreateLoad(llvm::Type::getInt32Ty(context_), currentValue_);
            printfArgs.push_back(currentValue_);
        }
        builder_.CreateCall(printfFunc, printfArgs, "printfCall");
    }
}

// 1.普通变量 2.一维数组 3.二维数组
void CodeGenerator::visit(LVal &node)
{
    // 查找变量符号（支持局部和全局变量）
    auto var = GetVarByName(node.name_);
    Value *basePtr = var.first;
    Type *baseTy = var.second;

    if (!basePtr)
    {
        currentValue_ = nullptr;
        return;
    }
    currentValue_ = basePtr;

    SmallVector<Value *, 4> indices;
    for (auto &idxExp : node.indices_)
    {
        idxExp->accept(*this);
        indices.push_back(loadIfPointer(currentValue_)); // 直接使用用户提供的索引值（如i和j）
    }

    // 处理一维数组索引
    if (indices.size() == 1)
    {
        // 一维数组
        basePtr = builder_.CreateInBoundsGEP(baseTy, basePtr, {builder_.getInt32(0), indices[0]}, node.name_ + ".idx");
        currentValue_ = basePtr; // 最终得到元素地址
        return;
    }

    // 处理数组索引（关键修正：删除硬编码的0，直接使用用户索引）
    if (baseTy->isPointerTy())
    {
        auto iter = pointerTypeToArray_.find(baseTy); // 查找键为 key 的元素
        if (iter != pointerTypeToArray_.end())
        {
            baseTy = iter->second; // 获取值
        }
        else
        {
            // 键不存在时的处理
        }
    }
    if (baseTy->isArrayTy())
    {
        bool istowdim = false;
        // 获取nei
        Type *innerType = baseTy->getArrayElementType();
        if (innerType->isArrayTy())
        {
            istowdim = true;
        }
        for (size_t i = 0; i < indices.size(); ++i)
        {
            // 对二维数组，第一次索引取行，第二次取列
            if (istowdim && i == 0)
            {
                basePtr = builder_.CreateInBoundsGEP(baseTy, basePtr, {builder_.getInt32(0), indices[i]}, node.name_ + ".idx" + std::to_string(i));
            }
            else
            {
                basePtr = builder_.CreateInBoundsGEP(baseTy, basePtr, {indices[i]}, node.name_ + ".idx" + std::to_string(i));
            }

            if (auto arrTy = dyn_cast<ArrayType>(baseTy))
            {
                baseTy = arrTy->getElementType(); // 从二维到一维，再到int
            }
        }
        isAddr = true;
        pointerTypeToArray_.insert(std::make_pair(basePtr->getType(), baseTy));
        currentValue_ = basePtr; // 最终得到元素地址
    }
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
    if (isAddr)
        return;
    currentValue_ = loadIfPointer(currentValue_); // TODO 需改进
    // 如果当前操作数是一个地址，需要从地址中加载值
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

    // 计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *val = currentValue_;

    llvm::Value *zero = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    llvm::Value *cond = builder_.CreateICmpNE(val, zero, "lor.cond0");
    // 如果为真，跳到 trueBB
    builder_.CreateCondBr(cond, trueBB, condBB);

    // 2. 处理后续子表达式
    builder_.SetInsertPoint(condBB);
    for (size_t i = 2; i < node.elements_.size(); i += 2)
    {
        // 计算当前子表达式
        std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
        val = currentValue_;
        cond = builder_.CreateICmpNE(val, zero, "lor.cond");
        // 如果为真，跳trueBB，否则继续到下一个子表达式或最终 falseBB
        BasicBlock *nextBB = (i + 2 < node.elements_.size())
                                 ? BasicBlock::Create(context_, "lor.next", function)
                                 : falseBB; // 最后一个子表达式假时留在 falseBB
        builder_.CreateCondBr(cond, trueBB, nextBB);
        // 移动插入点到nextBB以处理下一轮
        if (nextBB != falseBB)
            builder_.SetInsertPoint(nextBB);
    }

    // 生成返回1，然后跳转到mergeBB
    builder_.SetInsertPoint(trueBB);
    llvm::Value *one = llvm::ConstantInt::get(builder_.getInt32Ty(), 1);
    builder_.CreateBr(mergeBB);

    // 生成返回0，然后跳转到mergeBB
    builder_.SetInsertPoint(falseBB);
    llvm::Value *zeroRet = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    builder_.CreateBr(mergeBB);

    // 创建PHI节点合并结果
    builder_.SetInsertPoint(mergeBB);
    llvm::PHINode *phi = builder_.CreatePHI(builder_.getInt32Ty(), 2, "lor.result");
    phi->addIncoming(one, trueBB);
    phi->addIncoming(zeroRet, falseBB);

    currentValue_ = phi;
}

void CodeGenerator::visit(LAndExp &node)
{
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

    builder_.SetInsertPoint(trueBB);
    llvm::Value *one = llvm::ConstantInt::get(builder_.getInt32Ty(), 1);
    builder_.CreateBr(mergeBB);

    builder_.SetInsertPoint(falseBB);
    llvm::Value *zeroRet = llvm::ConstantInt::get(builder_.getInt32Ty(), 0);
    builder_.CreateBr(mergeBB);

    builder_.SetInsertPoint(mergeBB);
    llvm::PHINode *phi = builder_.CreatePHI(builder_.getInt32Ty(), 2, "land.result");
    phi->addIncoming(one, trueBB);
    phi->addIncoming(zeroRet, falseBB);

    currentValue_ = phi;
}

void CodeGenerator::visit(EqExp &node)
{
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *result = currentValue_;

    TokenType op = TokenType::UNKNOW;
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

            result = builder_.CreateZExt(cmp, builder_.getInt32Ty(), "eqext");
        }
    }

    // 3. 最终结果
    currentValue_ = result;
}

void CodeGenerator::visit(RelExp &node)
{
    // 计算第一个子表达式
    std::get<std::unique_ptr<Exp>>(node.elements_[0])->accept(*this);
    llvm::Value *result = currentValue_;

    // 遍历后续元素
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
            std::get<std::unique_ptr<Exp>>(node.elements_[i])->accept(*this);
            llvm::Value *rhs = currentValue_;

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
                // 直接指向下一个元素，直接返回
                break;
            }

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
    for (size_t i = 0; i < node.args_.size(); ++i)
    {
        node.args_[i]->accept(*this);
        Value *argVal = currentValue_;
        Type *paramTy = callee->getFunctionType()->getParamType(i);

        // 处理二维数组参数：将二维数组转换为指向内层数组的指针
        if (paramTy->isIntegerTy())
        {
            // 如果参数类型是整数，直接使用
        }
        else
        {
            Type *arrTy = pointerTypeToArray_[argVal->getType()];
            // TODO
            if (arrTy && arrTy->isArrayTy())
            {
                // 二维数组类型为 [M x [N x i32]]，需要获取第一行的地址（即降维为 [N x i32]*）
                Value *zero = ConstantInt::get(builder_.getInt32Ty(), 0);
                argVal = builder_.CreateInBoundsGEP(arrTy, argVal, {zero}, "array.param"); // 获取第一行指针
            }
        }

        args.push_back(argVal);
    }
    currentValue_ = builder_.CreateCall(callee, args);
}

void CodeGenerator::visit(Number &node)
{
    currentValue_ = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context_), node.value_);
}

Function *CodeGenerator::createGetintFunction(Module *module, LLVMContext &context)
{
    // i32 getint(void)
    FunctionType *funcType = FunctionType::get(Type::getInt32Ty(context), false);
    Function *getintFunc = Function::Create(funcType, GlobalValue::ExternalLinkage, "getint", module);

    BasicBlock *entryBB = BasicBlock::Create(context, "entry", getintFunc);
    IRBuilder<> builder(entryBB);

    Constant *formatStr = builder.CreateGlobalStringPtr("%d", "fmt");

    AllocaInst *nVar = builder.CreateAlloca(Type::getInt32Ty(context), nullptr, "n");

    FunctionType *scanfType = FunctionType::get(Type::getInt32Ty(context), {Type::getInt8PtrTy(context)}, true);

    Function *scanfFunc = cast<Function>(module->getOrInsertFunction("scanf", scanfType).getCallee());

    builder.CreateCall(scanfFunc, {formatStr, nVar}, "scanfCall");

    // 加载nVar的值
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

llvm::Value *CodeGenerator::loadIfPointer(llvm::Value *v)
{
    if (v && v->getType()->isPointerTy())
    {
        auto ptrTy = llvm::dyn_cast<llvm::PointerType>(v->getType());
        if (ptrTy)
        {
            return builder_.CreateLoad(llvm::Type::getInt32Ty(context_), v, "loadtmp");
        }
    }
    return v;
}