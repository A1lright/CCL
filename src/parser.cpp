#include "parser.h"

Parser::Parser(std::vector<Token> &tokens, SymbolTable &symbolTable) : tokens_(tokens), current_(0), token_(tokens[0]), symbolTable_(symbolTable)
{
}

std::unique_ptr<CompUnit> Parser::parseCompUnit()
{
    std::unique_ptr<CompUnit> compUnit = std::make_unique<CompUnit>();

    // 解析全局声明{Decl}
    while (check(TokenType::KEYWORD_CONST) || (check(TokenType::KEYWORD_INT) && peek(2).tokenType_ != TokenType::PUNCTUATION_LEFT_PAREN))
    {
        compUnit->decls_.push_back(parseDecl());
    }

    // 解析函数定义{FuncDef}
    while ((check(TokenType::KEYWORD_INT) || check(TokenType::KEYWORD_VOID)) && (peek(2).tokenType_ == TokenType::PUNCTUATION_LEFT_PAREN) && (peek(1).tokenType_ == TokenType::IDENTIFIER))
    {
        compUnit->funcDefs_.push_back(parseFuncDef());
    }

    // 解析主函数
    compUnit->mainfuncDef_ = parseMainFuncDef();

    return compUnit;
}

std::unique_ptr<Decl> Parser::parseDecl()
{
    if (check(TokenType::KEYWORD_CONST))
    { // 判断是否为常量声明
        return parseConstDecl();
    }
    else
    {
        return parseVarDecl(); // 默认为变量声明
    }
}

std::unique_ptr<ConstDecl> Parser::parseConstDecl()
{
    auto const_decl = std::make_unique<ConstDecl>();

    advance(); // consume "const"
    // 解析BType（目前只有int）
    const_decl->bType_ = parseBType();

    // 解析多个ConstDef，逗号分隔
    do
    {
        auto const_def = parseConstDef();

        const_decl->constDefs_.push_back(std::move(const_def));
    } while (match(TokenType::PUNCTUATION_COMMA));

    advance(); // consume ";"

    return const_decl;
}

std::unique_ptr<ConstDef> Parser::parseConstDef()
{
    auto const_def = std::make_unique<ConstDef>();
    const_def->name_ = advance().value_;

    // 解析数组维度 [ConstExp]
    while (match(TokenType::PUNCTUATION_LEFT_BRACKET))
    {
        const_def->dimensions_.push_back(parseConstExp());
        advance();
    }

    // 解析初始化值
    advance();

    const_def->initVal_ = parseConstInitVal();

    const_def->hasInit = true; // hasInit为多余的,必有初始化值

    return const_def;
}

std::unique_ptr<VarDecl> Parser::parseVarDecl()
{
    auto var_decl = std::make_unique<VarDecl>();
    var_decl->bType_ = parseBType();

    // 解析多个VarDef，逗号分隔
    do
    {
        auto var_def = parseVarDef();

        var_decl->varDefs_.push_back(std::move(var_def));
    } while (match(TokenType::PUNCTUATION_COMMA));

    advance(); // 必须分号结尾

    return var_decl;
}

std::unique_ptr<VarDef> Parser::parseVarDef()
{
    auto var_def = std::make_unique<VarDef>();
    // 解析标识符
    var_def->name_ = advance().value_;

    // 解析数组维度[ConstExp]
    while (match(TokenType::PUNCTUATION_LEFT_BRACKET))
    {
        // 非法情况：int a[];需要排除
        var_def->constExps_.push_back(parseConstExp());
        advance();
    }

    // 解析可选的初始化值
    if (match(TokenType::OPERATOR_ASSIGN))
    {
        var_def->initVal_ = parseInitVal();
        var_def->hasInit = true;
    }
    else
    {
        var_def->hasInit = false;
    }

    return var_def;
}

std::unique_ptr<FuncDef> Parser::parseFuncDef()
{
    auto func_def = std::make_unique<FuncDef>();

    // 解析返回类型
    func_def->returnType_ = parseFuncType();

    // 解析函数名
    func_def->name_ = advance().value_;

    // 解析形参列表
    advance(); // consume "("
    std::vector<TokenType> paramTypes;
    if (!check(TokenType::PUNCTUATION_RIGHT_PAREN))
    {
        do
        {
            auto param = parseFuncParam();

            // 添加参数类型信息
            paramTypes.push_back(param->bType_->typeName_ == "int" ? TokenType::KEYWORD_INT : TokenType::UNKNOW);

            func_def->params_.push_back(std::move(param));
        } while (match(TokenType::PUNCTUATION_COMMA));
    }
    advance(); // consume ")"

    // 解析函数体
    func_def->body_ = parseBlock();
    return func_def;
}

