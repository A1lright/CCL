#include <memory>
#include <vector>
#include "lexer.h"
#include <stdexcept>
#include "ast.h"

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);

    // 程序入口：解析整个程序
    std::unique_ptr<AST::Program> parse();

    

private:
    struct ParseError : std::runtime_error
    {
        int line_;
        int column_;

        ParseError(const std::string &msg, int ln, int col); 
    };
    
    const std::vector<Token> &tokens_;
    size_t currentPos_;
    Token currentToken_;
    std::vector<ParseError> errors_;

    //============基础方法==============================================================
    void advance();
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &message);

    //============错误处理==============================================================
    

    ParseError error(const std::string &message);
   
    void reportError(const ParseError&e);

    //错误恢复：同步到下一个声明或语句边界
    void synchronize();

    //==================声明解析==================

    bool checkDeclaration()const;

    std::unique_ptr<AST::Declaration> parseDeclaration();

    std::unique_ptr<AST::TypeSpecifier>parseType();


    //==================函数声明===================
    std::unique_ptr<AST::FunctionDeclaration>parseFunctionDeclaration(
        std::unique_ptr<AST::TypeSpecifier>returnType
    );

    std::vector<std::unique_ptr<AST::Parameter>>parseParameterList();

    std::unique_ptr<AST::Parameter>parseParameter();


    //=====================语句解析=================
    std::unique_ptr<AST::Statement>ParseStatement();

    //块语句
    std::unique_ptr<AST::CompoundStatement>parseBlockStatement();

    //if语句
    std::unique_ptr<AST::IfStatement>parseIfStatement();

    //while循环
    std::unique_ptr<AST::WhileStatement>parseWhileStatement();

    //return语句
    std::unique_ptr<AST::ReturnStatement>parseReturnStatement();

    //表达式语句
    std::unique_ptr<AST::ExpressionStatement>parseExpressionStatement();

    //===============表达式解析=======================
    std::unique_ptr<AST::Expression> parseExpression();

    //赋值表达式（右结合）
    std::unique_ptr<AST::Expression>parseAssignment();

    //关系表达式（==，!=）
    std::unique_ptr<AST::Expression>parseEquality();

    //比较表达式（<,<=,>,>=）
    std::unique_ptr<AST::Expression>parseRelational();

    //加减表达式
    std::unique_ptr<AST::Expression>parseAdditive();

    //乘除表达式
    std::unique_ptr<AST::Expression>parseMultiplicative();

    //基本表达式元素
    std::unique_ptr<AST::Expression> parsePrimary();

    //函数调用
    std::unique_ptr<AST::FunctionCall>parseFunctionCall();

    //工具函数：创建二元表达式
    std::unique_ptr<AST::BinaryExpression>makeBinary(
        std::unique_ptr<AST::Expression>lhs,const std::string&op,int precedence);

    //获取当前优先级
    int getCurrentPrecedence()const;

    bool check(TokenType type)const{
        return currentToken_.tokenType_==type;
    }

    std::unique_ptr<AST::Expression>parseRightOperand(int precedence);
};