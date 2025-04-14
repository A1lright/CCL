#include "lexer.h"
#include "parser.h"
#include "symbolTable.h"
#include "SemanticAnalyzer.h"
#include "codeGenerator.h"
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

    // 语义分析
    SemanticAnalyzer sema;
    program->accept(sema);

    CodeGenerator codeGen;
    program->accept(codeGen);
    codeGen.emitIRToFile("output.ll");
    codeGen.emitMIPSAssembly("output.s");

    // 输出错误信息
    errorManager.reportErrors();

    return 0;
}