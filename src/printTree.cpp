#include"printTree.h"

void PrintTree::visit(Program &program)
{
    for(auto &ptr : program.declarations_){
        ptr->accept(*this);
    }
}

void PrintTree::visit(TypeSpecifier &type)
{
    std::cout<<type.typeName_<<std::endl;
}

void PrintTree::visit(FunctionDeclaration &funcDecl)
{
    
}

void PrintTree::visit(Parameter &)
{
}

void PrintTree::visit(CompoundStatement &)
{
}

void PrintTree::visit(ExpressionStatement &)
{
}

void PrintTree::visit(ReturnStatement &)
{
}

void PrintTree::visit(IfStatement &)
{
}

void PrintTree::visit(WhileStatement &)
{
}

void PrintTree::visit(AssignmentExpression &)
{
}

void PrintTree::visit(BinaryExpression &)
{
}

void PrintTree::visit(UnaryExpression &)
{
}

void PrintTree::visit(Identifier &)
{
}

void PrintTree::visit(NumberLiteral &)
{
}

void PrintTree::visit(FunctionCall &)
{
}
