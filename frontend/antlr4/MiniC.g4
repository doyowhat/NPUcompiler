grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符statement的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义
compileUnit: (funcDef | varDecl)* EOF;

// 函数定义
funcDef: T_INT T_ID T_L_PAREN T_R_PAREN block;

// 语句块
block: T_L_BRACE blockItemList? T_R_BRACE;

// 块内元素列表
blockItemList: blockItem+;

// 块内元素
blockItem: statement | varDecl;

// 变量声明
varDecl: basicType varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型
basicType: T_INT;

// 变量定义
varDef: T_ID;

statement:
	T_RETURN expr T_SEMICOLON									# returnStatement
	| lVal T_ASSIGN expr T_SEMICOLON							# assignStatement
	| block														# blockStatement
	| expr? T_SEMICOLON											# expressionStatement
	| T_IF T_L_PAREN expr T_R_PAREN statement					# ifStatement
	| T_IF T_L_PAREN expr T_R_PAREN statement T_ELSE statement	# ifElseStatement
	| T_WHILE T_L_PAREN expr T_R_PAREN statement				# whileStatement
	| T_BREAK T_SEMICOLON										# breakStatement
	| T_CONTINUE T_SEMICOLON									# continueStatement;

// 表达式层次结构（按优先级从低到高排列）
expr: logicalOrExp;

logicalOrExp: logicalAndExp (logicalOrOp logicalAndExp)*;
logicalAndExp: equalityExp (logicalAndOp equalityExp)*;
equalityExp: relExp (equalityOp relExp)*;
relExp: addExp (relOp addExp)*;
addExp: mulExp (addOp mulExp)*;
mulExp: unaryExp (mulOp unaryExp)*;

// 新增：运算符定义
logicalOrOp: T_OR;
logicalAndOp: T_AND;
equalityOp: T_EQ | T_NE;
relOp: T_LT | T_GT | T_LE | T_GE;
addOp: T_ADD | T_SUB;
mulOp: T_MUL | T_DIV | T_MOD;

unaryExp:
	T_NOT unaryExp
	| T_SUB unaryExp
	| primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN;

primaryExp:
	T_L_PAREN expr T_R_PAREN
	| T_DIGIT
	| T_OCTAL
	| T_HEX
	| T_TRUE
	| T_FALSE
	| lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式
lVal: T_ID;
LINE_COMMENT: '//' ~[\r\n]* -> skip;
BLOCK_COMMENT: '/*' .*? '*/' -> skip;
// 词法规则
T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';

T_ASSIGN: '=';
T_COMMA: ',';

T_ADD: '+';
T_SUB: '-';
T_MUL: '*';
T_DIV: '/';
T_MOD: '%';

// 关系运算符
T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';
T_EQ: '==';
T_NE: '!=';

// 逻辑运算符
T_AND: '&&';
T_OR: '||';
T_NOT: '!';

// 控制流关键字
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';

// 布尔字面量
T_TRUE: 'true';
T_FALSE: 'false';

// 关键字（需放在T_ID前）
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
T_OCTAL: '0' [1-7][0-7]*;
T_HEX: '0' [xX] [0-9a-fA-F]+;
T_DIGIT: '0' | [1-9][0-9]*;

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;