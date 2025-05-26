/* 核心组件: 语法分析器 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"


table_entry table[128];        // 符号表 
int ptable;                    // 符号表的尾指针

bool declbegsys[symnum];       // 表示声明开始的符号集合
bool facbegsys[symnum];        // 表示因子开始的符号集合

/* 来自中间代码生成器 code.c */

extern int insptr;             // pcode指令指针
extern instruction code[512];  // pcode指令数组

/* 来自词法分析器 lex.c */

extern char   cur_sym[16];     // 当前标识符或者保留字
extern int    cur_num;         // 当前数字
extern symbol cur_symbol;      // 当前单词的类别

static void expression(int lev);

// 生成一个偏移量放在栈顶
static void locate(int lev, int pos)
{
    int id = 0, weight = 1;

    code_gen(lit, 0, 0); // offset = 0

    while (1) {

        lex_next(); // [
        if (cur_symbol != lpar) break;

        lex_next();
        expression(lev);
        assert(cur_symbol == rpar, "Except ']'\n"); // ]

        weight = 1;
        for (int i = 4; i > id; i--) {
            if (table[pos].len[i] == 0) continue;
            weight *= table[pos].len[i];
        }
        id++;

        code_gen(lit, 0, weight);
        code_gen(opr, 0, 5); // *
        code_gen(opr, 0, 3); // +

        // offset = offset + expr * weight
    }
}

// 在符号表中加入一个表项
static void enter(object kind, int lev, int* pdx)
{
    ptable++;
    table[ptable].kind = kind;
    strcpy(table[ptable].name, cur_sym);

    if (kind == procedure) {
        table[ptable].lev = lev;
        return;
    }

    table[ptable].lev = lev;
    table[ptable].adr = (*pdx);

    int len = 1, id = 0;
    while (1) {
        lex_next();
        if (cur_symbol == comma || cur_symbol == semicolon) {
            break;
        }
        else if (cur_symbol == lpar) {
            lex_next();
            assert(cur_symbol == number && cur_num > 0, "Except number > 0\n");

            table[ptable].len[id++] = cur_num;
            len = len * cur_num;
            assert(len <= 128, "Too big len\n");
            assert(id <= 4, "Too many level\n");

            lex_next();
            assert(cur_symbol == rpar, "Except ']'\n");
        }
        else {
            panic("Except ',' or ';' or '['\n");
        }
    }
    table[ptable].len[id] = 0;
    (*pdx) += len;
}

// 返回标识符id在符号表中的位置(倒序查询)
static int position(char* id)
{
    for (int i = ptable; i > 0; i--) {
        if (strcmp(table[i].name, id) == 0)
            return i;
    }
    panic("Unkown ident\n");
    return -1;
}

// 变量声明
static void var_declare(symbol kind, int lev, int* pdx)
{
    if (kind == intsym)
        enter(var_int, lev, pdx);
    else if (kind == boolsym)
        enter(var_bool, lev, pdx);
    else
        panic("Unknown op\n");
}

// 因子处理
// ident  (m, n, p, q....)
// number (1, 22, 0, 99)
// true false
// (.....)
static void factor(int lev)
{
    int pos;

    while (facbegsys[cur_symbol]) {

        if (cur_symbol == ident) {          // 因子为变量

            pos = position(cur_sym);

            if (table[pos].kind == var_int || table[pos].kind == var_bool) {

                locate(lev, pos);

                code_gen(lod, lev - table[pos].lev, table[pos].adr);

            }
            else {
                panic("Except not procedure\n");
            }

        }
        else if (cur_symbol == number) {  // 因子为数字

            code_gen(lit, 0, cur_num);
            lex_next();

        }
        else if (cur_symbol == truesym) {  // bool 类型 true

            code_gen(lit, 0, 1);
            lex_next();

        }
        else if (cur_symbol == falsesym) { // bool 类型 false

            code_gen(lit, 0, 0);
            lex_next();

        }
        else if (cur_symbol == lparen) {  // 因子为括号内的表达式 

            lex_next();
            expression(lev);
            assert(cur_symbol == rparen, "Except right paren\n");
            lex_next();

        }
        else {                           // 非法情况 
            panic("Unkown factor\n");
        }
    }
}

// 项处理
// factor
// factor * factor
// factor / factor
// factor % factor
static void term(int lev)
{
    symbol op;

    // 第一个factor
    factor(lev);

    // 后续factor
    while (cur_symbol == times || cur_symbol == slash || cur_symbol == mod)
    {
        op = cur_symbol;

        lex_next();
        factor(lev);

        if (op == times)
            code_gen(opr, 0, 5);
        else if (op == slash)
            code_gen(opr, 0, 6);
        else
            code_gen(opr, 0, 7);
    }
}

