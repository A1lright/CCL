#include "parser.h"

Parser::Parser(const std::vector<Token> &tokens):tokens_(tokens),currentPos_(0){
    if(!tokens_.empty()){
        currentToken_=tokens_[0];
    }
}

std::unique_ptr<AST::Program> Parser::parse()
{
    auto program=std::make_unique<AST::Program>();

    try{
        while(currentPos_<tokens_.size()-1){
            if(checkDeclaration()){
                program->declarations_.push_back(parseDeclaration());
            }else{
                synchronize();
            }
        }
    }catch(const ParseError&e){
        reportError(e);
    }

    return program;
}

void Parser::advance()
{
    if (currentPos_ < tokens_.size() - 1)
    {
        currentToken_ = tokens_[currentPos_++];
    }
    else
    {
        currentToken_.tokenType_ = TokenType::END_OF_FILE;
    }
}

bool Parser::match(TokenType type)
{
    if(currentToken_.tokenType_==type){
        advance();
        return true;
    }
    
    return false;
}

Token Parser::consume(TokenType type, const std::string &message)
{
    if(currentToken_.tokenType_==type){
        Token consumed=currentToken_;
        advance();
        return consumed;
    }
    throw error(message);
}

Parser::ParseError::ParseError(const std::string &msg, int ln, int col): std::runtime_error(msg), line_(ln), column_(col) {}


Parser::ParseError Parser::error(const std::string &message)
{
        return ParseError(message, currentToken_.line_, currentToken_.colume_);
}

void Parser::reportError(const ParseError &e)
{
    std::cerr << "[Parse Error] Line " << e.line_ << ":" << e.column_
                  << " - " << e.what() << std::endl;
}

void Parser::synchronize()
{
    advance();

    while(currentPos_<tokens_.size()){
        
    }
}

bool Parser::checkDeclaration() const
{
    //TODO:: 还需支持自定义的结构体等
    return currentToken_.tokenType_==TokenType::KEYWORD_INT||
            currentToken_.tokenType_==TokenType::KEYWORD_CHAR||
            currentToken_.tokenType_==TokenType::KEYWORD_VOID;
}

std::unique_ptr<AST::Declaration> Parser::parseDeclaration()
{
    auto type=parseType();
    advance();
    if(match(TokenType::IDENTIFIER)){
        //函数声明
        if(currentToken_.tokenType_==TokenType::PUNCTUATION_LEFT_PAREN){
            return parseFunctionDeclaration(std::move(type));
        }

        //声明变量(TODO)
        else{
            throw error("Variable declaration not implemented");
        }
    }
    throw error("Expected identifier after type specifier");
}

std::unique_ptr<AST::TypeSpecifier> Parser::parseType()
{
    auto type=std::make_unique<AST::TypeSpecifier>();
    type->typeName_=currentToken_.value_;
    advance();
    
    return type;
}

std::unique_ptr<AST::FunctionDeclaration> Parser::parseFunctionDeclaration(std::unique_ptr<AST::TypeSpecifier> returnType)
{
    auto func=std::make_unique<AST::FunctionDeclaration>();
    func->returnType_=std::move(returnType);
    func->name_=tokens_[currentPos_-1].value_;

    //参数列表
    consume(TokenType::PUNCTUATION_LEFT_PAREN,"Expected '(' after function name");
    func->params_= parseParameterList();
    consume(TokenType::PUNCTUATION_RIGHT_PAREN,"Expected ')' after parameters");

    //函数体
    func->body_=parseBlockStatement();
    return func;
}

std::vector<std::unique_ptr<AST::Parameter>> Parser::parseParameterList()
{
    std::vector<std::unique_ptr<AST::Parameter>>params;
    if(tokens_[currentPos_].tokenType_!=TokenType::PUNCTUATION_RIGHT_PAREN){
        do{
            params.push_back(parseParameter());
        }while(match(TokenType::PUNCTUATION_COMMA));
    }

    return params;
}

std::unique_ptr<AST::Parameter> Parser::parseParameter()
{
    auto param=std::make_unique<AST::Parameter>();
    param->type_=parseType();
    param->name_=currentToken_.value_;
    consume(TokenType::IDENTIFIER,"Expected parameter name");
    return param;
}

