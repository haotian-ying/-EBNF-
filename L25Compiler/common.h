#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdbool.h>

// sym������
typedef enum {
    // �ؼ���
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

    // ��ʶ��
    IDENT,
    NUMBER,

    // �����
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

    // �������
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    SEMICOLON, // ;
    COMMA,     // ,

    // �������
    TOKENEOF,
    ERROR
} SymbleType;

// symble�Ľṹ��
typedef struct {
    SymbleType type;
    char lexeme[256];  // �����ı�
    int line;          // ������Ϣ
    int column;        
} Symble;

// �ʷ�������
typedef struct {
    FILE* source;
    char current_char;
    int line;
    int column;
    int has_read_char;
} Lexer;

// Symbol��Ӧ�ķ��ű�����
typedef enum {
    FUNC,       
    VAR,        
    PARAM       // ��������
} ObjectType;

// ymbol��Ӧ�ķ��ű����
typedef struct {
    char name[256];     
    ObjectType kind;    
    int address;       
    int size;           
} TableEntry;

// Symbol table
typedef struct {
    TableEntry entries[100];  // symbol���ű�
    int size;                 
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


// lexer��������
Lexer* lexer_init(FILE* source);
void lexer_free(Lexer* lexer);
Symble get_sym(Lexer* lexer);
const char* symble_type_to_string(SymbleType type);

// �﷨�������ĺ�������
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

// �м��������
void code_gen(ObjectType op,int value);
instruction code_read(int p);
void code_list(int begin);

void interpret();

// ������غ���
void enter(char* name, ObjectType kind);
int position(char* name);
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
    16 �������� (ջ��Ԫ�طŵ�ָ��λ��)*/