// 表达式处理2
// (+/-/null) term
// (+/-/null) term + term
// (+/-/null) term - term
static void small_expression(int lev)
{
    symbol op;

    // 第一个term的处理 (考虑负数情况)
    if (cur_symbol == plus || cur_symbol == minus) { // 表达式2以正号或者负号开头

        op = cur_symbol;     // 保存开头的符号

        lex_next();
        term(lev);           // 处理项

        if (op = minus)       // 生成取负指令
            code_gen(opr, 0, 2);

    }
    else {

        term(lev);             // 处理项

    }

    // 后面term的处理
    while (cur_symbol == plus || cur_symbol == minus)
    {
        op = cur_symbol;

        lex_next();
        term(lev);

        // 生成加减指令
        if (op == plus)
            code_gen(opr, 0, 3);
        else
            code_gen(opr, 0, 4);
    }

}

// 表达式处理
// small_expression
// ! small_expression
// small_expression & small_expression
// small_expression | small_expression 
static void expression(int lev)
{
    if (cur_symbol == not) {
        lex_next();
        small_expression(lev);
        code_gen(opr, 0, 20);
        return;
    }

    symbol op;

    small_expression(lev);

    while (cur_symbol == and || cur_symbol == or)
    {
        op = cur_symbol;

        lex_next();
        small_expression(lev);

        if (op == and)
            code_gen(opr, 0, 18);
        else
            code_gen(opr, 0, 19);
    }
}

// 条件判断2 
// expr >= expr
// expr <= expr
// expr == expr
// expr != expr
// ............
static void small_condition(int lev)
{
    expression(lev);  // 第一个表达式

    symbol op = cur_symbol;

    lex_next();
    expression(lev);  // 第二个式子

    switch (op)
    {
    case eql:
        code_gen(opr, 0, 8);
        break;

    case neq:
        code_gen(opr, 0, 9);
        break;

    case lss:
        code_gen(opr, 0, 10);
        break;

    case geq:
        code_gen(opr, 0, 11);
        break;

    case gtr:
        code_gen(opr, 0, 12);
        break;

    case leq:
        code_gen(opr, 0, 13);
        break;
    }
}

// 条件判断
// small_condition 
// !(small_condition)
// (small_condition) & (small_condition)
// (small_condition) | (small_condition)
static void condition(int lev)
{
    if (cur_symbol == not) {

        lex_next();
        assert(cur_symbol == lparen, "Except '('\n");

        lex_next();
        small_condition(lev);

        assert(cur_symbol == rparen, "Except ')'\n");

        code_gen(opr, 0, 20);

        lex_next();

    }
    else if (cur_symbol == lparen) {

        symbol op;

        lex_next();
        small_condition(lev);

        assert(cur_symbol == rparen, "Except )\n");
        lex_next();

        op = cur_symbol;

        lex_next();
        assert(cur_symbol == lparen, "Except (\n");

        lex_next();
        small_condition(lev);

        assert(cur_symbol == rparen, "Except )\n");

        if (op == and)
            code_gen(opr, 0, 19);
        else
            code_gen(opr, 0, 20);

        lex_next();

    }
    else {

        small_condition(lev);

    }
}

// 函数调用处理
static void call_handle(int lev, int pos)
{
    assert(table[pos].kind == procedure, "Except procedure\n");

    lex_next();
    assert(cur_symbol == lparen, "Except lparen\n");

    // 参数传递
    while (1) {
        lex_next();
        if (cur_symbol == rparen) break;
        expression(lev);
        code_gen(opr, 0, 21);
        if (cur_symbol == comma) continue;
        else if (cur_symbol == rparen) break;
        else panic("Except ',' or ')'\n");
    }

    // 函数调用
    code_gen(cal, lev - table[pos].lev, table[pos].adr);

    lex_next();
    assert(cur_symbol == semicolon, "Except ';'\n");
}

