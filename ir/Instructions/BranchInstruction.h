///
/// @file BranchInstruction.h
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
#pragma once

#include <string>

#include "Instruction.h"
#include "LabelInstruction.h"
#include "Function.h"
#include "Value.h"

///
/// @brief 条件跳转指令基类
///
class BranchInstruction final : public Instruction {
public:
    ///
    /// @brief 构造函数（单向条件跳转 bt/bf）
    /// @param _func 所属函数
    /// @param _op 操作符（IRINST_OP_BT/IRINST_OP_BF）
    /// @param _condVar 条件变量（类型为i1）
    /// @param _targetLabel 跳转目标
    ///
    BranchInstruction(Function * _func, IRInstOperator _op, Value * _condVar, Instruction * _target);

    /// @brief 转换成字符串
    void toString(std::string & str) override;

    ///
    /// @brief 获取条件变量
    /// @return Value* 条件变量
    ///
    [[nodiscard]] Value * getCondVar() const;

    ///
    /// @brief 获取跳转目标（适用于BT/BF指令）
    /// @return LabelInstruction* 目标标签
    ///
    [[nodiscard]] Instruction * getTarget() const;

private:
    Value * condVar;      ///< 条件变量（类型为i1）
    Instruction * Target; ///< 跳转目标（仅用于BT/BF指令）
};