std::unique_ptr<MainFuncDef> Parser::parseMainFuncDef()
{
    auto main_func_def = std::make_unique<MainFuncDef>();
    advance(); // consume "int"
    advance(); // consume "main"
    advance(); // consume "("
    advance(); // consume ")"

    // 将 main 函数符号添加到全局符号表

    main_func_def->body_ = parseBlock();

    return main_func_def;
}

std::unique_ptr<FuncParam> Parser::parseFuncParam()
{
    auto param = std::make_unique<FuncParam>();
    param->bType_ = parseBType();
    param->name_ = advance().value_;

    // 处理数组类型（如 int a[] 或 int a[2][3]）
    while (match(TokenType::PUNCTUATION_LEFT_BRACKET))
    {
        if (match(TokenType::PUNCTUATION_RIGHT_BRACKET))
        {
            param->dimSizes_.push_back(nullptr); // 第一维空缺
        }
        else
        {
            param->dimSizes_.push_back(parseConstExp());
            advance();
        }
    }
    return param;
}

std::unique_ptr<Block> Parser::parseBlock()
{
    auto block = std::make_unique<Block>();
    if (check(TokenType::PUNCTUATION_LEFT_BRACE))
    {
        advance(); // consume '{'
        while (!check(TokenType::PUNCTUATION_RIGHT_BRACE))
        {
            block->items_.push_back(parseBlockItem());
        }
    }

    advance(); // consume '}'
    return block;
}

std::unique_ptr<BlockItem> Parser::parseBlockItem()
{
    auto block_item = std::make_unique<BlockItem>();

    // 判断当前Token是否为声明（const或int开头）
    if (check(TokenType::KEYWORD_CONST) || check(TokenType::KEYWORD_INT))
    {
        // 解析声明（Decl）
        auto decl = parseDecl();
        block_item->item_ = std::move(decl);
    }
    else
    {
        // 解析语句（Stmt）
        auto stmt = parseStmt();
        block_item->item_ = std::move(stmt);
    }

    return block_item;
}

std::unique_ptr<Stmt> Parser::parseStmt()
{
    if (check(TokenType::IDENTIFIER) && peek(1).tokenType_ == TokenType::OPERATOR_ASSIGN && peek(2).tokenType_ == TokenType::KEYWORD_GETINT)
    {
        return parseGetintStmt();
    }

    else if (check(TokenType::PUNCTUATION_LEFT_BRACE))
    {
        return parseBlock();
    }
    else if (match(TokenType::KEYWORD_IF))
    {
        return parseIfStmt();
    }
    else if (match(TokenType::KEYWORD_WHILE))
    {
        return parseWhileStmt();
    }
    else if (match(TokenType::KEYWORD_RETURN))
    {
        return parseReturnStmt();
    }
    else if (check(TokenType::KEYWORD_PRINTF))
    {
        return parsePrintfStmt();
    }
    else if (isAssignStmt())
    {
        return parseAssignStmt();
    }
    else
    {
        return parseExpStmt();
    }
}

std::unique_ptr<IfStmt> Parser::parseIfStmt()
{
    auto if_stmt = std::make_unique<IfStmt>();
    advance(); // consume "if"
    if_stmt->cond_ = parseLogicalOrExp();
    advance(); // consume ")"

    auto stmt = parseStmt();
    if_stmt->thenBranch_ = std::move(stmt);

    if (match(TokenType::KEYWORD_ELSE))
    {
        stmt = parseStmt();
        if_stmt->elseBranch_ = std::move(stmt);
    }
    return if_stmt;
}

