// codeGenerator.h
#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <memory>
#include <vector>
#include "astSysy.h"
#include "symbolTable.h"
#include "evalConstant.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Config/llvm-config.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"

using namespace AST;
using namespace llvm;

// llvm
// source -> llvm ir -> run -> output
// .ll -> module -> {function, global variable, type, constant} -> {basic block, instruction} -> {value, type}

class CodeGenerator : public AST::Visitor
{
public:
    // 初始化/清理
    CodeGenerator();
    ~CodeGenerator();

    void emitMIPSAssembly(const std::string &outputFilename);
    void emitIRToFile(const std::string &outputFilename);

    // 主入口
    void generateCode(AST::CompUnit &compUnit);

    // 获取生成的模块
    std::unique_ptr<llvm::Module> getModule() { return std::move(module_); }

    // Visitor模式接口
    // 编译单元节点
    void visit(CompUnit &node);

    // 声明
    void visit(ConstDef &node);
    void visit(ConstDecl &node);
    void visit(VarDef &node);
    void visit(VarDecl &node);

    // 语句
    void visit(ExpStmt &);
    void visit(Block &node);
    void visit(AssignStmt &node);
    void visit(IfStmt &node);
    void visit(WhileStmt &node);
    void visit(ReturnStmt &node);
    void visit(IOStmt &node);

    // 表达式
    void visit(LVal &node);
    void visit(PrimaryExp &node);
    void visit(UnaryExp &node);
    void visit(AddExp &node);
    void visit(MulExp &node);

    void visit(LOrExp &node);
    void visit(LAndExp &node);
    void visit(EqExp &node);
    void visit(RelExp &node);

    void visit(CallExp &node);
    void visit(Number &node);

    void visit(FuncParam &node);
    void visit(FuncDef &node);
    void visit(MainFuncDef &node);

    void visit(BType &node);
    void visit(InitVal &node);
    void visit(ConstInitVal &node);
    void visit(BlockItem &node);
    void visit(FuncType &node);

private:
    // LLVM核心组件
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    std::unique_ptr<llvm::Module> module_;
    llvm::Function *currentFunc_ = nullptr;

private:
    void AddLocalVarToMap(llvm::Value *addr, llvm::Type *ty, llvm::StringRef name);
    void AddGlobalVarToMap(llvm::Value *addr, llvm::Type *ty, llvm::StringRef name);
    std::pair<llvm::Value *, llvm::Type *> GetVarByName(llvm::StringRef name);

    void PushScope();
    void PopScope();
    void ClearVarScope();

    llvm::Value *loadIfPointer(llvm::Value *v);

private:
    // 符号表
    llvm::SmallVector<llvm::StringMap<std::pair<llvm::Value *, llvm::Type *>>> localVarMap;
    llvm::StringMap<std::pair<llvm::Value *, llvm::Type *>> globalVarMap;

    llvm::Value *currentValue_ = nullptr;

    Function *createGetintFunction(Module *module, LLVMContext &context);

    EvalConstant evalConstant;

    llvm::Type *currentType_ = nullptr;
};

#endif // CODEGENERATOR_H