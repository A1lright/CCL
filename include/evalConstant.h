#pragma once

#include "astSysy.h"
#include <variant>
using namespace AST;
class EvalConstant
{
private:
public:
    int Eval(Node *node);

private:
    int VisitNumberExp(Number *exp);
    int VisitAddExp(AddExp *exp);
    int VisitUnaryExp(UnaryExp *exp);
    int VisitMulExp(MulExp *exp);
    int VisitLValExp(LVal *exp);
    int VisitPrimaryExp(PrimaryExp *exp);
    int VisitLOrExp(LOrExp *exp);
    int VisitLAndExp(LAndExp *exp);
    int VisitEqExp(EqExp *exp);
    int VisitRelExp(RelExp *exp);
    int VisitCallExp(CallExp *exp);
};