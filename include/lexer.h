#ifndef LEXER_H
#define LEXER_H

#include<string>
#include<vector>
#include<unordered_map>
#include<list>
#include<iostream>

//定义词法单元类型

enum class TokenType{
    UNKNOW,
    // 关键字
    KEYWORD_AUTO,
    KEYWORD_BREAK,
    KEYWORD_CASE,
    KEYWORD_CHAR,
    KEYWORD_CONST,
    KEYWORD_CONTINUE,
    KEYWORD_DEFAULT,
    KEYWORD_DO,
    KEYWORD_DOUBLE,
    KEYWORD_ELSE,
    KEYWORD_ENUM,
    KEYWORD_EXTERN,
    KEYWORD_FLOAT,
    KEYWORD_FOR,
    KEYWORD_GOTO,
    KEYWORD_IF,
    KEYWORD_INT,
    KEYWORD_LONG,
    KEYWORD_REGISTER,
    KEYWORD_RETURN,
    KEYWORD_SHORT,
    KEYWORD_SIGNED,
    KEYWORD_SIZEOF,
    KEYWORD_STATIC,
    KEYWORD_STRUCT,
    KEYWORD_SWITCH,
    KEYWORD_TYPEDEF,
    KEYWORD_UNION,
    KEYWORD_UNSIGNED,
    KEYWORD_VOID,
    KEYWORD_VOLATILE,
    KEYWORD_WHILE,

    // 标识符
    IDENTIFIER,

    // 常量
    CONSTANT_INTEGER,
    CONSTANT_FLOAT,
    CONSTANT_CHAR,
    CONSTANT_STRING,

    // 运算符
    OPERATOR_ASSIGN,           // =
    OPERATOR_PLUS,             // +
    OPERATOR_MINUS,            // -
    OPERATOR_MULTIPLY,         // *
    OPERATOR_DIVIDE,           // /
    OPERATOR_MODULO,           // %
    OPERATOR_INCREMENT,        // ++
    OPERATOR_DECREMENT,        // --
    OPERATOR_EQUAL,            // ==
    OPERATOR_NOT_EQUAL,        // !=
    OPERATOR_LESS,             // <
    OPERATOR_LESS_EQUAL,       // <=
    OPERATOR_GREATER,          // >
    OPERATOR_GREATER_EQUAL,    // >=
    OPERATOR_LOGICAL_AND,      // &&
    OPERATOR_LOGICAL_OR,       // ||
    OPERATOR_LOGICAL_NOT,      // !
    OPERATOR_BITWISE_AND,      // &
    OPERATOR_BITWISE_OR,       // |
    OPERATOR_BITWISE_XOR,      // ^
    OPERATOR_BITWISE_NOT,      // ~
    OPERATOR_LEFT_SHIFT,       // <<
    OPERATOR_RIGHT_SHIFT,      // >>
    OPERATOR_PLUS_ASSIGN,      // +=
    OPERATOR_MINUS_ASSIGN,     // -=
    OPERATOR_MULTIPLY_ASSIGN,  // *=
    OPERATOR_DIVIDE_ASSIGN,    // /=
    OPERATOR_MODULO_ASSIGN,    // %=
    OPERATOR_AND_ASSIGN,       // &=
    OPERATOR_OR_ASSIGN,        // |=
    OPERATOR_XOR_ASSIGN,       // ^=
    OPERATOR_LEFT_SHIFT_ASSIGN, // <<=
    OPERATOR_RIGHT_SHIFT_ASSIGN, // >>=

    // 标点符号
    PUNCTUATION_LEFT_PAREN,    // (
    PUNCTUATION_RIGHT_PAREN,   // )
    PUNCTUATION_LEFT_BRACE,    // {
    PUNCTUATION_RIGHT_BRACE,   // }
    PUNCTUATION_LEFT_BRACKET,  // [
    PUNCTUATION_RIGHT_BRACKET, // ]
    PUNCTUATION_COMMA,         // ,
    PUNCTUATION_SEMICOLON,     // ;
    PUNCTUATION_COLON,         // :
    PUNCTUATION_DOT,           // .
    PUNCTUATION_ARROW,         // ->

    // 注释
    COMMENT_SINGLE_LINE,       // //
    COMMENT_MULTI_LINE,        /* /* */

    // 文件结束
    END_OF_FILE,

    // 错误
    ERROR
};





class Token{
public:
    Token();
    Token(TokenType tokentype,std::string value,int line,int colume);
    TokenType tokenType_;
    std::string value_;
    int line_;
    int colume_;
    
    

    // 将 TokenType 转换为字符串，方便调试和输出
    static std::string tokenTypeToString(TokenType type);
    friend std::ostream& operator<<(std::ostream&os,const Token&token);
};


class Lexer{
public:
    explicit Lexer(std::string sourceCode);
    void tokenize();    //扫描文本，序列化token，调用gettoken   
    std::vector<Token> getTokens();   

private:
    std::string sourceCode_;    //源代码
    size_t currentPosition_;    //当前读取位置
    int currentLine_;           //当前行
    int currentColumn_;         //当前列
    
