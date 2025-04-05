#include "evalConstant.h"

int EvalConstant::Eval(Node *node)
{
    return 0;
}

int EvalConstant::VisitNumberExp(Number *exp)
{
    return 0;
}

int EvalConstant::VisitAddExp(AddExp *exp)
{
    return 0;
}

int EvalConstant::VisitUnaryExp(UnaryExp *exp)
{
    return 0;
}

int EvalConstant::VisitMulExp(MulExp *exp)
{
    return 0;
}

int EvalConstant::VisitLValExp(LVal *exp)
{
    return 0;
}

int EvalConstant::VisitPrimaryExp(PrimaryExp *exp)
{
    return 0;
}

int EvalConstant::VisitLOrExp(LOrExp *exp)
{
    return 0;
}

int EvalConstant::VisitLAndExp(LAndExp *exp)
{
    return 0;
}

int EvalConstant::VisitEqExp(EqExp *exp)
{
    return 0;
}

int EvalConstant::VisitRelExp(RelExp *exp)
{
    return 0;
}

int EvalConstant::VisitCallExp(CallExp *exp)
{
    return 0;
}
