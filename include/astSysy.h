// Created by wuhao on 2021/4/4.
// 语法树节点类定义
#ifndef SYSY_ASTSYSY_H
#define SYSY_ASTSYSY_H
#pragma once

#include <memory>
#include "lexer.h"
#include <vector>
#include <variant>

namespace AST
{
    // 前向声明访问者类
    class Visitor;

    // AST节点基类
    class Node
    {
    public:
        virtual ~Node() = default;
        virtual void accept(Visitor &visitor) = 0;
    };

    // 声明基类
    class Decl : public Node
    {
    };

    // 表达式基类
    class Exp : public Node
    {
    };

    // 语句基类
    class Stmt : public Node
    {
    };

    // 声明节点
    class ConstDef;  // 常量定义
    class ConstDecl; // 常量声明（const int a=1, b[2]={1,2};）
    class VarDef;    // 变量定义（int a; 或 int a[2][3] = {{1,2}, {3,4}};）
    class VarDecl;   // 变量声明 （int a,b[2];）

    // 函数与形参节点
    class FuncParam;   // 函数形参（int a[] 或 int a[2][3]）
    class FuncDef;     // 函数定义（int func(int a, int b[]) { ... }）
    class MainFuncDef; // 主函数定义（int main() { ... }）

    // 语句节点
    class Block;      // 语句块（{ ... }）
    class AssignStmt; // 赋值语句（a = 5;）
    class IfStmt;     // if语句（if (cond) stmt [else stmt]）
    class WhileStmt;  // while语句（while (cond) stmt）
    class ReturnStmt; // （return exp; 或 return;）
    class IOStmt;     // 输入输出语句（getint/printf）

    // 表达式节点
    class LVal; // 左值（变量或数组元素）
    class PrimaryExp;
    // class BinaryExp; // 二元运算表达式（a + b, a && b 等）
    class AddExp;
    class MulExp;
    class LOrExp;
    class LAndExp;
    class EqExp;
    class RelExp;
    class UnaryExp; // 一元运算表达式（-a, !cond）
    class CallExp;  // 函数调用表达式（func(a, b)）
    class Number;   // 字面量（数值常量）

    // 编译单元节点
    class CompUnit;

    // 函数返回类型
    class FuncType;

    // 基本类型
    class BType;
    class InitVal;
    class ConstInitVal;
    class BlockItem;

    // 访问者接口
    class Visitor
    {
    public:
        virtual ~Visitor() = default;

        // 编译单元节点
        virtual void visit(CompUnit &) = 0;

        // 声明
        virtual void visit(ConstDef &) = 0;
        virtual void visit(ConstDecl &) = 0;
        virtual void visit(VarDef &) = 0;
        virtual void visit(VarDecl &) = 0;

        // 语句
        virtual void visit(Block &) = 0;
        virtual void visit(AssignStmt &) = 0;
        virtual void visit(IfStmt &) = 0;
        virtual void visit(WhileStmt &) = 0;
        virtual void visit(ReturnStmt &) = 0;
        virtual void visit(IOStmt &) = 0;

        // 表达式
        virtual void visit(LVal &) = 0;
        virtual void visit(PrimaryExp &) = 0;
        // virtual void visit(BinaryExp &) = 0;
        virtual void visit(UnaryExp &) = 0;
        virtual void visit(AddExp &) = 0;
        virtual void visit(MulExp &) = 0;

        virtual void visit(LOrExp &) = 0;
        virtual void visit(LAndExp &) = 0;
        virtual void visit(EqExp &) = 0;
        virtual void visit(RelExp &) = 0;

        virtual void visit(CallExp &) = 0;
        virtual void visit(Number &) = 0;

        virtual void visit(FuncParam &) = 0;
        virtual void visit(FuncDef &) = 0;
        virtual void visit(MainFuncDef &) = 0;

        virtual void visit(BType &) = 0;
        virtual void visit(InitVal &) = 0;
        virtual void visit(ConstInitVal &) = 0;
        virtual void visit(BlockItem &) = 0;
    };

