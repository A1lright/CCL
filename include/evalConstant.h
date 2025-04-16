#ifndef EVAL_CONSTANT_H
#define EVAL_CONSTANT_H

#include "astSysy.h"
#include <variant>
using namespace AST;

class EvalConstant
{
public:
    // 入口函数，传入 AST 基类指针，返回求值结果，只支持int型
    int Eval(Node *node);

private:
    int VisitNumberExp(Number *exp);
    int VisitAddExp(AddExp *exp);
    int VisitMulExp(MulExp *exp);
    int VisitUnaryExp(UnaryExp *exp);
    int VisitLValExp(LVal *exp);
    int VisitPrimaryExp(PrimaryExp *exp);
    int VisitLOrExp(LOrExp *exp);
    int VisitLAndExp(LAndExp *exp);
    int VisitEqExp(EqExp *exp);
    int VisitRelExp(RelExp *exp);
    int VisitCallExp(CallExp *exp);
    int VisitInitVal(InitVal *initVal);
    int VisitConstInitVal(ConstInitVal *constInitVal);
};

#endif // EVAL_CONSTANT_H