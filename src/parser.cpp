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

    //consume(TokenType::KEYWORD_CONST, "Expect 'const'");

    advance();
    // 解析BType（目前只有int）
    const_decl->bType_ = parseBType();

    // 解析多个ConstDef，逗号分隔
    do
    {
        auto const_def = parseConstDef();
        // 创建变量符号对象
        auto var_symbol = std::make_unique<VariableSymbol>();
        var_symbol->name_ = const_def->name_;
        var_symbol->symbolType_ = SymbolType::VARIABLE;
        var_symbol->dataType_ = TokenType::KEYWORD_INT;
        var_symbol->lineDefined_ = token_.line_;
        var_symbol->columnDefined_ = 0;
        var_symbol->isConst_ = true;
        var_symbol->isArray_ = const_def->dimensions_.size() > 0;
        var_symbol->dimensions_ = std::vector<int>(); /* TODO: 填充数组维度信息 */
        var_symbol->initValue_.intValue = const_def->hasInit ? value_ : 0;

        // 将符号插入符号表
        if (!symbolTable_.addSymbol(std::move(var_symbol)))
        {
        }

        const_decl->constDefs_.push_back(std::move(const_def));
    } while (match(TokenType::PUNCTUATION_COMMA));

    //consume(TokenType::PUNCTUATION_SEMICOLON, "Expect ';' after const declaration");
    advance();

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

        // 创建变量符号对象
        auto var_symbol = std::make_unique<VariableSymbol>();
        var_symbol->name_ = var_def->name_;
        var_symbol->symbolType_ = SymbolType::VARIABLE;
        var_symbol->dataType_ = TokenType::KEYWORD_INT;
        var_symbol->lineDefined_ = token_.line_;
        var_symbol->columnDefined_ = 0;
        var_symbol->isConst_ = false;
        var_symbol->isArray_ = !var_def->constExps_.empty();
        var_symbol->dimensions_ = std::vector<int>(); /* TODO: 填充数组维度信息 */
        var_symbol->initValue_.intValue = var_def->hasInit ? value_ : 0;

        // 将符号插入符号表
        if (!symbolTable_.addSymbol(std::move(var_symbol)))
        {
        }

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
    advance();
    std::vector<TokenType> paramTypes;
    if (!check(TokenType::PUNCTUATION_RIGHT_PAREN))
    {
        do
        {
            auto param = parseFuncParam();

            // 添加参数类型信息
            paramTypes.push_back(param->bType_->typeName_ == "int" ? TokenType::KEYWORD_INT : TokenType::UNKNOW);

            auto paramSymbol = std::make_unique<VariableSymbol>();
            paramSymbol->name_ = param->name_;
            paramSymbol->symbolType_ = SymbolType::VARIABLE;
            paramSymbol->dataType_ = TokenType::KEYWORD_INT;
            paramSymbol->lineDefined_ = token_.line_;
            paramSymbol->columnDefined_ = 0;
            paramSymbol->isConst_ = false;
            paramSymbol->isArray_ = !param->dimSizes_.empty();
            paramSymbol->dimensions_ = std::vector<int>(); /* TODO: 填充数组维度信息 */

            if (!symbolTable_.addSymbol(std::move(paramSymbol)))
            {
            }

            func_def->params_.push_back(std::move(param));
        } while (match(TokenType::PUNCTUATION_COMMA));
    }
    advance();

    // 创建函数符号对象并添加到全局符号表
    auto funcSymbol = std::make_unique<FunctionSymbol>();
    funcSymbol->name_ = func_def->name_;
    funcSymbol->symbolType_ = SymbolType::FUNCTION;
    funcSymbol->dataType_ = func_def->returnType_->typeName_ == "int" ? TokenType::KEYWORD_INT : TokenType::UNKNOW;
    funcSymbol->lineDefined_ = token_.line_;
    funcSymbol->columnDefined_ = 0;
    funcSymbol->paramTypes_ = paramTypes;
    funcSymbol->hasReturn_ = true;

    if (!symbolTable_.addSymbol(std::move(funcSymbol)))
    {
    }

    // 解析函数体
    func_def->body_ = parseBlock();
    return func_def;
}

std::unique_ptr<MainFuncDef> Parser::parseMainFuncDef()
{
    auto main_func_def = std::make_unique<MainFuncDef>();
    advance();
    advance();
    advance();
    advance();

    // 将 main 函数符号添加到全局符号表
    // 创建一个函数符号对象，用于表示 main 函数
    auto mainFuncSymbol = std::make_unique<FunctionSymbol>();
    mainFuncSymbol->name_ = "main";
    mainFuncSymbol->symbolType_ = SymbolType::FUNCTION;
    mainFuncSymbol->dataType_ = TokenType::KEYWORD_INT; // main 返回 int
    mainFuncSymbol->lineDefined_ = token_.line_;
    mainFuncSymbol->columnDefined_ = 0;
    mainFuncSymbol->hasReturn_ = true; // main 必须有返回值
    mainFuncSymbol->paramTypes_ = std::vector<TokenType>();

    // 参数列表为空
    if (!symbolTable_.addSymbol(std::move(mainFuncSymbol)))
    {
    }

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
    // 进入局部作用域
    symbolTable_.enterScope();
    auto block = std::make_unique<Block>();
    if (check(TokenType::PUNCTUATION_LEFT_BRACE))
    {
        advance();
        while (!check(TokenType::PUNCTUATION_RIGHT_BRACE))
        {
            block->items_.push_back(parseBlockItem());
        }
    }
    // 退出局部作用域
    symbolTable_.addFunctionSymbol("main");
    symbolTable_.exitScope();
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
    if (match(TokenType::PUNCTUATION_LEFT_BRACE))
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
    else if (check(TokenType::IDENTIFIER) && peek(1).tokenType_ == TokenType::OPERATOR_ASSIGN && peek(2).tokenType_ == TokenType::KEYWORD_GETINT)
    {
        return parseGetintStmt();
    }
    else if (check(TokenType::IDENTIFIER) && peek(1).tokenType_ == TokenType::OPERATOR_ASSIGN) //?????
    {
        return parseAssignStmt();
    }
    else if (check(TokenType::KEYWORD_PRINTF))
    {
        return parsePrintfStmt();
    }
    else
    {
        return parseExprStmt();
    }
}

