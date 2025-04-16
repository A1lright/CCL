#include "evalConstant.h"
#include <iostream>

using namespace AST;

int EvalConstant::Eval(Node *node)
{
    // 根据动态类型分发到相应的处理函数
    auto kind = node->getKind();

    switch (kind)
    {
    case Node::ND_Number:
        return VisitNumberExp(dynamic_cast<Number *>(node));

    case Node::ND_AddExp:
        return VisitAddExp(dynamic_cast<AddExp *>(node));

    case Node::ND_MulExp:
        return VisitMulExp(dynamic_cast<MulExp *>(node));

    case Node::ND_UnaryExp:
        return VisitUnaryExp(dynamic_cast<UnaryExp *>(node));

    case Node::ND_LVal:
        return VisitLValExp(dynamic_cast<LVal *>(node));

    case Node::ND_PrimaryExp:
        return VisitPrimaryExp(dynamic_cast<PrimaryExp *>(node));

    case Node::ND_LOrExp:
        return VisitLOrExp(dynamic_cast<LOrExp *>(node));

    case Node::ND_LAndExp:
        return VisitLAndExp(dynamic_cast<LAndExp *>(node));

    case Node::ND_EqExp:
        return VisitEqExp(dynamic_cast<EqExp *>(node));

    case Node::ND_RelExp:
        return VisitRelExp(dynamic_cast<RelExp *>(node));

    case Node::ND_CallExp:
        return VisitCallExp(dynamic_cast<CallExp *>(node));

    case Node::ND_InitVal:
        return VisitInitVal(dynamic_cast<InitVal *>(node));

    case Node::ND_ConstInitVal:
        return VisitConstInitVal(dynamic_cast<ConstInitVal *>(node));

    default:
        throw std::runtime_error("Unsupported node type in constant evaluation.");
    }
}

int EvalConstant::VisitNumberExp(Number *exp)
{
    return exp->value_;
}

