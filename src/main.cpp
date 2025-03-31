#include "lexer.h"
#include "parser.h"
#include "symbolTable.h"
#include "SymbolTypeBuilder.h"
#include "codeGenerator.h"
#include "SemanticAnalyzer.h"
#include "SyntaxOutputVisitor.hpp"
#include <iostream>
#include <fstream>

// 从文件中读取源代码
std::string getFile(std::string filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
    }
    // 使用迭代器读取文件内容
    std::string sourceCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return sourceCode;
}

int main(int argc, char *argv[])
{
    // 检查命令行参数数量，确保至少有一个文件名参数
    if (argc < 2)
    {
        std::cerr << "请提供文件名作为命令行参数。" << std::endl;
        return 1;
    }

    // 获取文件名并转换为 std::string 类型
    std::string filePath = argv[1];
    std::string sourceCode = getFile(filePath);

    // 初始化符号表和错误管理器
    SymbolTable symbolTable;
    ErrorManager &errorManager = ErrorManager::getInstance();

    // 词法分析
    Lexer lexer(sourceCode);
    lexer.tokenize();
    lexer.printTokens();
    std::vector<Token> tokenVector = lexer.getTokens();

    // 语法分析
    Parser parser(tokenVector, symbolTable);
    std::unique_ptr<AST::CompUnit> program = parser.parseCompUnit();
    SyntaxOutputVisitor syntaxcout;
    program->accept(syntaxcout);
    // program->accept(symbolManager);

    //语义分析
    SemanticAnalyzer semanticAnalyzer(symbolTable);
    program->accept(semanticAnalyzer);

    // 语法树输出中间代码
    CodeGenerator codeGenerator(symbolTable);
    codeGenerator.generateCode(*program);
    codeGenerator.emitMIPSAssembly("/home/lin/CCl/output.s");
    codeGenerator.getModule()->print(llvm::outs(), nullptr);

    // 输出错误信息
    errorManager.reportErrors();

    return 0;
}