    std::unordered_map<std::string,TokenType> keywords_={
        {"auto",TokenType::KEYWORD_AUTO},
        {"break",TokenType::KEYWORD_BREAK},
        {"case",TokenType::KEYWORD_CASE},  
        {"char", TokenType::KEYWORD_CHAR},
        {"const", TokenType::KEYWORD_CONST},
        {"continue", TokenType::KEYWORD_CONTINUE},
        {"default", TokenType::KEYWORD_DEFAULT},
        {"do", TokenType::KEYWORD_DO},
        {"double", TokenType::KEYWORD_DOUBLE},
        {"else", TokenType::KEYWORD_ELSE},
        {"enum", TokenType::KEYWORD_ENUM},
        {"extern", TokenType::KEYWORD_EXTERN},
        {"float", TokenType::KEYWORD_FLOAT},
        {"for", TokenType::KEYWORD_FOR},
        {"goto", TokenType::KEYWORD_GOTO},
        {"if", TokenType::KEYWORD_IF},
        {"int", TokenType::KEYWORD_INT},
        {"long", TokenType::KEYWORD_LONG},
        {"register", TokenType::KEYWORD_REGISTER},
        {"return", TokenType::KEYWORD_RETURN},
        {"short", TokenType::KEYWORD_SHORT},
        {"signed", TokenType::KEYWORD_SIGNED},
        {"sizeof", TokenType::KEYWORD_SIZEOF},
        {"static", TokenType::KEYWORD_STATIC},
        {"struct", TokenType::KEYWORD_STRUCT},
        {"switch", TokenType::KEYWORD_SWITCH},
        {"typedef", TokenType::KEYWORD_TYPEDEF},
        {"union", TokenType::KEYWORD_UNION},
        {"unsigned", TokenType::KEYWORD_UNSIGNED},
        {"void", TokenType::KEYWORD_VOID},
        {"volatile", TokenType::KEYWORD_VOLATILE},
        {"while", TokenType::KEYWORD_WHILE}};
    std::unordered_map<std::string,TokenType> operators_={
        {"+",TokenType::OPERATOR_PLUS},
        {"-",TokenType::OPERATOR_MINUS}, 
        {"*", TokenType::OPERATOR_MULTIPLY},
        {"/", TokenType::OPERATOR_DIVIDE},
        {"%", TokenType::OPERATOR_MODULO},
        {"++", TokenType::OPERATOR_INCREMENT},
        {"--", TokenType::OPERATOR_DECREMENT},

        // 赋值运算符
        {"=", TokenType::OPERATOR_ASSIGN},
        {"+=", TokenType::OPERATOR_AND_ASSIGN},
        {"-=", TokenType::OPERATOR_MINUS_ASSIGN},
        {"*=", TokenType::OPERATOR_MULTIPLY_ASSIGN},
        {"/=", TokenType::OPERATOR_DIVIDE_ASSIGN},
        {"%=", TokenType::OPERATOR_MODULO_ASSIGN},
        {"&=", TokenType::OPERATOR_AND_ASSIGN},
        {"|=",TokenType::OPERATOR_OR_ASSIGN},
        {"^=",TokenType::OPERATOR_XOR_ASSIGN},
        {"<<=",TokenType::OPERATOR_LEFT_SHIFT_ASSIGN},
        {">>=",TokenType::OPERATOR_RIGHT_SHIFT_ASSIGN},

        // 关系运算符
        {"==", TokenType::OPERATOR_EQUAL},
        {"!=", TokenType::OPERATOR_NOT_EQUAL},
        {">", TokenType::OPERATOR_GREATER},
        {"<", TokenType::OPERATOR_LESS},
        {">=", TokenType::OPERATOR_GREATER_EQUAL},
        {"<=", TokenType::OPERATOR_LESS_EQUAL},

        // 逻辑运算符
        {"&&", TokenType::OPERATOR_LOGICAL_AND},
        {"||", TokenType::OPERATOR_LOGICAL_OR},
        {"!", TokenType::OPERATOR_LOGICAL_NOT},

        // 位运算符
        {"&", TokenType::OPERATOR_BITWISE_AND},
        {"|", TokenType::OPERATOR_BITWISE_OR},
        {"^", TokenType::OPERATOR_BITWISE_XOR},
        {"~", TokenType::OPERATOR_BITWISE_NOT},
        {"<<", TokenType::OPERATOR_LEFT_SHIFT},
        {">>", TokenType::OPERATOR_RIGHT_SHIFT}};
    std::unordered_map<std::string,TokenType> punctuations_={
        {"(",TokenType::PUNCTUATION_LEFT_PAREN},
        {")",TokenType::PUNCTUATION_RIGHT_PAREN},
        {"{",TokenType::PUNCTUATION_LEFT_BRACE},
        {"}",TokenType::PUNCTUATION_RIGHT_BRACE},
        {"[",TokenType::PUNCTUATION_LEFT_BRACKET},
        {"]",TokenType::PUNCTUATION_RIGHT_BRACKET},
        {",",TokenType::PUNCTUATION_COMMA},
        {";",TokenType::PUNCTUATION_SEMICOLON},
        {":",TokenType::PUNCTUATION_COLON},
        {".",TokenType::PUNCTUATION_DOT},
        {"->",TokenType::PUNCTUATION_ARROW}
    };
    
    std::vector<Token> tokens_;

private:
    Token getNextToken();

    char peek(size_t ahead = 0)const;
    char advance();


    void skipWhitespaceOrComments();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readOperator();
    Token readSymbol();

    bool isOperator(char c);
    bool isSymbol(char c);
};


#endif