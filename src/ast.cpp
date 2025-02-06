#include "ast.h"
using namespace AST;

void Program::accept(Visitor &visitor) { visitor.visit(*this); }
void TypeSpecifier::accept(Visitor &visitor) { visitor.visit(*this); }
void FunctionDeclaration::accept(Visitor &visitor) { visitor.visit(*this); }
void Parameter::accept(Visitor &visitor) { visitor.visit(*this); }
void CompoundStatement::accept(Visitor &visitor) { visitor.visit(*this); }
void ExpressionStatement::accept(Visitor &visitor) { visitor.visit(*this); }
void ReturnStatement::accept(Visitor &visitor) { visitor.visit(*this); }
void IfStatement::accept(Visitor &visitor) { visitor.visit(*this); }
void WhileStatement::accept(Visitor &visitor) { visitor.visit(*this); }
void AssignmentExpression::accept(Visitor &visitor) { visitor.visit(*this); }
void BinaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }
void UnaryExpression::accept(Visitor &visitor) { visitor.visit(*this); }
void Identifier::accept(Visitor &visitor) { visitor.visit(*this); }
void NumberLiteral::accept(Visitor &visitor) { visitor.visit(*this); }
void FunctionCall::accept(Visitor &visitor) { visitor.visit(*this); }