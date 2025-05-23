///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.2
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// <tr><td>2025-05-23 <td>1.2     <td>zenglj  <td>添加while、break、continue的中间IR支持
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "BranchInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"
#include "UnaryInstruction.h"

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
    /* 叶子节点 */
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
    ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

    /* 表达式运算， 加减 */
    ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
    ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
    ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
    ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
    ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
    ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;

    /* 逻辑运算 */
    ast2ir_handlers[ast_operator_type::AST_OP_AND] = &IRGenerator::ir_and;
    ast2ir_handlers[ast_operator_type::AST_OP_OR] = &IRGenerator::ir_or;
    ast2ir_handlers[ast_operator_type::AST_OP_NOT] = &IRGenerator::ir_not;

    /*关系运算*/
    ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_eq;
    ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_ne;
    ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_lt;
    ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_le;
    ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_gt;
    ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_ge;
    /* 语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
    ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;
    /*控制流语句*/
    ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
    ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
    ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
    ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;
    /* 函数调用 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

    /* 函数定义 */
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
    ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

    /* 变量定义语句 */
    ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
    ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

    /* 语句块 */
    ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

    /* 编译单元 */
    ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
    ast_node * node;

    // 从根节点进行遍历
    node = ir_visit_ast_node(root);

    return node != nullptr;
}

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
    // 空节点
    if (nullptr == node) {
        return nullptr;
    }

    bool result;

    std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
    pIter = ast2ir_handlers.find(node->node_type);
    if (pIter == ast2ir_handlers.end()) {
        // 没有找到，则说明当前不支持
        result = (this->ir_default)(node);
    } else {
        result = (this->*(pIter->second))(node);
    }

    if (!result) {
        // 语义解析错误，则出错返回
        node = nullptr;
    }

    return node;
}

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
    // 未知的节点
    printf("Unkown node(%d)\n", (int) node->node_type);
    return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
    module->setCurrentFunction(nullptr);

    for (auto son: node->sons) {

        // 遍历编译单元，要么是函数定义，要么是语句
        ast_node * son_node = ir_visit_ast_node(son);
        if (!son_node) {
            // TODO 自行追加语义错误处理
            return false;
        }
    }

    return true;
}