// 计算语句
static void statement(int lev)
{
    int pos, pos2, insptr0, insptr1;

    switch (cur_symbol)
    {
    case mainsym: // 读到了main 后面应当是一组复合语句 {......};
        lex_next();
        assert(cur_symbol == begin, "Except '{'\n");
        statement(lev);
        break;

    case ident: // 赋值语句处理 ident = expression;

        pos = position(cur_sym);

        locate(lev, pos); // offset
        assert(cur_symbol == becomes, "Except '='\n");

        lex_next();
        expression(lev);  // value
        assert(cur_symbol == semicolon, "Except ';'\n");

        code_gen(sto, lev - table[pos].lev, table[pos].adr);

        lex_next();
        break;

    case readsym: // 读指令处理 int

        lex_next(); // '('
        assert(cur_symbol == lparen, "Except '(' \n");

        do {
            lex_next();
            assert(cur_symbol == ident, "Except ident\n");

            pos = position(cur_sym);
            assert(table[pos].kind == var_int, "Except int\n");

            locate(lev, pos);         // offset

            code_gen(opr, 0, 15);     // value

            code_gen(sto, lev - table[pos].lev, table[pos].adr);

        } while (cur_symbol == comma);

        assert(cur_symbol == rparen, "Except ')' \n");

        lex_next();
        assert(cur_symbol == semicolon, "Except ';'\n");

        lex_next();
        break;

    case writesym: // 写指令处理 int

        lex_next();
        assert(cur_symbol == lparen, "Except '(' \n");

        do {
            lex_next();
            expression(lev);

            code_gen(opr, 0, 14);

        } while (cur_symbol == comma);

        assert(cur_symbol == rparen, "Except ')' \n");

        lex_next();
        assert(cur_symbol == semicolon, "Except ';'\n");

        lex_next();
        break;

    case breadsym: // 读指令处理 bool

        lex_next(); // '('
        assert(cur_symbol == lparen, "Except '(' \n");

        do {
            lex_next();
            assert(cur_symbol == ident, "Except ident\n");

            pos = position(cur_sym);
            assert(table[pos].kind == var_bool, "Except int\n");

            locate(lev, pos);        // offset

            code_gen(opr, 0, 17);    // value

            code_gen(sto, lev - table[pos].lev, table[pos].adr);

        } while (cur_symbol == comma);

        assert(cur_symbol == rparen, "Except ')' \n");

        lex_next();
        assert(cur_symbol == semicolon, "Except ';'\n");
        lex_next();
        break;

    case bwritesym: // 写指令处理 bool

        lex_next();
        assert(cur_symbol == lparen, "Except '(' \n");

        do {
            lex_next();
            expression(lev);

            code_gen(opr, 0, 16);

        } while (cur_symbol == comma);

        assert(cur_symbol == rparen, "Except ')' \n");

        lex_next();
        assert(cur_symbol == semicolon, "Except ';'\n");
        lex_next();
        break;

    case returnsym: // 函数返回 return expression; | return ;

        lex_next();
        if (cur_symbol == semicolon) {

            code_gen(opr, 0, 0); // 无返回值的调用

        }
        else {

            code_gen(lit, 0, 0); // offset

            expression(lev);     // value

            code_gen(sto, 0, 3); // 把表达式的值放到返回值的位置

            code_gen(opr, 0, 1); // 调用返回(有返回值)
        }
        assert(cur_symbol == semicolon, "Except ';'\n");
        lex_next();
        break;

    case callsym: // 接受函数返回值 call ident = ident(a, b, c); | call ident(a, b, c);

        lex_next();
        assert(cur_symbol == ident, "Except ident\n");

        pos = position(cur_sym);

        if (table[pos].kind != procedure) {

            locate(pos, lev); // 在栈顶放置一个offset
            assert(cur_symbol == becomes, "Except '='\n");

            lex_next();
            pos2 = position(cur_sym);
            call_handle(lev, pos2); // 获得value

            code_gen(sto, lev - table[pos].lev, table[pos].adr); // 完成存储操作

        }
        else {

            call_handle(lev, pos);

        }

        lex_next();
        break;

    case ifsym: // if语句处理

        lex_next();
        assert(cur_symbol == lparen, "Except '('\n");

        lex_next();
        condition(lev);
        assert(cur_symbol == rparen, "Except ')'\n");

        insptr0 = insptr;
        code_gen(jpc, 0, 0);          // 跳过 if statement

        lex_next();
        statement(lev);

        if (cur_symbol == elsesym) {  // 遇到 else

            insptr1 = insptr;
            code_gen(jmp, 0, 0);     // 跳过 else statement

            code[insptr0].value = insptr; // jpc 代码回填

            lex_next();
            statement(lev);

            code[insptr1].value = insptr; // jmp 代码回填

        }
        else {
            code[insptr0].value = insptr; // jpc 代码回填
        }

        break;

    case whilesym: // while语句处理

        insptr0 = insptr;

        lex_next();
        assert(cur_symbol == lparen, "Except '('\n");

        lex_next();
        condition(lev);
        assert(cur_symbol == rparen, "Except ')'\n");

        insptr1 = insptr;
        code_gen(jpc, 0, 0);

        lex_next();
        statement(lev);

        code_gen(jmp, 0, insptr0);

        code[insptr1].value = insptr;

        break;

    case forsym: // for语句处理

        int insptr2, insptr3;
        // 循环头
        lex_next();
        assert(cur_symbol == lparen, "Except '('\n");

        lex_next();
        statement(lev);

        insptr3 = insptr;
        condition(lev);
        assert(cur_symbol == semicolon, "Except ';'\n");

        insptr0 = insptr;           // 这个位置是条件判断
        code_gen(jpc, 0, 0);        // 条件不满足则离开循环 jpc等待回填

        insptr1 = insptr;
        code_gen(jmp, 0, 0);        // 条件满足则跳过迭代部分 jmp等待回填

        insptr2 = insptr;
        lex_next();
        statement(lev);             // 迭代
        code_gen(jmp, 0, insptr3);  // 迭代结束回到条件判断

        code[insptr1].value = insptr; // jmp回填

        // 循环体
        lex_next();
        assert(cur_symbol == begin, "Except '{'\n");
        statement(lev);

        code_gen(jmp, 0, insptr2);    // 循环体结束进入迭代
        code[insptr0].value = insptr; // jpc 回填
        break;

    case begin: // 复合语句处理
        lex_next();
        while (cur_symbol != end)
            statement(lev);
        lex_next();
        break;

    default:
        panic("Unkown statement\n");
    }

    // assert(cur_symbol == semicolon, "Except ';'\n");
}

