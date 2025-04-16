#ifndef SYNTAXOUTPUTVISITOR_HPP
#define SYNTAXOUTPUTVISITOR_HPP
#include "parser.h"
#include <iostream>
#include <memory>
#include <unordered_set>

class SyntaxOutputVisitor : public Visitor
{
public:
    explicit SyntaxOutputVisitor(std::ostream &os = std::cout) : os_(os) {}

    // 实现所有visit方法
    // 编译单元节点
    void visit(CompUnit &node) override
    {
        // 遍历全局声明和函数定义
        for (auto &decl : node.decls_)
            decl->accept(*this);
        for (auto &func : node.funcDefs_)
            func->accept(*this);
        node.mainfuncDef_->accept(*this);
        outputNonTerminal("CompUnit");
    }

    // 声明
    void visit(ConstDef &node)
    {
        // 输出常量定义的Token（如标识符、维度、初始化符号）
        std::cout << node.name_ << std::endl; // 假设节点保存了名称Token

        // 遍历数组维度表达式（如 [2][3]）
        for (auto &dim : node.dimensions_)
        {
            std::cout << "[" << std::endl;
            dim->accept(*this); // 解析ConstExp
            std::cout << "]" << std::endl;
        }

        // 处理等号和初始化值
        std::cout << "ASSIGN =" << std::endl; // '='

        node.initVal_->accept(*this); // 解析ConstInitVal
    }
    void visit(ConstDecl &node) override
    {
        // 不输出Decl节点（根据文档要求过滤）
        for (auto &def : node.constDefs_)
            def->accept(*this);
    }
    void visit(VarDef &node)
    {
        // 类似ConstDef，处理变量定义
        std::cout << "IDENFR " << node.name_ << std::endl;
        for (auto &dim : node.constExps_)
        {
            std::cout << "[" << std::endl;
            dim->accept(*this);
            std::cout << "]" << std::endl;
        }
        if (node.hasInit)
        {
            std::cout << "ASSIGN =" << std::endl;
            node.initVal_->accept(*this);
        }
        std::cout << "<VarDef>" << std::endl;
    }
    void visit(VarDecl &node)
    {
        node.bType_->accept(*this);
        // 不输出Decl非终结符，遍历所有VarDef
        for (auto &def : node.varDefs_)
        {
            def->accept(*this);
        }

        std::cout << "SEMICN ;" << std::endl;
        std::cout << "<VarDecl>" << std::endl;
    }

    void visit(ExpStmt &)
    {
    }

    // 语句
    void visit(Block &node)
    {
        std::cout << "LBRACE {" << std::endl; // 输出 '{'
        for (auto &item : node.items_)
        {
            item->accept(*this); // 遍历BlockItem（自动过滤BlockItem标签）
        }
        std::cout << "RBRACE }" << std::endl; // 输出 '}'
        outputNonTerminal("Block");           // 输出 <Block>
    }

    void visit(AssignStmt &node)
    {
        node.lval_->accept(*this);            // 输出左值（如 IDENFR a）
        std::cout << "ASSIGN =" << std::endl; // 输出 '='
        node.exp_->accept(*this);             // 输出右值表达式
        outputNonTerminal("Exp");
        std::cout << "SEMICN ;" << std::endl; // 输出 ';'
        outputNonTerminal("Stmt");            // 输出 <Stmt>
    }

    void visit(IfStmt &node)
    {
        std::cout << "if" << std::endl; // 输出 'if'
        std::cout << "(" << std::endl;  // 输出 '('
        node.cond_->accept(*this);      // 输出条件表达式
        std::cout << ")" << std::endl;  // 输出 ')'

        node.thenBranch_->accept(*this); // 输出 then 分支

        if (node.elseBranch_)
        {
            std::cout << "else" << std::endl; // 输出 'else'
            node.elseBranch_->accept(*this);  // 输出 else 分支
        }
        outputNonTerminal("Stmt"); // 输出 <Stmt>
    }

    void visit(WhileStmt &node)
    {
        std::cout << "while" << std::endl; // 输出 'while'
        std::cout << "(";                  // 输出 '('
        node.cond_->accept(*this);         // 输出条件表达式
        std::cout << ")" << std::endl;     // 输出 ')'
        node.body_->accept(*this);         // 输出循环体
        outputNonTerminal("Stmt");         // 输出 <Stmt>
    }

