#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdbool.h>

// Token types
typedef enum {
    // Keywords
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

    // Identifiers 
    IDENT,
    NUMBER,

    // Operators
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

    // Punctuation
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    SEMICOLON, // ;
    COMMA,     // ,

    // Special tokens
    TOKENEOF,
    ERROR
} SymbleType;

// Token structure
typedef struct {
    SymbleType type;
    char lexeme[256];  // The actual text of the token
    int line;          // Line number where token appears
    int column;        // Column number where token appears
} Symble;

// Lexer structure
typedef struct {
    FILE* source;
    char current_char;
    int line;
    int column;
    int has_read_char;
} Lexer;

// Symbol table entry types
typedef enum {
    FUNC,       // Function
    VAR,        // Variable
    PARAM       // Parameter
} ObjectType;

// Symbol table entry
typedef struct {
    char name[256];     // Identifier name
    ObjectType kind;    // Type of the entry
    int address;        // Memory address
    int size;           // Size of data area (for functions)
} TableEntry;

// Symbol table
typedef struct {
    TableEntry entries[100];  // Array of symbol table entries
    int size;                 // Current size of symbol table
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


// Function declarations of lexer
Lexer* lexer_init(FILE* source);
void lexer_free(Lexer* lexer);
Symble get_sym(Lexer* lexer);
const char* symble_type_to_string(SymbleType type);

// Function declarations of parser
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

// code generate
void code_gen(ObjectType op,int value);
instruction code_read(int p);
void code_list(int begin);

void interpret();

// Helper functions for symbol table
void enter(const char* name, ObjectType kind);
int position(const char* name);
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
    16 输出 bool (fprint)
    17 输入 bool (fscan)
    18 次栈顶 &  栈顶 (bool类型)
    19 次栈顶 |  栈顶 (bool类型)
    20 次栈顶 !  栈顶 (bool类型)
    21 函数传参 (栈顶元素放到指定位置)*/