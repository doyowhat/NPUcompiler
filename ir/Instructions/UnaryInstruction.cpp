#include "UnaryInstruction.h"

/// @brief 构造函数
/// @param _op 操作符
/// @param _result 结果操作数
/// @param _srcVal1 源操作数1
UnaryInstruction::UnaryInstruction(Function * _func, IRInstOperator _op, Value * _srcVal1, Type * _type)
    : Instruction(_func, _op, _type)
{
    addOperand(_srcVal1);
}

/// @brief 转换成字符串
/// @param str 转换后的字符串
void UnaryInstruction::toString(std::string & str)
{

    Value * src1 = getOperand(0);
    switch (op) {
        case IRInstOperator::IRINST_OP_NEG_I:
            // 取负指令，一元运算
            str = getIRName() + " = neg " + src1->getIRName();
            break;
        default:
            // 未知指令
            Instruction::toString(str);
            break;
    }
}