    void visit(ReturnStmt &node)
    {
        std::cout << "RETURN return" << std::endl; // 输出 'return'
        if (node.exp_)
        {
            node.exp_->accept(*this); // 输出返回值表达式
            outputNonTerminal("Exp");
        }
        std::cout << "SEMICN ;" << std::endl; // 输出 ';'
        outputNonTerminal("Stmt");            // 输出 <Stmt>
    }

    void visit(IOStmt &node)
    {
        if (node.kind == IOStmt::IOKind::Getint)
        {
            node.target_->accept(*this); // 输出目标左值

            std::cout << "ASSIGN =" << std::endl;
            std::cout << "GETINTTK getint" << std::endl;

            std::cout << "LPARENT (" << std::endl;

            std::cout << "RPARENT )" << std::endl;
        }
        else
        {
            std::cout << "PRINTFTK printf" << std::endl;
            std::cout << "LPARENT (" << std::endl;
            std::cout << "STRCON " << node.formatString_ << std::endl;

            if (!node.args_.empty())
            {
                std::cout << "COMMA ," << std::endl;
                for (size_t i = 0; i < node.args_.size(); ++i)
                {
                    node.args_[i]->accept(*this);
                    outputNonTerminal("Exp");
                    if (i != node.args_.size() - 1)
                    {
                        std::cout << "COMMA ," << std::endl;
                    }
                }
            }
            std::cout << "RPARENT )" << std::endl;
        }
        std::cout << "SEMICN ;" << std::endl; // 输出 ';'
        outputNonTerminal("Stmt");            // 输出 <Stmt>
    }

    // 表达式
    void visit(LVal &node)
    {
        std::cout << "IDENFR " << node.name_ << std::endl;
        // 处理数组下标（如 [exp][exp]）
        for (auto &index : node.indices_)
        {
            std::cout << "[" << std::endl;
            index->accept(*this); // 解析下标表达式
            std::cout << "]" << std::endl;
        }
        outputNonTerminal("LVal"); // 输出 <LVal>
    }

    void visit(UnaryExp &node)
    {
        // 逻辑与InitVal类似，但只能包含ConstExp
        node.operand_->accept(*this);
        outputNonTerminal("UnaryExp");
    }

