#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdbool.h>

// sym的种类
typedef enum {
    // 关键词
    PROGRAMSYM,
    FUNCSYM,
    MAINSYM,
    LETSYM,
    IFSYM,
    ELSESYM,
    WHILESYM,
    RETURNSYM,
    INPUTSYM,
    OUTPUTSYM,

    // 标识符
    IDENT,
    NUMBER,

    // 运算符
    PLUS,      // +
    MINUS,     // -
    MULTIPLY,  // *
    DIVIDE,    // /
    EQL,        // ==
    NEQ,       // !=
    LSS,        // <
    LEQ,        // <=
    GTR,        // >
    GEQ,        // >=
    BECOMES,    // =

    // 定义相关
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    SEMICOLON, // ;
    COMMA,     // ,

    // 特殊符号
    TOKENEOF,
    ERROR
} SymbleType;

// symble的结构体
typedef struct {
    SymbleType type;
    char lexeme[256];  // 具体文本
    int line;          // 行列信息
    int column;        
} Symble;

// 词法分析器
typedef struct {
    FILE* source;
    char current_char;
    int line;
    int column;
    int has_read_char;
} Lexer;

// Symbol对应的符号表类型
typedef enum {
    FUNC,       
    VAR,        
    PARAM       // 函数参数
} ObjectType;

// ymbol对应的符号表表项
typedef struct {
    char name[256];     
    ObjectType kind;    
    int address;       
    int size;           
} TableEntry;

// Symbol table
typedef struct {
    TableEntry entries[100];  // symbol符号表
    int size;                 
} SymbolTable;

/* Pcode操作码 */
typedef enum {
    lit,          // 立即数赋值
    opr,          // 各种操作
    lod,          // load
    sto,          // store
    cal,          // 函数调用
    ini,          // 开辟栈中空间(为了避免和int重名)
    jmp,          // 无条件跳转
    jpc,          // 条件跳转
    ret
} Operation;

/* pcode指令 */
typedef struct {
    Operation op;       // 操作码
    int value;    // 操作数
} instruction;


// lexer函数定义
Lexer* lexer_init(FILE* source);
void lexer_free(Lexer* lexer);
Symble get_sym(Lexer* lexer);
const char* symble_type_to_string(SymbleType type);

// 语法分析器的函数定义
void pogram();
void func_def();
void param_list();
void stmt_list();
void stmt();
void declare(int* pdx);
void block(int dx, int tx, bool is_main);
void declare_stmt();
void assign_stmt();
void if_stmt();
void while_stmt();
void func_call(int pos);
void arg_list();
void input_stmt();
void output_stmt();
void bool_expr();
void return_stmt();
void expr();
void term();
void factor();

// 中间代码生成
void code_gen(ObjectType op,int value);
instruction code_read(int p);
void code_list(int begin);

void interpret();

// 其他相关函数
void enter(char* name, ObjectType kind);
int position(char* name);
void error(int n);
void print_symbol_table();
#endif 

/*
    opr 指令
    0  调用返回(无返回值)
    1  调用返回(有返回值)
    2  栈顶元素取负
    3  次栈顶 +  栈顶
    4  次栈顶 -  栈顶
    5  次栈顶 *  栈顶
    6  次栈顶 /  栈顶
    7  栈顶顶 %  栈顶
    8  次栈顶 == 栈顶 ? 1 : 0
    9  次栈顶 != 栈顶 ? 1 : 0
    10 次栈顶 <  栈顶 ? 1 : 0
    11 次栈顶 >= 栈顶 ? 1 : 0
    12 次栈顶 >  栈顶 ? 1 : 0
    13 次栈顶 <= 栈顶 ? 1 : 0
    14 输出 int (print)
    15 输入 int (scan)
    16 函数传参 (栈顶元素放到指定位置)*/