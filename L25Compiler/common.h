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

/* Pcode������ */
typedef enum {
    lit,          // ��������ֵ
    opr,          // ���ֲ���
    lod,          // load
    sto,          // store
    cal,          // ��������
    ini,          // ����ջ�пռ�(Ϊ�˱����int����)
    jmp,          // ��������ת
    jpc,          // ������ת
    ret
} Operation;

/* pcodeָ�� */
typedef struct {
    Operation op;       // ������
    int value;    // ������
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
    opr ָ��
    0  ���÷���(�޷���ֵ)
    1  ���÷���(�з���ֵ)
    2  ջ��Ԫ��ȡ��
    3  ��ջ�� +  ջ��
    4  ��ջ�� -  ջ��
    5  ��ջ�� *  ջ��
    6  ��ջ�� /  ջ��
    7  ջ���� %  ջ��
    8  ��ջ�� == ջ�� ? 1 : 0
    9  ��ջ�� != ջ�� ? 1 : 0
    10 ��ջ�� <  ջ�� ? 1 : 0
    11 ��ջ�� >= ջ�� ? 1 : 0
    12 ��ջ�� >  ջ�� ? 1 : 0
    13 ��ջ�� <= ջ�� ? 1 : 0
    14 ��� int (print)
    15 ���� int (scan)
    16 ��� bool (fprint)
    17 ���� bool (fscan)
    18 ��ջ�� &  ջ�� (bool����)
    19 ��ջ�� |  ջ�� (bool����)
    20 ��ջ�� !  ջ�� (bool����)
    21 �������� (ջ��Ԫ�طŵ�ָ��λ��)*/