std::string IRGenerator::generateLabel()
{
    // 生成一个新的Label
    static int labelCount = 0;
    std::string label = "L" + std::to_string(labelCount++);
    return label;
}
/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
    bool result;

    // 创建一个函数，用于当前函数处理
    if (module->getCurrentFunction()) {
        // 函数中嵌套定义函数，这是不允许的，错误退出
        // TODO 自行追加语义错误处理
        return false;
    }

    // 函数定义的AST包含四个孩子
    // 第一个孩子：函数返回类型
    // 第二个孩子：函数名字
    // 第三个孩子：形参列表
    // 第四个孩子：函数体即block
    ast_node * type_node = node->sons[0];
    ast_node * name_node = node->sons[1];
    ast_node * param_node = node->sons[2];
    ast_node * block_node = node->sons[3];

    // 创建一个新的函数定义
    Function * newFunc = module->newFunction(name_node->name, type_node->type);
    if (!newFunc) {
        // 新定义的函数已经存在，则失败返回。
        // TODO 自行追加语义错误处理
        return false;
    }

    // 当前函数设置有效，变更为当前的函数
    module->setCurrentFunction(newFunc);

    // 进入函数的作用域
    module->enterScope();

    // 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
    InterCode & irCode = newFunc->getInterCode();

    // 这里也可增加一个函数入口Label指令，便于后续基本块划分

    // 创建并加入Entry入口指令
    irCode.addInst(new EntryInstruction(newFunc));

    // 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
    LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);

    // 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
    newFunc->setExitLabel(exitLabelInst);

    // 遍历形参，没有IR指令，不需要追加
    result = ir_function_formal_params(param_node);
    if (!result) {
        // 形参解析失败
        // TODO 自行追加语义错误处理
        return false;
    }
    node->blockInsts.addInst(param_node->blockInsts);

    // 新建一个Value，用于保存函数的返回值，如果没有返回值可不用申请
    LocalVariable * retValue = nullptr;
    if (!type_node->type->isVoidType()) {

        // 保存函数返回值变量到函数信息中，在return语句翻译时需要设置值到这个变量中
        retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
    }
    newFunc->setReturnValue(retValue);

    // 这里最好设置返回值变量的初值为0，以便在没有返回值时能够返回0

    // 函数内已经进入作用域，内部不再需要做变量的作用域管理
    block_node->needScope = false;

    // 遍历block
    result = ir_block(block_node);
    if (!result) {
        // block解析失败
        // TODO 自行追加语义错误处理
        return false;
    }

    // IR指令追加到当前的节点中
    node->blockInsts.addInst(block_node->blockInsts);

    // node节点的指令移动到函数的IR指令列表中
    irCode.addInst(node->blockInsts);

    // 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
    irCode.addInst(exitLabelInst);

    // 函数出口指令
    irCode.addInst(new ExitInstruction(newFunc, retValue));

    // 恢复成外部函数
    module->setCurrentFunction(nullptr);

    // 退出函数的作用域
    module->leaveScope();

    return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
    // TODO 目前形参还不支持，直接返回true

    // 每个形参变量都创建对应的临时变量，用于表达实参转递的值
    // 而真实的形参则创建函数内的局部变量。
    // 然后产生赋值指令，用于把表达实参值的临时变量拷贝到形参局部变量上。
    // 请注意这些指令要放在Entry指令后面，因此处理的先后上要注意。

    return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
    std::vector<Value *> realParams;

    // 获取当前正在处理的函数
    Function * currentFunc = module->getCurrentFunction();

    // 函数调用的节点包含两个节点：
    // 第一个节点：函数名节点
    // 第二个节点：实参列表节点

    std::string funcName = node->sons[0]->name;
    int64_t lineno = node->sons[0]->line_no;

    ast_node * paramsNode = node->sons[1];

    // 根据函数名查找函数，看是否存在。若不存在则出错
    // 这里约定函数必须先定义后使用
    auto calledFunction = module->findFunction(funcName);
    if (nullptr == calledFunction) {
        minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
        return false;
    }

    // 当前函数存在函数调用
    currentFunc->setExistFuncCall(true);

    // 如果没有孩子，也认为是没有参数
    if (!paramsNode->sons.empty()) {

        int32_t argsCount = (int32_t) paramsNode->sons.size();

        // 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
        // 因为目前的语言支持的int和float都是四字节的，只统计个数即可
        if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
            currentFunc->setMaxFuncCallArgCnt(argsCount);
        }

        // 遍历参数列表，孩子是表达式
        // 这里自左往右计算表达式
        for (auto son: paramsNode->sons) {

            // 遍历Block的每个语句，进行显示或者运算
            ast_node * temp = ir_visit_ast_node(son);
            if (!temp) {
                return false;
            }

            realParams.push_back(temp->val);
            node->blockInsts.addInst(temp->blockInsts);
        }
    }

    // TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
    if (realParams.size() != calledFunction->getParams().size()) {
        // 函数参数的个数不一致，语义错误
        minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
        return false;
    }

    // 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
    Type * type = calledFunction->getReturnType();

    FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

    // 创建函数调用指令
    node->blockInsts.addInst(funcCallInst);

    // 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
    node->val = funcCallInst;

    return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
    // 进入作用域
    if (node->needScope) {
        module->enterScope();
    }

    std::vector<ast_node *>::iterator pIter;
    for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

        // 遍历Block的每个语句，进行显示或者运算
        ast_node * temp = ir_visit_ast_node(*pIter);
        if (!temp) {
            return false;
        }

        node->blockInsts.addInst(temp->blockInsts);
    }

    // 离开作用域
    if (node->needScope) {
        module->leaveScope();
    }

    return true;
}