std::unique_ptr<WhileStmt> Parser::parseWhileStmt()
{
    auto while_stmt = std::make_unique<WhileStmt>();
    advance();
    while_stmt->cond_ = parseLogicalOrExp();
    advance();
    while_stmt->body_ = parseStmt();
    return while_stmt;
}

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt()
{
    auto return_stmt = std::make_unique<ReturnStmt>();

    // 如果下一个 token 是分号，则说明没有返回值
    if (check(TokenType::PUNCTUATION_SEMICOLON))
    {
        return_stmt->exp_ = nullptr;
    }
    else
    {
        // 否则解析返回表达式
        return_stmt->exp_ = parseExp();
    }
    // 消费分号
    advance();
    return return_stmt;
}

std::unique_ptr<AssignStmt> Parser::parseAssignStmt()
{
    auto assignment = std::make_unique<AssignStmt>();
    assignment->lval_ = parseLVal();
    advance(); // consume =
    assignment->exp_ = parseExp();
    advance();
    return assignment;
}

std::unique_ptr<Exp> Parser::parseExp()
{
    return parseAddExp();
}

std::unique_ptr<LOrExp> Parser::parseLogicalOrExp()
{
    auto exp = std::make_unique<LOrExp>();
    exp->elements_.push_back(parseLogicalAndExp());

    while (match(TokenType::OPERATOR_LOGICAL_OR))
    {
        exp->elements_.push_back(TokenType::OPERATOR_LOGICAL_OR);
        exp->elements_.push_back(parseLogicalAndExp());
    }
    return exp;
}

std::unique_ptr<Exp> Parser::parseLogicalAndExp()
{
    auto exp = std::make_unique<LAndExp>();
    exp->elements_.push_back(parseEqExp());

    while (match(TokenType::OPERATOR_LOGICAL_AND))
    {
        exp->elements_.push_back(TokenType::OPERATOR_LOGICAL_AND);
        exp->elements_.push_back(parseEqExp());
    }
    return exp;
}

std::unique_ptr<Exp> Parser::parseEqExp()
{
    auto exp = std::make_unique<EqExp>();
    exp->elements_.push_back(parseRelExp());

    while (true)
    {
        if (match(TokenType::OPERATOR_EQUAL))
        {
            exp->elements_.push_back(TokenType::OPERATOR_EQUAL);
            exp->elements_.push_back(parseRelExp());
        }
        else if (match(TokenType::OPERATOR_NOT_EQUAL))
        {
            exp->elements_.push_back(TokenType::OPERATOR_NOT_EQUAL);
            exp->elements_.push_back(parseRelExp());
        }
        else
        {
            break;
        }
    }
    return exp;
}

std::unique_ptr<Exp> Parser::parseRelExp()
{
    auto exp = std::make_unique<RelExp>();
    exp->elements_.push_back(parseAddExp());

    while (true)
    {
        if (match(TokenType::OPERATOR_LESS))
        {
            exp->elements_.push_back(TokenType::OPERATOR_LESS);
            exp->elements_.push_back(parseAddExp());
        }
        else if (match(TokenType::OPERATOR_GREATER))
        {
            exp->elements_.push_back(TokenType::OPERATOR_GREATER);
            exp->elements_.push_back(parseAddExp());
        }
        else if (match(TokenType::OPERATOR_LESS_EQUAL))
        {
            exp->elements_.push_back(TokenType::OPERATOR_LESS_EQUAL);
            exp->elements_.push_back(parseAddExp());
        }
        else if (match(TokenType::OPERATOR_GREATER_EQUAL))
        {
            exp->elements_.push_back(TokenType::OPERATOR_GREATER_EQUAL);
            exp->elements_.push_back(parseAddExp());
        }
        else
        {
            break;
        }
    }
    return exp;
}

std::unique_ptr<AddExp> Parser::parseAddExp()
{
    auto exp = std::make_unique<AddExp>();

    // 第一个元素必须是MulExp
    exp->elements_.push_back(parseMulExp());

    // 后续处理运算符和操作数
    while (true)
    {
        if (match(TokenType::OPERATOR_PLUS))
        {
            exp->elements_.push_back(TokenType::OPERATOR_PLUS);
            exp->elements_.push_back(parseMulExp());
        }
        else if (match(TokenType::OPERATOR_MINUS))
        {
            exp->elements_.push_back(TokenType::OPERATOR_MINUS);
            exp->elements_.push_back(parseMulExp());
        }
        else
        {
            break;
        }
    }
    return exp;
}

