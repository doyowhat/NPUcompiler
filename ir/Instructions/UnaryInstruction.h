#pragma once

#include "Instruction.h"

///
/// @brief 二元运算指令
///
class UnaryInstruction : public Instruction {

public:
    /// @brief 构造函数
    /// @param _op 操作符
    /// @param _result 结果操作数
    /// @param _srcVal1 源操作数1
    UnaryInstruction(Function * _func, IRInstOperator _op, Value * _srcVal1, Type * _type);
    /// @brief 转换成字符串
    void toString(std::string & str) override;
};