std::unique_ptr<IfStmt> Parser::parseIfStmt()
{
    auto if_stmt = std::make_unique<IfStmt>();
    advance();
    if_stmt->cond_ = parseExp();
    advance();
    if_stmt->elseBranch_ = parseStmt();

    if (match(TokenType::KEYWORD_ELSE))
    {
        if_stmt->elseBranch_ = parseStmt();
    }
    return if_stmt;
}

std::unique_ptr<WhileStmt> Parser::parseWhileStmt()
{
    auto while_stmt = std::make_unique<WhileStmt>();
    advance();
    while_stmt->cond_ = parseExp();
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
    advance();
    assignment->exp_ = parseExp();
    advance();
    return assignment;
}

std::unique_ptr<Exp> Parser::parseExp()
{
    return parseAddExp();
}

std::unique_ptr<Exp> Parser::parseLogicalOrExp()
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

// 类似地实现其他优先级层次（Equality, Relational, Additive, Multiplicative）

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
    // 情况2：函数调用（Ident '(' ... ）
    if (check(TokenType::IDENTIFIER) && peek(1).tokenType_ == TokenType::PUNCTUATION_LEFT_PAREN)
    {
        return parseCallExp();
    }

    // 情况1：单目运算符（+、-、！）
    if (check(TokenType::OPERATOR_PLUS) || check(TokenType::OPERATOR_MINUS) || check(TokenType::OPERATOR_LOGICAL_NOT))
    {
        if (match(TokenType::OPERATOR_PLUS))
        {
            unary_exp->op = UnaryExp::Op::Plus;
        }
        else if (match(TokenType::OPERATOR_MINUS))
        {
            unary_exp->op = UnaryExp::Op::Minus;
        }
        else
        {
            unary_exp->op = UnaryExp::Op::Not;
        }
    }

    unary_exp->operand_ = parsePrimaryExp();

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

void Parser::synchronize()
{
    advance(); // 跳过当前错误Token
    // 同步到下一个语句开始处
    while (!isAtEnd())
    {
        if (previous().tokenType_ == TokenType::PUNCTUATION_SEMICOLON)
            return;
        switch (peek().tokenType_)
        {
        case TokenType::KEYWORD_INT:
        case TokenType::KEYWORD_VOID:
        case TokenType::KEYWORD_IF:
        case TokenType::KEYWORD_WHILE:
        case TokenType::KEYWORD_RETURN:
            return;
        }
        advance();
    }
}

std::unique_ptr<LVal> Parser::parseLVal()
{
    auto lVal = std::make_unique<LVal>();
    lVal->name_ = token_.value_;

    // 在符号表中查找该标识符
    Symbol *sym = symbolTable_.lookup(lVal->name_);
    if (!sym)
    {
        
    }
    else
    {

    }
    advance();
    //advance();
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
    value_ = num->value_;
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
    advance();

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
            throw ParseError(previous().line_, "Trailing comma in initializer list");
        }
    }

    advance();
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
    bType->typeName_ = advance().value_;
    return bType;
}

std::unique_ptr<Exp> Parser::parseConstExp()
{
    return parseAddExp();
}

std::unique_ptr<Stmt> Parser::parseExprStmt()
{
    return std::unique_ptr<Stmt>();
}

std::unique_ptr<CallExp> Parser::parseCallExp()
{
    auto call_exp = std::make_unique<CallExp>();
    call_exp->funcName = advance().value_;

    // 在符号表中查找函数
    Symbol *funcSym = symbolTable_.lookup(call_exp->funcName);
    if (!funcSym || funcSym->symbolType_ != SymbolType::FUNCTION)
    {
        // 报告错误
    }

    advance();

    // 解析参数列表（可能为空）
    if (!check(TokenType::PUNCTUATION_RIGHT_PAREN))
    {
        do
        {
            call_exp->args_.push_back(parseExp());
        } while (match(TokenType::PUNCTUATION_COMMA));
    }

    advance();
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

std::unique_ptr<IOStmt> Parser::parsePrintfStmt()
{
    auto stmt = std::make_unique<IOStmt>();
    stmt->kind = IOStmt::IOKind::Printf;
    advance();
    advance();

    // 解析格式字符串
    if (!check(TokenType::CONSTANT_STRING))
    {
        throw ParseError(peek().line_, "Expect format string");
    }
    stmt->formatString_ = token_.value_;
    advance();

    // 解析参数列表
    if (match(TokenType::PUNCTUATION_COMMA))
    {
        do
        {
            stmt->args_.push_back(parseExp());
        } while (match(TokenType::PUNCTUATION_COMMA));
    }

    advance();
    advance();
    return stmt;
}

std::unique_ptr<IOStmt> Parser::parseGetintStmt()
{
    auto stmt = std::make_unique<IOStmt>();
    stmt->kind = IOStmt::IOKind::Getint;
    stmt->target_ = parseLVal();
    advance();
    advance();
    advance();
    advance();
    advance();
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
        token=tokens_[current_++];
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