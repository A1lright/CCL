#include <memory>
#include "lexer.h"
#include <vector>

namespace AST
{

    class Program;
    class TypeSpecifier;
    class FunctionDeclaration;
    class Parameter;
    class CompoundStatement;
    class ExpressionStatement;
    class ReturnStatement;
    class IfStatement;
    class WhileStatement;
    class AssignmentExpression;
    class BinaryExpression;
    class UnaryExpression;
    class Identifier;
    class NumberLiteral;
    class FunctionCall;

    // 访问者接口
    class Visitor
    {
    public:
        virtual ~Visitor() = default;

        // 程序
        virtual void visit(Program &) = 0;

        // 类型说明符
        virtual void visit(TypeSpecifier &) = 0;

        // 声明
        virtual void visit(FunctionDeclaration &) = 0;
        virtual void visit(Parameter &) = 0;

        // 语句
        virtual void visit(CompoundStatement &) = 0;
        virtual void visit(ExpressionStatement &) = 0;
        virtual void visit(ReturnStatement &) = 0;
        virtual void visit(IfStatement &) = 0;
        virtual void visit(WhileStatement &) = 0;

        // 表达式
        virtual void visit(AssignmentExpression &) = 0;
        virtual void visit(BinaryExpression &) = 0;
        virtual void visit(UnaryExpression &) = 0;
        virtual void visit(Identifier &) = 0;
        virtual void visit(NumberLiteral &) = 0;
        virtual void visit(FunctionCall &) = 0;
    };

    class Node
    {
    public:
        virtual ~Node() = default;
        virtual void accept(Visitor &visitor) = 0;
    };

    // 表达式基类
    class Expression : public Node
    {
    };
    // 语句基类
    class Statement : public Node
    {
    };
    // 声明基类
    class Declaration : public Node
    {
    };

    //------------------具体节点类型----------------------

    // 程序节点（根节点）
    class Program : public Node
    {
    public:
        std::vector<std::unique_ptr<Declaration>> declarations_;

        void accept(Visitor &visitor) override;
    };

    // 类型说明符
    class TypeSpecifier : public Node
    {
    public:
        std::string typeName_; //"int char void 等"TODO自定义类型？

        void accept(Visitor &visitor) override;
    };

    // 函数声明
    class FunctionDeclaration : public Declaration
    {
    public:
        std::unique_ptr<TypeSpecifier> returnType_;
        std::string name_;
        std::vector<std::unique_ptr<Parameter>> params_;
        std::unique_ptr<CompoundStatement> body_;

        void accept(Visitor &visitor) override;
    };

    // 函数参数
    class Parameter : public Node
    {
    public:
        std::unique_ptr<TypeSpecifier> type_;
        std::string name_;

        void accept(Visitor &visitor) override;
    };

    // 复合语句（代码块）
    class CompoundStatement : public Statement
    {
    public:
        std::vector<std::unique_ptr<Statement>> statements_;

        void accept(Visitor &visitor) override;
    };

    // 表达式语句
    class ExpressionStatement : public Statement
    {
    public:
        std::unique_ptr<Expression> expression_;

        void accept(Visitor &visitor) override;
    };

    // 返回语句
    class ReturnStatement : public Statement
    {
    public:
        std::unique_ptr<Expression> expression_; // 可为null

        void accept(Visitor &visitor) override;
    };

    // If语句
    class IfStatement : public Statement
    {
    public:
        std::unique_ptr<Expression> condition_;
        std::unique_ptr<Statement> thenBranch_;
        std::unique_ptr<Statement> elseBranch_; // 可为null

        void accept(Visitor &visitor);
    };

    // while语句
    class WhileStatement : public Statement
    {
    public:
        std::unique_ptr<Expression> condition_;
        std::unique_ptr<Statement> body_;

        void accept(Visitor &visitor);
    };

    // 赋值表达式
    class AssignmentExpression : public Expression
    {
    public:
        std::unique_ptr<Expression> lhs_;
        std::unique_ptr<Expression> rhs_;

        void accept(Visitor &visitor) override;
    };

    // 二元表达式
    class BinaryExpression : public Expression
    {
    public:
        std::unique_ptr<Expression> lhs_;
        std::string op; // op: "+ - * / == !="
        std::unique_ptr<Expression> rhs_;

        void accept(Visitor &visitor) override;
    };

    // 一元表达式
    class UnaryExpression : public Expression
    {
    public:
        Token token_;
        std::unique_ptr<Expression> operand_;

        void accept(Visitor &visitor) override;
    };

    // 标识符
    class Identifier : public Expression
    {
    public:
        Token token_;

        void accept(Visitor &visitor) override;
    };

    // 数字字面量
    class NumberLiteral : public Expression
    {
    public:
        Token token_; // 原始字符串表示

        void accept(Visitor &visitor) override;
    };

    // 函数调用
    class FunctionCall : public Expression
    {
    public:
        std::string callee_;
        std::vector<std::unique_ptr<Expression>> arguments_;

        void accept(Visitor &visitor) override;
    };
}