std::unique_ptr<AST::Statement> Parser::ParseStatement()
{
    if(match(TokenType::KEYWORD_IF))return parseIfStatement();
    if(match(TokenType::KEYWORD_WHILE))return parseWhileStatement();
    if(match(TokenType::KEYWORD_RETURN))return parseReturnStatement();
    if(match(TokenType::PUNCTUATION_LEFT_BRACE))return parseBlockStatement();
    return parseExpressionStatement();
}

std::unique_ptr<AST::CompoundStatement> Parser::parseBlockStatement()
{
    auto block=std::make_unique<AST::CompoundStatement>();
    while(!check(TokenType::PUNCTUATION_RIGHT_BRACE)){
        block->statements_.push_back(ParseStatement());
    }
    consume(TokenType::PUNCTUATION_RIGHT_BRACE,"Expected '}' to close block");
    return block;
}

std::unique_ptr<AST::IfStatement> Parser::parseIfStatement()
{
    auto ifStmt=std::make_unique<AST::IfStatement>();
    consume(TokenType::PUNCTUATION_LEFT_PAREN,"Expected '(' after 'if'");
    ifStmt->condition_=parseExpression();
    consume(TokenType::PUNCTUATION_RIGHT_BRACE,"Expected ')' after condition");

    ifStmt->thenBranch_=ParseStatement();

    if(match(TokenType::KEYWORD_ELSE)){
        ifStmt->elseBranch_=ParseStatement();
    }

    return ifStmt;
}

std::unique_ptr<AST::WhileStatement> Parser::parseWhileStatement()
{
    auto whileStmt=std::make_unique<AST::WhileStatement>();

    consume(TokenType::PUNCTUATION_LEFT_PAREN,"Expected '(' after 'while'");
    whileStmt->condition_=parseExpression();
    consume(TokenType::PUNCTUATION_RIGHT_PAREN,"Expected ')' after condition");

    whileStmt->body_=ParseStatement();
    return whileStmt;
}

std::unique_ptr<AST::ReturnStatement> Parser::parseReturnStatement()
{
    auto returnStmt=std::make_unique<AST::ReturnStatement>();
    if(!check(TokenType::PUNCTUATION_SEMICOLON)){
        returnStmt->expression_=parseExpression();
    }

    consume(TokenType::PUNCTUATION_SEMICOLON,"Expected ';' after return");
    return returnStmt;
}

//表达式语句
std::unique_ptr<AST::ExpressionStatement> Parser::parseExpressionStatement()
{
    auto stmt = std::make_unique<AST::ExpressionStatement>();
    stmt->expression_=parseExpression();
    consume(TokenType::PUNCTUATION_SEMICOLON,"Expected ';' after expression");
    return stmt;
}

std::unique_ptr<AST::Expression> Parser::parseExpression()
{
    return parseAssignment();
}

std::unique_ptr<AST::Expression> Parser::parseAssignment()
{
    auto expr=parseEquality();

    if(match(TokenType::OPERATOR_ASSIGN)){
        auto assign=std::make_unique<AST::AssignmentExpression>();
        assign->lhs_=std::move(expr);
        assign->rhs_=parseAssignment();
        return assign;
    }

    return expr;
}

std::unique_ptr<AST::Expression> Parser::parseEquality()
{
    auto expr=parseRelational();

    while(true){
        int currPrec=getCurrentPrecedence();
        if(match(TokenType::OPERATOR_EQUAL)){
            expr=makeBinary(std::move(expr),"==",currPrec);
        }else if(match(TokenType::OPERATOR_NOT_EQUAL)){
            expr=makeBinary(std::move(expr),"!=",currPrec);
        }else{
            break;
        }
    }
    return expr;
}

std::unique_ptr<AST::Expression> Parser::parseRelational()
{
    auto expr=parseAdditive();

    while(true){
        int currPrec=getCurrentPrecedence();
        if(currPrec!=5)break; //只处理优先级5的
        if(match(TokenType::OPERATOR_LESS)){
            expr=makeBinary(std::move(expr),"<",currPrec);
        }else if(match(TokenType::OPERATOR_LESS_EQUAL)){
            expr=makeBinary(std::move(expr),"<=",currPrec);
        }else if(match(TokenType::OPERATOR_GREATER)){
            expr=makeBinary(std::move(expr),">",currPrec);
        }else if(match(TokenType::OPERATOR_GREATER_EQUAL)){
            expr=makeBinary(std::move(expr),">=",currPrec);
        }else{
            break;
        }
    }

    return expr;
}