bool IRGenerator::ir_if(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    std::string true_label = generateLabel();  // 真分支标签
    std::string false_label = generateLabel(); // 假分支标签
    std::string end_label = generateLabel();   // 结束标签

    // 条件表达式（此处假设true为常量1，false为0）
    ast_node * condNode = node->sons[0];
    ir_visit_ast_node(condNode);

    // 创建标签对象
    auto trueLabelInst = new LabelInstruction(currentFunc, true_label);
    auto falseLabelInst = new LabelInstruction(currentFunc, false_label);
    auto endLabelInst = new LabelInstruction(currentFunc, end_label);

    // 生成 BF 指令：如果条件为假，跳转到假分支
    node->blockInsts.addInst(condNode->blockInsts);
    node->blockInsts.addInst(
        new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BF, condNode->val, falseLabelInst));

    // 真分支
    node->blockInsts.addInst(trueLabelInst);
    ir_visit_ast_node(node->sons[1]);
    node->blockInsts.addInst(node->sons[1]->blockInsts);
    // 真分支语句（a=48）
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabelInst)); // 跳转到结束标签

    // 假分支
    node->blockInsts.addInst(falseLabelInst);
    if (node->sons.size() > 2) {
        ir_visit_ast_node(node->sons[2]);
        node->blockInsts.addInst(node->sons[2]->blockInsts);
    }
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabelInst)); // 跳转到结束标签

    // 结束标签
    node->blockInsts.addInst(endLabelInst);
    return true;
}

bool IRGenerator::ir_while(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    std::string loop_entry_label = generateLabel();
    std::string loop_body_label = generateLabel();
    std::string loop_exit_label = generateLabel();

    loop_contexts.push({loop_entry_label, loop_body_label, loop_exit_label});

    // 循环入口标签
    auto loopEntryLabel = new LabelInstruction(currentFunc, loop_entry_label);
    node->blockInsts.addInst(loopEntryLabel);

    // 条件表达式
    ast_node * condNode = node->sons[0];
    ir_visit_ast_node(condNode);
    node->blockInsts.addInst(condNode->blockInsts);
    // 生成BT指令：条件为真时跳转到循环体
    auto loopBodyLabel = new LabelInstruction(currentFunc, loop_body_label);
    node->blockInsts.addInst(
        new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BT, condNode->val, loopBodyLabel));

    // 循环出口标签（条件为假时直接到达）
    auto loopExitLabel = new LabelInstruction(currentFunc, loop_exit_label);
    node->blockInsts.addInst(loopExitLabel);

    // 循环体入口标签
    node->blockInsts.addInst(loopBodyLabel);
    ir_visit_ast_node(node->sons[1]); // 循环体
    node->blockInsts.addInst(node->sons[1]->blockInsts);

    // 无条件跳转到循环条件判断
    node->blockInsts.addInst(new GotoInstruction(currentFunc, loopEntryLabel));

    loop_contexts.pop();
    return true;
}

bool IRGenerator::ir_break(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc || loop_contexts.empty()) {
        minic_log(LOG_ERROR, "break语句不在循环内部");
        return false;
    }

    // 获取循环出口标签
    const LoopContext & context = loop_contexts.top();
    auto loopExitLabel = new LabelInstruction(currentFunc, context.loop_exit_label);

    // 生成跳转到出口标签的指令
    node->blockInsts.addInst(new GotoInstruction(currentFunc, loopExitLabel));
    return true;
}

bool IRGenerator::ir_continue(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc || loop_contexts.empty()) {
        minic_log(LOG_ERROR, "continue语句不在循环内部");
        return false;
    }

    // 获取循环入口标签
    const LoopContext & context = loop_contexts.top();
    auto loopEntryLabel = new LabelInstruction(currentFunc, context.loop_entry_label);

    // 生成跳转到入口标签的指令
    node->blockInsts.addInst(new GotoInstruction(currentFunc, loopEntryLabel));
    return true;
}

bool IRGenerator::ir_and(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    std::string left_true_label = generateLabel();
    std::string left_false_label = generateLabel();
    std::string end_label = generateLabel();

    // 左操作数：计算后若为假，跳转到false_label
    ast_node * left = node->sons[0];
    ir_visit_ast_node(left);
    node->blockInsts.addInst(left->blockInsts);
    auto falseLabel = new LabelInstruction(currentFunc, left_false_label);
    node->blockInsts.addInst(new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BF, left->val, falseLabel));

    // 左操作数为真，计算右操作数
    auto trueLabel = new LabelInstruction(currentFunc, left_true_label);
    node->blockInsts.addInst(trueLabel);
    auto endLabel = new LabelInstruction(currentFunc, end_label);
    ast_node * right = node->sons[1];
    ir_visit_ast_node(right);
    node->blockInsts.addInst(right->blockInsts);
    // 右操作数若为假，跳转到false_label
    node->blockInsts.addInst(new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BF, right->val, falseLabel));

    // 结果为真
    node->blockInsts.addInst(trueLabel); // 复用左操作数的trueLabel
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    node->blockInsts.addInst(new MoveInstruction(currentFunc, result, new ConstInt(1)));
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));

    // 结果为假
    node->blockInsts.addInst(falseLabel);
    node->blockInsts.addInst(new MoveInstruction(currentFunc, result, new ConstInt(0)));
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));

    // 结束标签
    node->blockInsts.addInst(new LabelInstruction(currentFunc, end_label));
    node->val = result;
    return true;
}

