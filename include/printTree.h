#include"ast.h"
using namespace AST;

class PrintTree:public Visitor{

    // 程序
    void visit(Program&);

    //类型说明符
    void visit(TypeSpecifier&);
    
    // 声明
    void visit(FunctionDeclaration&);
    void visit(Parameter&);
    
    // 语句
    void visit(CompoundStatement&);
    void visit(ExpressionStatement&);
    void visit(ReturnStatement&);
    void visit(IfStatement&);
    void visit(WhileStatement&);
    
    // 表达式
    void visit(AssignmentExpression&);
    void visit(BinaryExpression&);
    void visit(UnaryExpression&);
    void visit(Identifier&);
    void visit(NumberLiteral&);
    void visit(FunctionCall&);
};