    void visit(AddExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理MulExp
            }
            else
            {
                TokenType op = std::get<TokenType>(elem);
                std::cout << Token::tokenTypeToString(op) << std::endl;
            }
        }
        outputNonTerminal("AddExp");
    }
    void visit(MulExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理UnaryExp
            }
            else
            {
                TokenType op = std::get<TokenType>(elem);
                os_ << Token::tokenTypeToString(op) << std::endl;
            }
        }
        outputNonTerminal("MulExp");
    }

    void visit(LOrExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理LAndExp
            }
            else
            {
                std::cout << "||" << std::endl;
            }
        }
        outputNonTerminal("LOrExp");
    }

    void visit(LAndExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理EqExp
            }
            else
            {
                std::cout << "&&" << std::endl;
            }
        }
        outputNonTerminal("LAndExp");
    }

    void visit(EqExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理RelExp
            }
            else
            {
                TokenType op = std::get<TokenType>(elem);
                std::cout << Token::tokenTypeToString(op) << std::endl;
            }
        }
        outputNonTerminal("EqExp");
    }

    void visit(RelExp &node)
    {
        for (auto &elem : node.elements_)
        {
            if (auto ptr = std::get_if<std::unique_ptr<Exp>>(&elem))
            {
                (*ptr)->accept(*this); // 递归处理AddExp
            }
            else
            {
                TokenType op = std::get<TokenType>(elem);
                std::cout << Token::tokenTypeToString(op) << std::endl;
            }
        }
        outputNonTerminal("RelExp");
    }

    void visit(CallExp &node)
    {
        // 输出函数名（如 IDENFR func）
        std::cout << node.funcName;
        std::cout << "(";

        // 解析参数列表
        for (size_t i = 0; i < node.args_.size(); ++i)
        {
            node.args_[i]->accept(*this);
            if (i != node.args_.size() - 1)
            {
                std::cout << ","; // 输出逗号分隔符
            }
        }

        std::cout << ")";
        outputNonTerminal("Exp"); // 输出 <CallExp>
    }

    void visit(Number &node)
    {
        std::cout << node.value_;
        outputNonTerminal("Number"); // 输出 <Number>
    }

    void visit(FuncParam &node)
    {
        // 输出类型（INTTK int）
        std::cout << Token::tokenTypeToString(TokenType::KEYWORD_INT);
        std::cout << node.name_; // 输出参数名（IDENFR a）

        // 处理数组类型（如 [ ] 或 [2][3]）
        for (auto &dim : node.dimSizes_)
        {
            std::cout << "[";
            if (dim)
            {
                dim->accept(*this); // 解析维度表达式（ConstExp）
            }
            std::cout << "]";
        }
        outputNonTerminal("FuncParam");
    }

    void visit(FuncDef &node) override
    {
        std::cout << node.name_;
        std::cout << "(" << std::endl;
        for (auto &param : node.params_)
            param->accept(*this);
        std::cout << ")" << std::endl;
        node.body_->accept(*this);
        outputNonTerminal("FuncDef");
    }

    void visit(MainFuncDef &node)
    {
        std::cout << Token::tokenTypeToString(TokenType::KEYWORD_INT) << " int" << std::endl;
        std::cout << "MAINTK main" << std::endl;
        std::cout << "LPARENT (" << std::endl; // 输出 '('
        std::cout << "RPARENT )" << std::endl; // 输出 ')'
        node.body_->accept(*this);             // 解析函数体（Block）
        outputNonTerminal("MainFuncDef");      // 输出 <MainFuncDef>
    }

    void visit(BType &node)
    {
        std::cout << "INTTK int" << std::endl;
    }

    void visit(InitVal &node)
    {

        if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
        {
            auto *exp = std::get_if<std::unique_ptr<AST::Exp>>(&node.value_);

            // auto exp = std::get<std::unique_ptr<AST::Exp>>(node.value_);
            //  解析单个表达式
            (*exp)->accept(*this);
        }
        else
        {
            auto *list = std::get_if<std::vector<std::unique_ptr<AST::InitVal>>>(&node.value_);
            std::cout << "{";
            for (size_t i = 0; i < (*list).size(); ++i)
            {

                (*list)[i]->accept(*this); // 递归解析子初始化值

                if (i != (*list).size() - 1)
                {
                    std::cout << "," << std::endl; // 输出逗号
                }
            }
            std::cout << "}" << std::endl; // 输出 '}'
        }

        outputNonTerminal("InitVal"); // 输出 <InitVal>
    }

    void visit(ConstInitVal &node)
    {
        // 逻辑与InitVal类似，但只能包含ConstExp
        if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.value_))
        {
            auto *exp = std::get_if<std::unique_ptr<AST::Exp>>(&node.value_);

            (*exp)->accept(*this);
        }
        else
        {
            auto *list = std::get_if<std::vector<std::unique_ptr<AST::ConstInitVal>>>(&node.value_);

            std::cout << "(" << std::endl;
            for (size_t i = 0; i < (*list).size(); ++i)
            {
                (*list)[i]->accept(*this);
                if (i != (*list).size() - 1)
                {
                    std::cout << "," << std::endl;
                }
            }
            std::cout << ")" << std::endl;
        }

        outputNonTerminal("ConstInitVal"); // 输出 <ConstInitVal>
    }

    void visit(BlockItem &node)
    {
        node.item_->accept(*this);
    }

    void visit(PrimaryExp &node)
    {
        if (std::holds_alternative<std::unique_ptr<AST::Exp>>(node.operand_))
        {
            auto *exp = std::get_if<std::unique_ptr<AST::Exp>>(&node.operand_);

            (*exp)->accept(*this);
            outputNonTerminal("Exp");
        }
        else if (std::holds_alternative<std::unique_ptr<AST::LVal>>(node.operand_))
        {
            auto *lVal = std::get_if<std::unique_ptr<AST::LVal>>(&node.operand_);

            (*lVal)->accept(*this);
        }
        else
        {
            auto *num = std::get_if<std::unique_ptr<AST::Number>>(&node.operand_);

            (*num)->accept(*this);
        }

        outputNonTerminal("PrimaryExp");
    }

    void visit(FuncType &node)
    {
        std::cout << node.typeName_ << std::endl;
        outputNonTerminal("FuncType");
    }

private:
    std::ostream &os_;
    const std::unordered_set<std::string> skipped_rules_ = {"BlockItem", "Decl", "BType"};

    void outputNonTerminal(const std::string &rule)
    {
        if (skipped_rules_.count(rule) == 0)
        {
            os_ << "<" << rule << ">" << std::endl;
        }
    }
};

#endif // SYNTAXOUTPUTVISITOR_HPP