bool IRGenerator::ir_or(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    std::string left_true_label = generateLabel();
    std::string left_false_label = generateLabel();
    std::string end_label = generateLabel();

    // 左操作数：计算后若为真，跳转到true_label
    ast_node * left = node->sons[0];
    ir_visit_ast_node(left);
    node->blockInsts.addInst(left->blockInsts);
    auto trueLabel = new LabelInstruction(currentFunc, left_true_label);
    node->blockInsts.addInst(new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BT, left->val, trueLabel));

    // 左操作数为假，计算右操作数
    auto falseLabel = new LabelInstruction(currentFunc, left_false_label);
    auto endLabel = new LabelInstruction(currentFunc, end_label);
    node->blockInsts.addInst(falseLabel);
    ast_node * right = node->sons[1];
    ir_visit_ast_node(right);
    node->blockInsts.addInst(right->blockInsts);
    // 右操作数若为真，跳转到true_label
    node->blockInsts.addInst(new BranchInstruction(currentFunc, IRInstOperator::IRINST_OP_BT, right->val, trueLabel));

    // 结果为真
    node->blockInsts.addInst(trueLabel);
    LocalVariable * result = static_cast<LocalVariable *>(module->newVarValue(IntegerType::getTypeInt()));
    node->blockInsts.addInst(new MoveInstruction(currentFunc, result, new ConstInt(1)));
    node->blockInsts.addInst(new GotoInstruction(currentFunc, endLabel));

    // 结果为假
    node->blockInsts.addInst(new LabelInstruction(currentFunc, end_label));
    node->blockInsts.addInst(new MoveInstruction(currentFunc, result, new ConstInt(0)));

    node->val = result;
    return true;
}

bool IRGenerator::ir_not(ast_node * node)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;
    // 交换真假出口标签
    std::swap(node->true_label, node->false_label);
    // 递归处理操作数
    return ir_visit_ast_node(node->sons[0]);
}

// 等于（==）
bool IRGenerator::ir_eq(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_EQ_I);
}

// 不等于（!=）
bool IRGenerator::ir_ne(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_NE_I);
}

// 小于（<）
bool IRGenerator::ir_lt(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_LT_I);
}

// 大于（>）
bool IRGenerator::ir_gt(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_GT_I);
}

// 小于等于（<=）
bool IRGenerator::ir_le(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_LE_I);
}

// 大于等于（>=）
bool IRGenerator::ir_ge(ast_node * node)
{
    return ir_relop(node, IRInstOperator::IRINST_OP_GE_I);
}

bool IRGenerator::ir_relop(ast_node * node, IRInstOperator op)
{
    Function * currentFunc = module->getCurrentFunction();
    if (!currentFunc)
        return false;

    ast_node * left = ir_visit_ast_node(node->sons[0]);
    ast_node * right = ir_visit_ast_node(node->sons[1]);
    if (!left || !right)
        return false;

    // 生成比较指令，直接将指令作为结果值
    BinaryInstruction * cmpInst =
        new BinaryInstruction(currentFunc, op, left->val, right->val, IntegerType::getTypeInt());

    // 将子节点的指令和当前比较指令添加到block中
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(cmpInst);

    // 直接将指令作为结果值
    node->val = cmpInst;

    return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * addInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_ADD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(addInst);

    node->val = addInst;

    return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
    ast_node * src1_node = node->sons[0];
    ast_node * src2_node = node->sons[1];

    // 加法节点，左结合，先计算左节点，后计算右节点

    // 加法的左边操作数
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left) {
        // 某个变量没有定值
        return false;
    }

    // 加法的右边操作数
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    BinaryInstruction * subInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_SUB_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(subInst);

    node->val = subInst;

    return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
    ast_node *src1_node = node->sons[0], *src2_node = node->sons[1];
    //乘法运算
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left)
        return false;
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right)
        return false;

    BinaryInstruction * mulInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MUL_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(mulInst);
    node->val = mulInst;
    return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
    ast_node *src1_node = node->sons[0], *src2_node = node->sons[1];
    //除法运算
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left)
        return false;
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right)
        return false;

    BinaryInstruction * divInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_DIV_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(divInst);
    node->val = divInst;
    return true;
}

