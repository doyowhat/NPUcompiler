///
/// @file BranchInstruction.cpp
/// @brief 条件跳转指令（bc、bt、bf）的统一实现
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2024-09-29
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// </table>
///

#include "BranchInstruction.h"
#include "VoidType.h"
#include "IntegerType.h"
#include <iostream>
// 双向条件跳转构造函数（bc）
BranchInstruction::BranchInstruction(Function * _func,
                                     IRInstOperator _op,
                                     Value * _condVar,
                                     Instruction * _trueTarget,
                                     Instruction * _falseTarget)
    : Instruction(_func, _op, VoidType::getType()), condVar(_condVar), trueTarget(_trueTarget),
      falseTarget(_falseTarget), Target(nullptr)
{
    // 验证条件变量类型为i1
    // assert(condVar->getType() == IntegerType::getTypeInt1() && "条件变量类型必须为i1");
    // assert(_op == IRInstOperator::IRINST_OP_BC && "双向构造函数仅适用于BC指令");
}

// 单向条件跳转构造函数（bt/bf）
BranchInstruction::BranchInstruction(Function * _func, IRInstOperator _op, Value * _condVar, Instruction * _Target)
    : Instruction(_func, _op, VoidType::getType()), condVar(_condVar), trueTarget(nullptr), falseTarget(nullptr),
      Target(_Target)
{
    // 验证条件变量类型为i1
    // assert(condVar->getType() == IntegerType::getTypeInt1() && "条件变量类型必须为i1");
    // assert(_op == IRInstOperator::IRINST_OP_BT ||_op == IRInstOperator::IRINST_OP_BF &&
    // "单向构造函数仅适用于BT/BF指令");
}

void BranchInstruction::toString(std::string & str)
{
    switch (op) {
        case IRInstOperator::IRINST_OP_BC:
            // bc condvar, label X, label Y
            str = "bc " + condVar->getIRName() + ", " + trueTarget->getIRName() + ", " + falseTarget->getIRName();
            break;
        case IRInstOperator::IRINST_OP_BT:
            // bt condvar, label X
            std::cerr << condVar << std::endl;
            std::cerr << Target << std::endl;
            str = "bt " + condVar->getIRName() + ", " + Target->getIRName();
            break;
        case IRInstOperator::IRINST_OP_BF:
            // bf condvar, label X
            std::cerr << condVar << std::endl;
            std::cerr << Target << std::endl;
            str = "bf " + condVar->getIRName() + ", " + Target->getIRName();
            break;
        default:
            // 未知指令
            Instruction::toString(str);
            break;
    }
}

Value * BranchInstruction::getCondVar() const
{
    return condVar;
}

Instruction * BranchInstruction::getTrueTarget() const
{
    // assert(op == IRInstOperator::IRINST_OP_BC && "仅BC指令有trueLabel");
    return trueTarget;
}

Instruction * BranchInstruction::getFalseTarget() const
{
    // assert(op == IRInstOperator::IRINST_OP_BC && "仅BC指令有falseLabel");
    return falseTarget;
}

Instruction * BranchInstruction::getTarget() const
{
    // assert(op == IRInstOperator::IRINST_OP_BT || op == IRInstOperator::IRINST_OP_BF && "仅BT/BF指令有targetLabel");
    return Target;
}