    class CompUnit : public Node
    {
    public:
        std::vector<std::unique_ptr<Decl>> decls_;       // 全局声明（变量/常量）
        std::vector<std::unique_ptr<FuncDef>> funcDefs_; // 函数定义
        std::unique_ptr<MainFuncDef> mainfuncDef_;       // 主函数

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    //--------------------------------------------------------------------------
    // 常量声明（const int a=1,b[2]={1,2};)
    class ConstDecl : public Decl
    {
    public:
        std::unique_ptr<BType> bType_;
        std::vector<std::unique_ptr<ConstDef>> constDefs_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 常量定义
    class ConstDef : public Node
    {
    public:
        std::string name_;
        std::vector<std::unique_ptr<Exp>> dimensions_;
        std::unique_ptr<ConstInitVal> initVal_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 变量声明
    class VarDecl : public Decl
    {
    public:
        std::unique_ptr<BType> bType_;
        std::vector<std::unique_ptr<VarDef>> varDefs_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    class BType : public Node
    {
    public:
        std::string typeName_{"int"};

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 变量定义
    class VarDef : public Node
    {
    public:
        std::string name_;
        std::vector<std::unique_ptr<Exp>> constExps_;
        std::unique_ptr<InitVal> initVal_; // 可为空
        bool hasInit = false;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    //----------------------------------------------------------------------------
    // 函数形参（int a[]或 int a[2][3]）
    class FuncParam : public Node
    {
    public:
        std::unique_ptr<BType> bType_;
        std::string name_;
        std::vector<std::unique_ptr<Exp>> dimSizes_; // 数组维度（第一维可缺）Exp指向 BinaryAdd
        bool isArray_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 主函数定义
    class MainFuncDef : public Node
    {
    public:
        std::unique_ptr<Block> body_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 函数定义
    class FuncDef : public Node
    {
    public:
        std::unique_ptr<FuncType> returnType_; //"int" 或 "void"
        std::string name_;
        std::vector<std::unique_ptr<FuncParam>> params_;
        std::unique_ptr<Block> body_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    //--------------------------------------------------------------------------
    // 语句块
    class Block : public Stmt
    {
    public:
        std::vector<std::unique_ptr<BlockItem>> items_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 语句块
    class BlockItem : public Node
    {
    public:
        std::unique_ptr<Node> item_; // Decl Or Stmt

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 赋值语句
    class AssignStmt : public Stmt
    {
    public:
        std::unique_ptr<LVal> lval_;
        std::unique_ptr<Exp> exp_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // If语句
    class IfStmt : public Stmt
    {
    public:
        std::unique_ptr<Exp> cond_;
        std::unique_ptr<Stmt> thenBranch_;
        std::unique_ptr<Stmt> elseBranch_; // 可为null

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // while语句
    class WhileStmt : public Stmt
    {
    public:
        std::unique_ptr<Exp> cond_;
        std::unique_ptr<Stmt> body_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 返回语句
    class ReturnStmt : public Stmt
    {
    public:
        std::unique_ptr<Exp> exp_; // 可为null

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 输入输出语句
    class IOStmt : public Stmt
    {
    public:
        enum class IOKind
        {
            Getint,
            Printf
        };
        IOKind kind;
        std::unique_ptr<LVal> target_;           // getint时使用
        std::string formatString_;               // printf时使用
        std::vector<std::unique_ptr<Exp>> args_; // printf的参数

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    //----------------------------------------------------------------------------------

    // 左值
    class LVal : public Exp
    {
    public:
        std::string name_;
        std::vector<std::unique_ptr<Exp>> indices_; // 数组下标（可为空）

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    class PrimaryExp : public Exp
    {
    public:
        std::variant<std::unique_ptr<Exp>, std::unique_ptr<LVal>, std::unique_ptr<Number>> operand_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 一元运算表达式
    class UnaryExp : public Exp
    {
    public:
        enum class Op
        {
            Plus,
            Minus,
            Not
        };
        Op op;
        std::unique_ptr<PrimaryExp> operand_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // // 二元运算表达式
    // class BinaryExp : public Exp
    // {
    // public:
    //     enum class Op
    //     {
    //         Add,
    //         Sub,
    //         Mul,
    //         Div,
    //         Mod,
    //         Lt,
    //         Le,
    //         Gt,
    //         Ge,
    //         Eq,
    //         Neq,
    //         And,
    //         Or
    //     };
    //     BinaryExp(Op &op, std::unique_ptr<Exp> left, std::unique_ptr<Exp> right)
    //         : op(op), left_(std::move(left)), right_(std::move(right)) {}
    //     Op op;
    //     std::unique_ptr<Exp> left_;
    //     std::unique_ptr<Exp> right_;

    //     void accept(Visitor &v) override
    //     {
    //         v.visit(*this);
    //     }
    // };

    // 加法表达式层（对应 + - 运算）
    class AddExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType                // 运算符（PLUS/MINUS）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    // 乘法表达式层（对应 * / % 运算）
    class MulExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType             // 运算符（MULT/DIV/MOD）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    class LOrExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType             // 运算符（||）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    class LAndExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType             // 运算符（&&）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    class EqExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType             // 运算符（==/!=）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    class RelExp : public Exp
    {
    public:
        std::vector<std::variant<
            std::unique_ptr<Exp>, // 操作数
            TokenType             // 运算符（<、>、<=、>=）
            >>
            elements_;

        void accept(Visitor &v) override { v.visit(*this); }
    };

    // 函数调用表达式
    class CallExp : public Exp
    {
    public:
        std::string funcName;
        std::vector<std::unique_ptr<Exp>> args_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 字面量
    class Number : public Exp
    {
    public:
        int value_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    //-----------------------------------------------------------------------

    class FuncType
    {
    public:
        std::string typeName_;
    };

    class ConstInitVal : public Node
    {
    public:
        // 可能是单个常量表达式或嵌套数组初始化
        std::variant<std::unique_ptr<Exp>, std::vector<std::unique_ptr<ConstInitVal>>> value_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

    // 变量初始化值（可包含运行时表达式）
    class InitVal : public Node
    {
    public:
        // 可能是单个表达式或嵌套数组初始化
        std::variant<std::unique_ptr<Exp>, std::vector<std::unique_ptr<InitVal>>> value_;

        void accept(Visitor &v) override
        {
            v.visit(*this);
        }
    };

}

#endif //SYSY_ASTSYSY_H