/// @brief 整数取模AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
    ast_node *src1_node = node->sons[0], *src2_node = node->sons[1];
    //取模运算
    ast_node * left = ir_visit_ast_node(src1_node);
    if (!left)
        return false;
    ast_node * right = ir_visit_ast_node(src2_node);
    if (!right)
        return false;

    BinaryInstruction * modInst = new BinaryInstruction(module->getCurrentFunction(),
                                                        IRInstOperator::IRINST_OP_MOD_I,
                                                        left->val,
                                                        right->val,
                                                        IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(modInst);
    node->val = modInst;
    return true;
}

/// @brief 整数取负AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_neg(ast_node * node)
{
    ast_node * src_node = node->sons[0];
    if (!src_node)
        return false;

    ast_node * operand = ir_visit_ast_node(src_node);
    if (!operand)
        return false;
    UnaryInstruction * negInst = new UnaryInstruction(module->getCurrentFunction(),
                                                      IRInstOperator::IRINST_OP_NEG_I,
                                                      operand->val,
                                                      IntegerType::getTypeInt());
    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(operand->blockInsts);
    node->blockInsts.addInst(negInst);
    node->val = negInst;
    return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
    ast_node * son1_node = node->sons[0];
    ast_node * son2_node = node->sons[1];

    // 赋值节点，自右往左运算

    // 赋值运算符的左侧操作数
    ast_node * left = ir_visit_ast_node(son1_node);
    if (!left) {
        // 某个变量没有定值
        // 这里缺省设置变量不存在则创建，因此这里不会错误
        return false;
    }

    // 赋值运算符的右侧操作数
    ast_node * right = ir_visit_ast_node(son2_node);
    if (!right) {
        // 某个变量没有定值
        return false;
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

    MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

    // 创建临时变量保存IR的值，以及线性IR指令
    node->blockInsts.addInst(right->blockInsts);
    node->blockInsts.addInst(left->blockInsts);
    node->blockInsts.addInst(movInst);

    // 这里假定赋值的类型是一致的
    node->val = movInst;

    return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
    ast_node * right = nullptr;

    // return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
    if (!node->sons.empty()) {

        ast_node * son_node = node->sons[0];

        // 返回的表达式的指令保存在right节点中
        right = ir_visit_ast_node(son_node);
        if (!right) {

            // 某个变量没有定值
            return false;
        }
    }

    // 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
    Function * currentFunc = module->getCurrentFunction();

    // 返回值存在时则移动指令到node中
    if (right) {

        // 创建临时变量保存IR的值，以及线性IR指令
        node->blockInsts.addInst(right->blockInsts);

        // 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
        node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

        node->val = right->val;
    } else {
        // 没有返回值
        node->val = nullptr;
    }

    // 跳转到函数的尾部出口指令上
    node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

    return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
    // 不需要做什么，直接从节点中获取即可。

    return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
    Value * val;

    // 查找ID型Value
    // 变量，则需要在符号表中查找对应的值

    val = module->findVarValue(node->name);

    node->val = val;

    return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
    ConstInt * val;

    // 新建一个整数常量Value
    val = module->newConstInt((int32_t) node->integer_val);

    node->val = val;

    return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
    bool result = false;

    for (auto & child: node->sons) {

        // 遍历每个变量声明
        result = ir_variable_declare(child);
        if (!result) {
            break;
        }
    }

    return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
    // 共有两个孩子，第一个类型，第二个变量名

    // TODO 这里可强化类型等检查

    node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

    return true;
}