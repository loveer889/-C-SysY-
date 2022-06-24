这是一个用cpp语言编写的编译器。

这个编译器可以把SysY语言编写的程序编译成MIPS汇编代码，汇编代码可以在MARS模拟器上运行。

SysY是C语言的一个子集，其文法如下：

```
编译单元 CompUnit → { ConstDecl | VarDecl } {FuncDef} MainFuncDef 
常量声明 ConstDecl → 'const' 'int' ConstDef { ',' ConstDef } ';' 
常数定义 ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
常量初值 ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}' 
变量声明 VarDecl → 'int' VarDef { ',' VarDef } ';' 
变量定义 VarDef → Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal
变量初值 InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}'
函数定义 FuncDef → FuncType Ident '(' [FuncFParams] ')' Block 
主函数定义 MainFuncDef → 'int' 'main' '(' ')' Block 
函数类型 FuncType → 'void' | 'int'
函数形参表 FuncFParams → FuncFParam { ',' FuncFParam } 
函数形参 FuncFParam → 'int' Ident ['[' ']' { '[' ConstExp ']' }] 
语句块 Block → '{' { ConstDecl | VarDecl | Stmt } '}' 
语句 Stmt → LVal '=' Exp ';' 
 | [Exp] ';' 
 | Block
 | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] 
 | 'while' '(' Cond ')' Stmt
 | 'break' ';' | 'continue' ';'
 | 'return' [Exp] ';' // 1.有Exp 2.⽆Exp
 | LVal '=' 'getint''('')'';'
 | 'printf''('FormatString{','Exp}')'';' 
表达式 Exp → AddExp 
条件表达式 Cond → LOrExp 
左值表达式 LVal → Ident {'[' Exp ']'} 
基本表达式 PrimaryExp → '(' Exp ')' | LVal | Number 
数值 Number → IntConst 
⼀元表达式 UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp 
单⽬运算符 UnaryOp → '+' | '−' | '!' 
函数实参表 FuncRParams → Exp { ',' Exp } 
乘除模表达式 MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp 
加减表达式 AddExp → MulExp { ('+' | '−') MulExp }
关系表达式 RelExp → AddExp { ('<' | '>' | '<=' | '>=') AddExp } 
相等性表达式 EqExp → RelExp { ('==' | '!=') RelExp } 
逻辑与表达式 LAndExp → EqExp { '&&' EqExp }
逻辑或表达式 LOrExp → LAndExp { '||' LAndExp } 
常量表达式 ConstExp → AddExp 
```



整个代码分为三个部分：**词法分析**、**语法分析和中间代码生成**、**代码优化**、**输出**。

**词法分析**

读入源语言代码，把代码分解成若干个token

**语法分析和中间代码生成**

对词法分析给出的单词序列进行语法分析，给出中间代码

**代码优化**

优化中间代码

- 划分基本块
- 到达定义分析&活跃变量分析
- 活跃变量分析
- 图着色寄存器分配
- （换页算法）局部寄存器分配
- 除法降级
- （构造DAG图）公共子表达式删除

**输出**

把中间代码输出为目标语言代码