std::unique_ptr<MulExp> Parser::parseMulExp()
{
    auto exp = std::make_unique<MulExp>();

    // 第一个元素必须是UnaryExp
    exp->elements_.push_back(parseUnaryExp());

    // 后续处理运算符和操作数
    while (true)
    {
        if (match(TokenType::OPERATOR_MULTIPLY))
        {
            exp->elements_.push_back(TokenType::OPERATOR_MULTIPLY);
            exp->elements_.push_back(parseUnaryExp());
        }
        else if (match(TokenType::OPERATOR_DIVIDE))
        {
            exp->elements_.push_back(TokenType::OPERATOR_DIVIDE);
            exp->elements_.push_back(parseUnaryExp());
        }
        else if (match(TokenType::OPERATOR_MODULO))
        {
            exp->elements_.push_back(TokenType::OPERATOR_MODULO);
            exp->elements_.push_back(parseUnaryExp());
        }
        else
        {
            break;
        }
    }
    return exp;
}

std::unique_ptr<Exp> Parser::parseUnaryExp()
{
    auto unary_exp = std::make_unique<UnaryExp>();
    // 1. 操作符
    if (match(TokenType::OPERATOR_PLUS))
    {
        unary_exp->op = UnaryExp::Op::Plus;
    }
    else if (match(TokenType::OPERATOR_MINUS))
    {
        unary_exp->op = UnaryExp::Op::Minus;
    }
    else if (match(TokenType::OPERATOR_LOGICAL_NOT))
    {
        unary_exp->op = UnaryExp::Op::Not;
    }

    // 2. operand_ 现在可能是 CallExp、UnaryExp 或 PrimaryExp
    if (check(TokenType::IDENTIFIER) && peek(1).tokenType_ == TokenType::PUNCTUATION_LEFT_PAREN)
    {
        unary_exp->operand_ = parseCallExp();
    }
    else if (check(TokenType::OPERATOR_PLUS) || check(TokenType::OPERATOR_MINUS) || check(TokenType::OPERATOR_LOGICAL_NOT))
    {
        // 连续的一元运算符
        unary_exp->operand_ = parseUnaryExp();
    }
    else
    {
        unary_exp->operand_ = parsePrimaryExp();
    }

    return unary_exp;
}

std::unique_ptr<PrimaryExp> Parser::parsePrimaryExp()
{
    auto primary_exp = std::make_unique<PrimaryExp>();
    if (check(TokenType::PUNCTUATION_LEFT_PAREN))
    {
        advance();
        primary_exp->operand_ = parseExp();
    }

    if (token_.tokenType_ == TokenType::IDENTIFIER)
    {
        primary_exp->operand_ = parseLVal();
    }
    else
    {
        primary_exp->operand_ = parseNumber();
    }
    return primary_exp;
}

std::unique_ptr<LVal> Parser::parseLVal()
{
    auto lVal = std::make_unique<LVal>();
    lVal->name_ = token_.value_;

    advance(); // consume IDENTIFIER

    while (match(TokenType::PUNCTUATION_LEFT_BRACKET))
    {
        lVal->indices_.push_back(parseExp());
        advance();
    }
    return lVal;
}

std::unique_ptr<Number> Parser::parseNumber()
{
    auto num = std::make_unique<Number>();
    num->value_ = std::stoi(token_.value_);
    advance();
    return num;
}

std::unique_ptr<ConstInitVal> Parser::parseConstInitVal()
{
    auto init_val = std::make_unique<ConstInitVal>();
    if (match(TokenType::PUNCTUATION_LEFT_BRACE))
    {
        std::vector<std::unique_ptr<ConstInitVal>> elements;
        if (!check(TokenType::PUNCTUATION_RIGHT_BRACE))
        {
            do
            {
                elements.push_back(parseConstInitVal());
            } while (match(TokenType::PUNCTUATION_COMMA));
        }
        advance();
        init_val->value_ = std::move(elements);
    }
    else
    {
        init_val->value_ = parseConstExp();
    }
    return init_val;
}

