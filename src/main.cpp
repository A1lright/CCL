#include <iostream>
#include "lexer.h"
#include <fstream>

int main(int argc,char *argv[])
{
    
    // 检查命令行参数数量，确保至少有一个文件名参数
    if (argc < 2) {
        std::cerr << "请提供文件名作为命令行参数。" << std::endl;
        return 1;
    }

    // 获取文件名并转换为 std::string 类型
    std::string fileName = argv[1];

    std::string filePath(fileName);

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
    }
    // 使用迭代器读取文件内容
    std::string sourceCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    

    Lexer lexer(sourceCode);
    lexer.tokenize();
    std::vector<Token> tokenVector = lexer.getTokens();
    for(Token token:tokenVector){
        std::cout<<token;
    }

    return 0;
}