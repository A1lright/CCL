// codeGenerator.h
#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <memory>
#include <vector>
#include "astSysy.h"
#include "symbolTable.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/MCTargetOptions.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Option/Option.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ToolOutputFile.h"

using namespace AST;
using namespace llvm;

// llvm
// source -> llvm ir -> run -> output
// .ll -> module -> {function, global variable, type, constant} -> {basic block, instruction} -> {value, type}

class CodeGenerator : public AST::Visitor
{
public:
    // 初始化/清理
    CodeGenerator(SymbolTable &symtab);
    ~CodeGenerator();

    void emitMIPSAssembly(const std::string &outputFilename);

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
    void visit(Block &node);
    void visit(AssignStmt &node);
    void visit(IfStmt &node);
    void visit(WhileStmt &node);
    void visit(ReturnStmt &node);
    void visit(IOStmt &node);

    // 表达式
    void visit(LVal &node);
    void visit(PrimaryExp &node);
    // virtual void visit(BinaryExp &) = 0;
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

private:
    // LLVM核心组件
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    std::unique_ptr<llvm::Module> module_;

    // 符号表
    SymbolTable &symtab_;

    // 类型系统
    // llvm::Type* getLLVMType(const Type& type);
    Function *createGetintFunction(Module *module, LLVMContext &context);
    // 数组处理
    llvm::Value *handleArraySubscript(llvm::Value *base,
                                      const std::vector<std::unique_ptr<AST::Exp>> &indices);
    void handleGlobalInit(llvm::GlobalVariable *gvar,
                          const AST::InitVal &initVal);
    llvm::Value *generateInitValue(const AST::InitVal &initVal);

    // 控制流辅助
    llvm::BasicBlock *createMergeBlock(const std::string &name = "");
    bool verifyTerminator();

    // 错误处理
    void reportError(const std::string &msg, int line);

    // 当前函数上下文
    llvm::Function *currentFunction = nullptr;

    // 循环上下文栈（处理break/continue）
    struct LoopContext
    {
        llvm::BasicBlock *condBlock;
        llvm::BasicBlock *exitBlock;
    };
    std::vector<LoopContext> loopStack;

    llvm::Value *currentValue_ = nullptr;
};

#endif // CODEGENERATOR_H