// 声明语句
static void declare(int lev, int* pdx)
{
    symbol kind;            // 变量使用
    bool have_return;       // 函数使用
    int argv, tx;           // 函数使用

    while (declbegsys[cur_symbol]) {      // 声明 

        while (cur_symbol == intsym || cur_symbol == boolsym) {  // 变量声明处理

            kind = cur_symbol;

            do {
                lex_next();
                assert(cur_symbol == ident, "Except ident\n");
                var_declare(kind, lev, pdx);
            } while (cur_symbol == comma);
            assert(cur_symbol == semicolon, "Except semicolon\n");

            lex_next();
        }

        while (cur_symbol == procsym) {    // 函数声明处理

            // 函数类型 void int bool
            lex_next();
            if (cur_symbol == voidsym)
                have_return = false;
            else if (cur_symbol == intsym || cur_symbol == boolsym)
                have_return = true;
            else
                panic("Except int or bool or void\n");

            // 函数名 ident
            lex_next();
            assert(cur_symbol == ident, "Except ident\n");
            enter(procedure, lev, pdx);


            // 左括号
            lex_next();
            assert(cur_symbol == lparen, "Except left paren\n");

            // 函数参数 int a; int b; bool c;
            argv = 4;
            tx = ptable;
            lex_next();
            declare(lev + 1, &argv);
            assert(cur_symbol == rparen, "Except right paren\n");

            // begin
            lex_next();
            assert(cur_symbol == begin, "Except '{'\n");

            // 调用过程处理函数
            lex_next();
            block(lev + 1, argv, tx, have_return);

            // end
            assert(cur_symbol == end, "Except '}'\n");

            // 继续往后
            lex_next();
        }
    }
}

// 初始化
void block_init()
{
    ptable = 0;

    /* 设置符号集 */
    for (int i = 0; i < symnum; i++) {
        declbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[boolsym] = true;
    declbegsys[intsym] = true;
    declbegsys[structsym] = true;
    declbegsys[procsym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[truesym] = true;
    facbegsys[falsesym] = true;
    facbegsys[lparen] = true;
    // facbegsys[not] = true;
}

// 函数内的语法分析
void block(int lev, int dx, int tx, bool have_return)
{
    // dx = 栈内空间 SL + DL + RA + 返回值 + 参数

    int insptr0 = insptr;

    code_gen(jmp, 0, 0);                 // 生成跳转指令

    declare(lev, &dx);                   // step-1 处理函数主体之前的声明部分

    code[insptr0].value = insptr;        // 回填跳转位置

    table[tx].adr = insptr;             // 函数起始位置

    code_gen(ini, 0, dx);                // 开辟空间

    statement(lev);                      // step-2 函数主体语句处理

    if (have_return)
        code_gen(opr, 0, 1);             // 有返回值函数退栈
    else
        code_gen(opr, 0, 0);             // 无返回值函数退栈
}