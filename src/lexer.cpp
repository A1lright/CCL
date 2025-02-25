#include "lexer.h"
#include <iomanip>

Token::Token() : tokenType_(TokenType::UNKNOW), value_(std::string("unknow")), line_(-1), colume_(-1) {}

Token::Token(TokenType tokentype, std::string value, int line, int colume) : tokenType_(tokentype), value_(value), line_(line), colume_(colume) {}

std::string Token::tokenTypeToString(TokenType tokenType)
{
    switch (tokenType)
    {
    // 关键字
    case TokenType::KEYWORD_AUTO:
        return "KEYWORD_AUTO";
    case TokenType::KEYWORD_BREAK:
        return "BREAKTK";
    case TokenType::KEYWORD_CASE:
        return "KEYWORD_CASE";
    case TokenType::KEYWORD_CHAR:
        return "KEYWORD_CHAR";
    case TokenType::KEYWORD_CONST:
        return "CONSTTK";
    case TokenType::KEYWORD_CONTINUE:
        return "CONTINUETK";
    case TokenType::KEYWORD_DEFAULT:
        return "KEYWORD_DEFAULT";
    case TokenType::KEYWORD_DO:
        return "KEYWORD_DO";
    case TokenType::KEYWORD_DOUBLE:
        return "KEYWORD_DOUBLE";
    case TokenType::KEYWORD_ELSE:
        return "ELSETK";
    case TokenType::KEYWORD_ENUM:
        return "KEYWORD_ENUM";
    case TokenType::KEYWORD_EXTERN:
        return "KEYWORD_EXTERN";
    case TokenType::KEYWORD_FLOAT:
        return "KEYWORD_FLOAT";
    case TokenType::KEYWORD_FOR:
        return "KEYWORD_FOR";
    case TokenType::KEYWORD_GOTO:
        return "KEYWORD_GOTO";
    case TokenType::KEYWORD_IF:
        return "IFTK";
    case TokenType::KEYWORD_INT:
        return "INTTK";
    case TokenType::KEYWORD_LONG:
        return "KEYWORD_LONG";
    case TokenType::KEYWORD_REGISTER:
        return "KEYWORD_REGISTER";
    case TokenType::KEYWORD_RETURN:
        return "RETURNTK";
    case TokenType::KEYWORD_SHORT:
        return "KEYWORD_SHORT";
    case TokenType::KEYWORD_SIGNED:
        return "KEYWORD_SIGNED";
    case TokenType::KEYWORD_SIZEOF:
        return "KEYWORD_SIZEOF";
    case TokenType::KEYWORD_STATIC:
        return "KEYWORD_STATIC";
    case TokenType::KEYWORD_STRUCT:
        return "KEYWORD_STRUCT";
    case TokenType::KEYWORD_SWITCH:
        return "KEYWORD_SWITCH";
    case TokenType::KEYWORD_TYPEDEF:
        return "KEYWORD_TYPEDEF";
    case TokenType::KEYWORD_UNION:
        return "KEYWORD_UNION";
    case TokenType::KEYWORD_UNSIGNED:
        return "KEYWORD_UNSIGNED";
    case TokenType::KEYWORD_VOID:
        return "VOIDTK";
    case TokenType::KEYWORD_VOLATILE:
        return "KEYWORD_VOLATILE";
    case TokenType::KEYWORD_WHILE:
        return "WHILETK";

    case TokenType::KEYWORD_GETINT:
        return "GETINTTK";
    case TokenType::KEYWORD_PRINTF:
        return "PRINTTK";
    case TokenType::KEYWORD_MAIN:
        return "MAINTK";

    // 标识符
    case TokenType::IDENTIFIER:
        return "IDENFR";

    // 常量
    case TokenType::CONSTANT_INTEGER:
        return "INTCON";
    case TokenType::CONSTANT_FLOAT:
        return "CONSTANT_FLOAT";
    case TokenType::CONSTANT_CHAR:
        return "CONSTANT_CHAR";
    case TokenType::CONSTANT_STRING:
        return "STRCON";

    // 运算符
    case TokenType::OPERATOR_ASSIGN:
        return "ASSIGN";
    case TokenType::OPERATOR_PLUS:
        return "PLUS";
    case TokenType::OPERATOR_MINUS:
        return "MINU";
    case TokenType::OPERATOR_MULTIPLY:
        return "MULT";
    case TokenType::OPERATOR_DIVIDE:
        return "DIV";
    case TokenType::OPERATOR_MODULO:
        return "MOD";
    case TokenType::OPERATOR_INCREMENT:
        return "OPERATOR_INCREMENT";
    case TokenType::OPERATOR_DECREMENT:
        return "OPERATOR_DECREMENT";
    case TokenType::OPERATOR_EQUAL:
        return "OPERATOR_EQUAL";
    case TokenType::OPERATOR_NOT_EQUAL:
        return "OPERATOR_NOT_EQUAL";
    case TokenType::OPERATOR_LESS:
        return "OPERATOR_LESS";
    case TokenType::OPERATOR_LESS_EQUAL:
        return "OPERATOR_LESS_EQUAL";
    case TokenType::OPERATOR_GREATER:
        return "OPERATOR_GREATER";
    case TokenType::OPERATOR_GREATER_EQUAL:
        return "OPERATOR_GREATER_EQUAL";
    case TokenType::OPERATOR_LOGICAL_AND:
        return "AND";
    case TokenType::OPERATOR_LOGICAL_OR:
        return "OR";
    case TokenType::OPERATOR_LOGICAL_NOT:
        return "NOT";
    case TokenType::OPERATOR_BITWISE_AND:
        return "OPERATOR_BITWISE_AND";
    case TokenType::OPERATOR_BITWISE_OR:
        return "OPERATOR_BITWISE_OR";
    case TokenType::OPERATOR_BITWISE_XOR:
        return "OPERATOR_BITWISE_XOR";
    case TokenType::OPERATOR_BITWISE_NOT:
        return "OPERATOR_BITWISE_NOT";
    case TokenType::OPERATOR_LEFT_SHIFT:
        return "OPERATOR_LEFT_SHIFT";
    case TokenType::OPERATOR_RIGHT_SHIFT:
        return "OPERATOR_RIGHT_SHIFT";
    case TokenType::OPERATOR_PLUS_ASSIGN:
        return "OPERATOR_PLUS_ASSIGN";
    case TokenType::OPERATOR_MINUS_ASSIGN:
        return "OPERATOR_MINUS_ASSIGN";
    case TokenType::OPERATOR_MULTIPLY_ASSIGN:
        return "OPERATOR_MULTIPLY_ASSIGN";
    case TokenType::OPERATOR_DIVIDE_ASSIGN:
        return "OPERATOR_DIVIDE_ASSIGN";
    case TokenType::OPERATOR_MODULO_ASSIGN:
        return "OPERATOR_MODULO_ASSIGN";
    case TokenType::OPERATOR_AND_ASSIGN:
        return "OPERATOR_AND_ASSIGN";
    case TokenType::OPERATOR_OR_ASSIGN:
        return "OPERATOR_OR_ASSIGN";
    case TokenType::OPERATOR_XOR_ASSIGN:
        return "OPERATOR_XOR_ASSIGN";
    case TokenType::OPERATOR_LEFT_SHIFT_ASSIGN:
        return "OPERATOR_LEFT_SHIFT_ASSIGN";
    case TokenType::OPERATOR_RIGHT_SHIFT_ASSIGN:
        return "OPERATOR_RIGHT_SHIFT_ASSIGN";

    // 标点符号
    case TokenType::PUNCTUATION_LEFT_PAREN:
        return "LPARENT";
    case TokenType::PUNCTUATION_RIGHT_PAREN:
        return "RPARENT";
    case TokenType::PUNCTUATION_LEFT_BRACE:
        return "LBRACE";
    case TokenType::PUNCTUATION_RIGHT_BRACE:
        return "RBRACE";
    case TokenType::PUNCTUATION_LEFT_BRACKET:
        return "LBRACK";
    case TokenType::PUNCTUATION_RIGHT_BRACKET:
        return "RBRACK";
    case TokenType::PUNCTUATION_COMMA:
        return "COMMA";
    case TokenType::PUNCTUATION_SEMICOLON:
        return "SEMICN";
    case TokenType::PUNCTUATION_COLON:
        return "PUNCTUATION_COLON";
    case TokenType::PUNCTUATION_DOT:
        return "PUNCTUATION_DOT";
    case TokenType::PUNCTUATION_ARROW:
        return "PUNCTUATION_ARROW";

    // 注释
    case TokenType::COMMENT_SINGLE_LINE:
        return "COMMENT_SINGLE_LINE";
    case TokenType::COMMENT_MULTI_LINE:
        return "COMMENT_MULTI_LINE";

    // 文件结束
    case TokenType::END_OF_FILE:
        return "END_OF_FILE";

    // 错误
    case TokenType::ERROR:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

Lexer::Lexer(std::string sourceCode) : sourceCode_(sourceCode), currentPosition_(0), currentLine_(1), currentColumn_(1) {}

void Lexer::tokenize()
{
    Token token;
    while (token.tokenType_ != TokenType::END_OF_FILE)
    {
        token = getNextToken();
        //std::cout << token;
        tokens_.push_back(token);
    }
}

std::vector<Token> Lexer::getTokens()
{
    return tokens_;
}

Token Lexer::getNextToken()
{

    // 跳过空白字符
    skipWhitespaceOrComments();

    // 是否结尾
    if (currentPosition_ >= sourceCode_.length())
    {
        return Token(TokenType::END_OF_FILE, std::string(), currentLine_, currentColumn_);
    }

    char c = peek(); // 读取当前指向字符

    // 起始状态
    // 进入识别标识符/关键字状态
    if (isalpha(c) || c == '_')
    {
        return readIdentifierOrKeyword();
    }else if(c=='"'){
        return readString();
    }
    else if (isdigit(c))
    {
        return readNumber();
    }
    else if (isOperator(peek()))
    {
        return readOperator();
    }

    else if (isSymbol(sourceCode_[currentPosition_]))
    {
        return readSymbol();
    }
    else
    {
        Token error(TokenType::UNKNOW, std::string(1, c), currentLine_, currentColumn_);
        advance();
        return error;
    }
}

// 默认读取当前指向字符
char Lexer::peek(size_t ahead) const
{
    size_t pos = currentPosition_ + ahead;
    return pos < sourceCode_.length() ? sourceCode_[pos] : '\0';
}

// 读取当前pos指向位置字符，并后移一位
char Lexer::advance()
{
    if (currentPosition_ >= sourceCode_.length())
        return '\0';

    char c = sourceCode_[currentPosition_++];
    if (c == '\n')
    {
        currentLine_++;
        currentColumn_ = 1;
    }
    else
    {
        currentColumn_++;
    }
    return c;
}

Token Lexer::readIdentifierOrKeyword()
{
    int startLine = currentLine_;
    int startColumn = currentColumn_;

    std::string identifier;
    while (std::isalpha(peek()) || peek() == '_')
    {
        identifier += advance();
    }

    if (keywords_.find(identifier) != keywords_.end())
    {
        return Token(keywords_[identifier], identifier, startLine, startColumn);
    }
    return Token(TokenType::IDENTIFIER, identifier, startLine, startColumn);
}

Token Lexer::readNumber()
{
    int startLine = currentLine_;
    int startColumn = currentColumn_;
    std::string num;
    bool isFloat = false;
    bool isHex = false;
    bool isScientific = false;
    bool error = false;
    std::string errorMsg;
    TokenType type;

    // 数字类型检测
    if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X'))
    {
        // 十六进制处理
        num += advance();
        num += advance();
        isHex = true;

        bool hasDigits = false;
        while (isxdigit(peek()))
        {
            num += advance();
            hasDigits = true;
        }

        if (!hasDigits)
        {
            error = true;
            errorMsg = "Hexadecimal number requires at least one digit";
        }
    }
    else
    {
        // 十进制处理（可能包含浮点数）
        while (std::isdigit(peek()) || peek() == '.')
        {
            if (peek() == '.')
            {
                if (isFloat)
                {
                    error = true;
                    errorMsg = "Multiple decimal points in number";
                }
                isFloat = true;
                num += advance();

                // 小数点后必须有数字
                if (!std::isdigit(peek()))
                {
                    error = true;
                    errorMsg = "Decimal point must be followed by digit";
                }
            }
            else
            {
                num += advance();
            }
        }

        // 科学计数法处理
        if (!isHex && (peek() == 'e') || peek() == 'E')
        {
            isScientific = true;
            isFloat = true;
            ;
            num += advance(); // e/E

            // 处理符号
            if (peek() == '+' || peek() == '-')
            {
                num += advance();
            }

            // 指数部分必须有数字
            bool hasExponentDigits = false;
            while (std::isdigit(peek()))
            {
                num += advance();
                hasExponentDigits = true;
            }

            if (!hasExponentDigits)
            {
                error = true;
                errorMsg = "Exponent requires at least one digit";
            }
        }
    }

    // 后缀处理（可扩展类型后缀，如f/F、u/U等）
    if (!isHex)
    {
        while (std::isalpha(peek()))
        {
            char c = advance();
            num += c;
            if (c == 'f' || c == 'F' || c == 'l' || c == 'L')
            {
                isFloat = true;
            }
            else
            {
                error = true;
                errorMsg = "Invalid numeric suffix '" + std::string(1, c) + "'";
            }
        }
    }

    // 错误处理
    if (error)
    {
        // 读取到非法字符时继续前进知道数字结束
        while (std::isalnum(peek()) || peek() == '.')
        {
            num += advance();
        }
        return Token(TokenType::ERROR, errorMsg, startLine, startColumn);
    }

    // 验证数字格式
    if (isHex && isFloat)
    {
        return Token(TokenType::ERROR, "Hexadecimal cannot have decimal point,startLine,startColumn", startLine, startColumn);
    }
    if (num.find('.') != std::string::npos && !isFloat)
    {
        return Token(TokenType::ERROR, "Invalid number format", startLine, startColumn);
    }

    return Token{isFloat ? TokenType::CONSTANT_FLOAT : TokenType::CONSTANT_INTEGER, num, startLine, startColumn};
}

Token Lexer::readString()
{
    int startLine = currentLine_;
    int startColumn = currentColumn_;
    
    std::string value;
    value+=peek();
    advance();
    while (peek() != '"' && currentPosition_<=sourceCode_.length()) {
        if (peek() == '\\') { // 处理转义
            advance();
            // 只允许\n转义
            if (peek() != 'n') {
                return Token(TokenType::ERROR, "Invalid escape sequence",startLine,startColumn);
            }
            advance();
        }
        value+=peek();
        advance();
    }
    
    if (currentPosition_>=sourceCode_.length()) return Token(TokenType::ERROR,"Unclosed string",startLine,startColumn);
    value+=peek();
    advance(); // 跳过闭合的"
    return Token(TokenType::CONSTANT_STRING,value,startLine,startColumn);
}

Token Lexer::readOperator()
{
    int startLine = currentLine_;
    int startColumn = currentColumn_;

    std::string operatorString;
    std::string first(1, peek());

    // 处理三字运算符
    std::string third(first);
    third += peek(1);
    third += peek(2);
    if (operators_.find(third) != operators_.end())
    {
        advance();
        advance();
        advance();
        return Token(operators_[third], third, startLine, startColumn);
    }

    // 处理双字符运算符
    std::string second(first);
    second += peek(1);
    if (operators_.find(second) != operators_.end())
    {
        advance();
        advance();
        return Token(operators_[second], second, startLine, startColumn);
    }

    // 处理单字符运算符
    if (operators_.find(first) != operators_.end())
    {
        advance();
        return Token(operators_[first], first, startLine, startColumn);
    }

    return Token(TokenType::UNKNOW,first,startLine,startColumn);
}

Token Lexer::readSymbol()
{
    int startLine = currentLine_;
    int startColumn = currentColumn_;

    std::string symbol;
    symbol = sourceCode_[currentPosition_++];

    return Token(punctuations_[symbol], symbol, startLine, startColumn);
}

void Lexer::skipWhitespaceOrComments()
{
    while (true)
    {
        char c = peek();
        if (c == '/')
        {
            if (peek(1) == '/')
            {
                while (peek() != '\n')
                    advance();
            }
            else if (peek(1) == '*')
            {
                advance();
                advance();

                while (!(peek() == '*' && peek(1) == '/'))
                {
                    advance();
                }
                advance();
                advance();
            }
        }
        else if(isspace(peek()))
        {
            while (std::isspace(peek()))
            {
                advance();
            }
        }else{
            break;
        }
    }
}

bool Lexer::isOperator(char c)
{
    static const std::string operators = "+-*/=<>!&|?:";
    return operators.find(c) != std::string::npos;
}

bool Lexer::isSymbol(char c)
{
    static const std::string symbols = "{}();,";
    return symbols.find(c) != std::string::npos;
}

std::ostream &operator<<(std::ostream &os, const Token &token)
{
    os << std::left;
    // os<<std::setw(30)<<"TokenType:"<<std::setw(10)<<"value:"<<std::setw(10)<<"line:"<<std::setw(10)<<"colume:"<<std::endl;
    os << std::setw(30) << Token::tokenTypeToString(token.tokenType_) << std::setw(10) << token.value_ << std::setw(10) << token.line_ << std::setw(10) << token.colume_ << std::endl;
    return os;
}