int EvalConstant::VisitAddExp(AddExp *exp)
{
    // 第一个元素必须是一个表达式指针
    // 后续依次为运算符（TokenType）和表达式指针
    bool first = true;
    int result = 0;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            int value = Eval(child.get());
            if (first)
            {
                result = value;
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_PLUS)
                    result += value;
                else if (op == TokenType::OPERATOR_MINUS)
                    result -= value;
                else
                    throw std::runtime_error("Invalid operator in AddExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result;
}

int EvalConstant::VisitMulExp(MulExp *exp)
{
    bool first = true;
    int result = 0;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            auto &child = std::get<std::unique_ptr<Exp>>(elem);
            int value = Eval(child.get());
            if (first)
            {
                result = value;
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_MULTIPLY)
                    result *= value;
                else if (op == TokenType::OPERATOR_DIVIDE)
                {
                    if (value == 0)
                        throw std::runtime_error("Division by zero in constant expression.");
                    result /= value;
                }
                else if (op == TokenType::OPERATOR_MODULO)
                {
                    if (value == 0)
                        throw std::runtime_error("Modulo by zero in constant expression.");
                    result %= value;
                }
                else
                    throw std::runtime_error("Invalid operator in MulExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result;
}

int EvalConstant::VisitUnaryExp(UnaryExp *exp)
{
    int operand = Eval(exp->operand_.get());
    switch (exp->op)
    {
    case UnaryExp::Op::Plus:
        return operand;
    case UnaryExp::Op::Minus:
        return -operand;
    case UnaryExp::Op::Not:
        return (operand == 0) ? 1 : 0;
    default:
        throw std::runtime_error("Unknown unary operator.");
    }
}

int EvalConstant::VisitLValExp(LVal *exp)
{
    // 对于常量表达式中的变量引用，如果变量是常量则应在语义分析阶段已将其值求出；
    return 0;
    throw std::runtime_error("LVal cannot be evaluated as a constant expression.");
}

int EvalConstant::VisitPrimaryExp(PrimaryExp *exp)
{
    // PrimaryExp 的 operand_ 是 std::variant<std::unique_ptr<Exp>, std::unique_ptr<LVal>, std::unique_ptr<Number>>
    if (std::holds_alternative<std::unique_ptr<Exp>>(exp->operand_))
    {
        return Eval(std::get<std::unique_ptr<Exp>>(exp->operand_).get());
    }
    else if (std::holds_alternative<std::unique_ptr<LVal>>(exp->operand_))
    {
        return Eval(std::get<std::unique_ptr<LVal>>(exp->operand_).get());
    }
    else if (std::holds_alternative<std::unique_ptr<Number>>(exp->operand_))
    {
        return Eval(std::get<std::unique_ptr<Number>>(exp->operand_).get());
    }
    throw std::runtime_error("Invalid PrimaryExp variant.");
}

int EvalConstant::VisitLOrExp(LOrExp *exp)
{
    // 实现短路逻辑或运算
    bool result = false;
    bool first = true;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            int value = Eval(std::get<std::unique_ptr<Exp>>(elem).get());
            if (first)
            {
                result = (value != 0);
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_LOGICAL_OR)
                    result = result || (value != 0);
                else
                    throw std::runtime_error("Invalid operator in LOrExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result ? 1 : 0;
}

int EvalConstant::VisitLAndExp(LAndExp *exp)
{
    // 实现短路逻辑与运算
    bool result = true;
    bool first = true;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            int value = Eval(std::get<std::unique_ptr<Exp>>(elem).get());
            if (first)
            {
                result = (value != 0);
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_LOGICAL_AND)
                    result = result && (value != 0);
                else
                    throw std::runtime_error("Invalid operator in LAndExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result ? 1 : 0;
}

int EvalConstant::VisitEqExp(EqExp *exp)
{
    // 处理相等运算：支持 "==" 与 "!="
    bool first = true;
    int result = 0;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            int value = Eval(std::get<std::unique_ptr<Exp>>(elem).get());
            if (first)
            {
                result = value;
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_EQUAL)
                    result = (result == value) ? 1 : 0;
                else if (op == TokenType::OPERATOR_NOT_EQUAL)
                    result = (result != value) ? 1 : 0;
                else
                    throw std::runtime_error("Invalid operator in EqExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result;
}

int EvalConstant::VisitRelExp(RelExp *exp)
{
    // 处理关系运算：<, >, <=, >=
    bool first = true;
    int result = 0;
    TokenType op = TokenType::UNKNOW;
    for (auto &elem : exp->elements_)
    {
        if (std::holds_alternative<std::unique_ptr<Exp>>(elem))
        {
            int value = Eval(std::get<std::unique_ptr<Exp>>(elem).get());
            if (first)
            {
                result = value;
                first = false;
            }
            else
            {
                if (op == TokenType::OPERATOR_LESS)
                    result = (result < value) ? 1 : 0;
                else if (op == TokenType::OPERATOR_GREATER)
                    result = (result > value) ? 1 : 0;
                else if (op == TokenType::OPERATOR_LESS_EQUAL)
                    result = (result <= value) ? 1 : 0;
                else if (op == TokenType::OPERATOR_GREATER_EQUAL)
                    result = (result >= value) ? 1 : 0;
                else
                    throw std::runtime_error("Invalid operator in RelExp.");
            }
        }
        else if (std::holds_alternative<TokenType>(elem))
        {
            op = std::get<TokenType>(elem);
        }
    }
    return result;
}

int EvalConstant::VisitCallExp(CallExp *exp)
{
    // 常量表达式中一般不允许函数调用
    return 0;
    throw std::runtime_error("Function call not allowed in constant expression.");
}

int EvalConstant::VisitInitVal(InitVal *initVal)
{
    if(std::holds_alternative<std::unique_ptr<Exp>>(initVal->value_))
    {
        return Eval(std::get<std::unique_ptr<Exp>>(initVal->value_).get());
    }
}

int EvalConstant::VisitConstInitVal(ConstInitVal *constInitVal){
    if(std::holds_alternative<std::unique_ptr<Exp>>(constInitVal->value_))
    {
        return Eval(std::get<std::unique_ptr<Exp>>(constInitVal->value_).get());
    }
}