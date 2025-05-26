#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

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
    int level;          // Nesting level
    int address;        // Memory address
    int size;           // Size of data area (for functions)
    int param_count;    // Number of parameters (for functions)
    int return_type;    // Return type (for functions)
} TableEntry;

// Symbol table
typedef struct {
    TableEntry entries[100];  // Array of symbol table entries
    int size;                 // Current size of symbol table
} SymbolTable;

// Intermediate code instructions
typedef enum {
    LIT,    // Load literal
    OPR,    // Operation
    LOD,    // Load variable
    STO,    // Store variable
    CAL,    // Call function
    INT,    // Initialize stack frame
    JMP,    // Jump
    JPC,    // Jump if false
    RET,    // Return from function
    PAR,    // Parameter passing
} InstructionType;

// Operation codes
typedef enum {
    ADD,    // Addition
    SUB,    // Subtraction
    MUL,    // Multiplication
    DIV,    // Division
    NEG,    // Negation
    ODD,    // Odd test
    EQ,     // Equal
    NE,     // Not equal
    LT,     // Less than
    LE,     // Less or equal
    GT,     // Greater than
    GE,     // Greater or equal
} OperationCode;

// Intermediate code instruction
typedef struct {
    InstructionType type;
    int level;      // Lexical level
    int address;    // Address or value
    OperationCode op; // Operation code for OPR
} Instruction;

// Code generation context
typedef struct {
    Instruction code[1000];  // Array of instructions
    int code_size;          // Current size of code array
    int data_size;          // Current size of data area
    int level;              // Current lexical level
} CodeContext;

// Function declarations of lexer
Lexer* lexer_init(FILE* source);
void lexer_free(Lexer* lexer);
Symble get_sym(Lexer* lexer);
const char* symble_type_to_string(SymbleType type);

// Function declarations of parser
void program(SymbolTable* table);
void func_def(SymbolTable* table);
void param_list(SymbolTable* table);
void stmt_list(SymbolTable* table);
void stmt(SymbolTable* table);
void declare_stmt(SymbolTable* table);
void assign_stmt(SymbolTable* table);
void if_stmt(SymbolTable* table);
void while_stmt(SymbolTable* table);
void func_call(SymbolTable* table);
void arg_list(SymbolTable* table);
void input_stmt(SymbolTable* table);
void output_stmt(SymbolTable* table);
void bool_expr(SymbolTable* table);
void return_stmt(SymbolTable* table);
void expr(SymbolTable* table);
void term(SymbolTable* table);
void factor(SymbolTable* table);

// Helper functions for symbol table
void enter(SymbolTable* table, const char* name, ObjectType kind);
int position(SymbolTable* table, const char* name);
void error(int n);
void print_symbol_table(SymbolTable* table);

// Function declarations for code generation
void gen_instruction(CodeContext* ctx, InstructionType type, int level, int address);
void gen_operation(CodeContext* ctx, OperationCode op);
void gen_function_call(CodeContext* ctx, const char* func_name, int param_count);
void gen_return(CodeContext* ctx, int has_value);

#endif // COMMON_H 