std::unique_ptr<AST::Expression> Parser::parseAdditive()
{
    auto expr=parseMultiplicative();
    while(true){
        int currPrec=getCurrentPrecedence();
        if(match(TokenType::OPERATOR_PLUS)){
            expr=makeBinary(std::move(expr),"+",currPrec);
        }else if(match(TokenType::OPERATOR_MINUS)){
            expr=makeBinary(std::move(expr),"-",currPrec);
        }else{
            break;
        }
    }
    return expr;
}

std::unique_ptr<AST::Expression> Parser::parseMultiplicative()
{
    auto expr=parsePrimary();

    while(true){
        int currPrec=getCurrentPrecedence();
        if(match(TokenType::OPERATOR_MULTIPLY)){
            expr=makeBinary(std::move(expr),"*",currPrec);
        }else if(match(TokenType::OPERATOR_DIVIDE)){
            expr=makeBinary(std::move(expr),"/",currPrec);
        }else{
            break;
        }
    }

    return expr;
}

std::unique_ptr<AST::Expression> Parser::parsePrimary()
{
    if(match(TokenType::CONSTANT_INTEGER)){
        auto num=std::make_unique<AST::NumberLiteral>();
        num->token_=tokens_[currentPos_-1];
        return num;
    }

    if(match(TokenType::IDENTIFIER)){
        //函数调用或变量引用
        if(check(TokenType::PUNCTUATION_LEFT_PAREN)){
            return parseFunctionCall();
        }
        auto ident=std::make_unique<AST::Identifier>();
        ident->token_=tokens_[currentPos_-1];
        return ident;
    }

    if(match(TokenType::PUNCTUATION_LEFT_PAREN)){
        auto expr=parseExpression();
        consume(TokenType::PUNCTUATION_RIGHT_PAREN,"Expected ')' after expression");
        return expr;
    }

    throw error("Expected primary expression");
}

std::unique_ptr<AST::FunctionCall> Parser::parseFunctionCall()
{
    auto call=std::make_unique<AST::FunctionCall>();
    call->callee_=tokens_[currentPos_-1].value_;

    consume(TokenType::PUNCTUATION_LEFT_PAREN,"Expected '(' after function name");
    if(!check(TokenType::PUNCTUATION_RIGHT_PAREN)){
        do{
             call->arguments_.push_back(parseExpression());
        }while(match(TokenType::PUNCTUATION_COMMA));
    }
    consume(TokenType::PUNCTUATION_RIGHT_PAREN,"Expected ')' after arguments");
    return call;
}

std::unique_ptr<AST::BinaryExpression> Parser::makeBinary(std::unique_ptr<AST::Expression> lhs, const std::string &op,int precedence)
{
    auto expr=std::make_unique<AST::BinaryExpression>();
    expr->lhs_=std::move(lhs);
    expr->op=op;
    expr->rhs_=parseRightOperand(precedence); //根据优先级解析右操作数
    return expr;
}

int Parser::getCurrentPrecedence() const
{
    static std::unordered_map<std::string,int>precedence={
        {"=",1},{"==",3},{"!=",3},
        {"<",4},{"<=",4},{">",4},{">=",4},
        {"+",5},{"-",5},
        {"*",6},{"/",6}
    };
    auto it=precedence.find(currentToken_.value_);
    return it!=precedence.end()? it->second:0;
}

std::unique_ptr<AST::Expression> Parser::parseRightOperand(int precedence)
{
    //根据优先层级调用对应解析函数
    switch (precedence)
    {
    case 1:
        return parseAssignment();
    case 3:
        return parseEquality();
    case 4:
        return parseRelational();
    case 5:
        return parseAdditive();
    case 6:
        return parseMultiplicative();
    
    default:
        return parsePrimary();
    }
}