std::unique_ptr<InitVal> Parser::parseInitVal()
{
    auto init_val = std::make_unique<InitVal>();

    // 情况1：初始化值为表达式（非数组初始化）
    if (!check(TokenType::PUNCTUATION_LEFT_BRACE))
    {
        init_val->value_ = parseExp(); // 解析单个表达式
        return init_val;
    }

    // 情况2：初始化值为数组初始化列表（{ ... }）
    advance(); // consume '{'

    std::vector<std::unique_ptr<InitVal>> elements;

    // 处理可能的空列表（例如 int a[2] = {};）
    if (!check(TokenType::PUNCTUATION_RIGHT_BRACE))
    {
        do
        {
            elements.push_back(parseInitVal()); // 递归解析子初始化值
        } while (match(TokenType::PUNCTUATION_COMMA)); // 逗号分隔多个元素

        // 检查尾部逗号（例如 {1, 2,}）
        if (previous().tokenType_ == TokenType::PUNCTUATION_COMMA)
        {
        }
    }

    advance();                              // consume '}'
    init_val->value_ = std::move(elements); // 保存嵌套的初始化列表
    return init_val;
}

std::unique_ptr<FuncType> Parser::parseFuncType()
{
    auto func_type = std::make_unique<FuncType>();
    func_type->typeName_ = peek().value_;
    advance();
    return func_type;
}

std::unique_ptr<BType> Parser::parseBType()
{
    auto bType = std::make_unique<BType>();
    bType->typeName_ = advance().value_; //  // 目前只有int
    return bType;
}

std::unique_ptr<Exp> Parser::parseConstExp()
{
    return parseAddExp();
}

std::unique_ptr<Stmt> Parser::parseExpStmt()
{
    auto exp_stmt = std::make_unique<ExpStmt>();
    exp_stmt->exp_ = parseExp();
    advance(); // consume ";"
    return exp_stmt;
}

std::unique_ptr<CallExp> Parser::parseCallExp()
{
    auto call_exp = std::make_unique<CallExp>();
    call_exp->funcName = advance().value_;

    advance(); // consume "("

    // 解析参数列表（可能为空）
    if (!check(TokenType::PUNCTUATION_RIGHT_PAREN))
    {
        do
        {
            call_exp->args_.push_back(parseExp());
        } while (match(TokenType::PUNCTUATION_COMMA));
    }

    advance(); // consume ")"
    return call_exp;
}

bool Parser::isAtEnd()
{
    return current_ >= tokens_.size();
}

Token Parser::previous()
{
    return current_ != 0 ? tokens_[current_ - 1] : tokens_[0];
}

bool Parser::isAssignStmt()
{
    size_t i = 0;
    while (peek(i).tokenType_ != TokenType::PUNCTUATION_SEMICOLON)
    {
        if (peek(i).tokenType_ == TokenType::OPERATOR_ASSIGN)
        {
            return true;
        }
        i++;
    }
    return false;
}

std::unique_ptr<IOStmt> Parser::parsePrintfStmt()
{
    auto stmt = std::make_unique<IOStmt>();
    stmt->kind = IOStmt::IOKind::Printf;
    advance(); // consume "printf"
    advance(); // consume "("

    // 解析格式字符串
    if (!check(TokenType::CONSTANT_STRING))
    {
    }
    stmt->formatString_ = token_.value_;
    advance(); // consume format string

    // 解析参数列表
    if (match(TokenType::PUNCTUATION_COMMA))
    {
        do
        {
            stmt->args_.push_back(parseExp());
        } while (match(TokenType::PUNCTUATION_COMMA));
    }

    advance(); // consume ")"
    advance(); // consume ";"
    return stmt;
}

std::unique_ptr<IOStmt> Parser::parseGetintStmt()
{
    auto stmt = std::make_unique<IOStmt>();
    stmt->kind = IOStmt::IOKind::Getint;
    stmt->target_ = parseLVal();
    advance(); // consume "="
    advance(); // consume "getint"
    advance(); // consume "("
    advance(); // consume ")"
    advance(); // consume ";"
    return stmt;
}

Token Parser::peek(size_t ahead) const
{
    if (current_ + ahead >= tokens_.size())
    {
        return Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return tokens_[current_ + ahead];
}

Token Parser::advance()
{
    Token token;
    if (current_ < tokens_.size())
    {
        token = tokens_[current_++];
        token_ = tokens_[current_];
    }
    else
    {
        token_ = Token(TokenType::END_OF_FILE, "", 0, 0);
    }
    return token;
}

bool Parser::match(TokenType type)
{
    if (token_.tokenType_ == type)
    {
        advance();
        return true;
    }

    return false;
}

bool Parser::check(TokenType type) const
{
    return type == token_.tokenType_;
}