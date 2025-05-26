#include "common.h"
#include <stdlib.h>
#include <string.h>

// Global variables
extern Lexer* lexer;
extern Symble sym;
SymbolTable table = { 0 };  // Initialize symbol table
extern int insptr;             // pcode指令指针
extern instruction code[200];  // pcode指令数组

// Error handling
void error(int n) {
    printf("Error %d at line %d, column %d\t%s\t%d\n", n, sym.line, sym.column,sym.lexeme,sym.type);
    exit(1);
}

// Symbol table operations
void enter(const char* name, int level,ObjectType kind) {
    if (table.size >= 100) {
        error(1); // Symbol table overflow
    }
    strcpy(table.entries[table.size].name, name);
    table.entries[table.size].kind = kind;
    table.size++;
}

int position(const char* name) {
    for (int i = table.size - 1; i >= 0; i--) {
        if (strcmp(table.entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Parser implementation
void program() {
    if (sym.type != PROGRAMSYM) error(2);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(3);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(4);
    sym = get_sym(lexer);
    
    while (sym.type == FUNCSYM) {
        func_def();
    }
    
    if (sym.type != MAINSYM) error(5);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(6);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(7);
    sym = get_sym(lexer);
    
    if (sym.type != RBRACE) error(8);
    sym = get_sym(lexer);
}

void func_def() {
    sym = get_sym(lexer); // Skip 'func'
    
    if (sym.type != IDENT) error(9);
    char func_name[256];
    strcpy(func_name, sym.lexeme);
    sym = get_sym(lexer);
    
    if (sym.type != LPAREN) error(10);
    sym = get_sym(lexer);
    code_gen(jmp, 0);
    
    enter(func_name, 0,FUNC);
    
    if (sym.type != RPAREN) {
        param_list();
    }
    
    if (sym.type != RPAREN) error(11);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(12);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(15);
    sym = get_sym(lexer);
}

void param_list() {
    if (sym.type != IDENT) error(16);
    enter(sym.lexeme,0,PARAM);
    sym = get_sym(lexer);
    
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        if (sym.type != IDENT) error(17);
        enter(sym.lexeme, 0,PARAM);
        sym = get_sym(lexer);
    }
}

void stmt_list() {
    stmt();
    if (sym.type != SEMICOLON) error(18);
    sym = get_sym(lexer);
    
    while (sym.type != RBRACE) {
        stmt();
        if (sym.type != SEMICOLON) error(19);
        sym = get_sym(lexer);
    }
}

void stmt() {
    switch (sym.type) {
        case LETSYM:
            declare_stmt();
            break;
        case IDENT:
            assign_stmt();
            break;
        case IFSYM:
            if_stmt();
            break;
        case WHILESYM:
            while_stmt();
            break;
        case INPUTSYM:
            input_stmt();
            break;
        case OUTPUTSYM:
            output_stmt();
            break;
        case RETURNSYM:
            return_stmt();
            break;
        default:
            error(20);
    }
}

void declare_stmt() {
    sym = get_sym(lexer); // Skip 'let'
    
    if (sym.type != IDENT) error(21);
    enter(sym.lexeme,0,VAR);
    sym = get_sym(lexer);
    
    if (sym.type == BECOMES) {
        sym = get_sym(lexer);
        expr();
    }
}

void assign_stmt() {
    if (position(sym.lexeme) == -1) error(22);
    sym = get_sym(lexer);
    
    if (sym.type != BECOMES) error(23);
    sym = get_sym(lexer);
    
    expr();
}

void if_stmt() {
    sym = get_sym(lexer); // Skip 'if'
    
    if (sym.type != LPAREN) error(24);
    sym = get_sym(lexer);
    
    bool_expr();
    
    if (sym.type != RPAREN) error(25);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(26);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(27);
    sym = get_sym(lexer);
    
    if (sym.type == ELSESYM) {
        sym = get_sym(lexer);
        if (sym.type != LBRACE) error(28);
        sym = get_sym(lexer);
        stmt_list();
        if (sym.type != RBRACE) error(29);
        sym = get_sym(lexer);
    }
}

void while_stmt() {
    sym = get_sym(lexer); // Skip 'while'
    
    if (sym.type != LPAREN) error(30);
    sym = get_sym(lexer);
    
    bool_expr();
    
    if (sym.type != RPAREN) error(31);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(32);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(33);
    sym = get_sym(lexer);
}

void func_call() {
    sym = get_sym(lexer);
    
    if (sym.type != RPAREN) {
        arg_list();
    }
    
    if (sym.type != RPAREN) error(36);
    sym = get_sym(lexer);
}

void arg_list() {
    expr();
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        expr();
    }
}

void input_stmt() {
    sym = get_sym(lexer); // Skip 'input'
    
    if (sym.type != LPAREN) error(37);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(38);
    if (position(sym.lexeme) == -1) error(39);
    sym = get_sym(lexer);
    
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        if (sym.type != IDENT) error(40);
        if (position(sym.lexeme) == -1) error(41);
        sym = get_sym(lexer);
    }
    
    if (sym.type != RPAREN) error(42);
    sym = get_sym(lexer);
}

void output_stmt() {
    sym = get_sym(lexer); // Skip 'output'
    
    if (sym.type != LPAREN) error(43);
    sym = get_sym(lexer);
    
    expr();
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        expr();
    }
    
    if (sym.type != RPAREN) error(44);
    sym = get_sym(lexer);
}

// Add return statement handling
void return_stmt() {
    sym = get_sym(lexer); // Skip 'return'

    // Check if there's an expression to return
    if (sym.type != SEMICOLON) {
        expr(table);
    }
}


void bool_expr() {
    expr();
    if (sym.type != EQL && sym.type != NEQ && 
        sym.type != LSS && sym.type != LEQ && 
        sym.type != GTR && sym.type != GEQ) {
        error(45);
    }
    sym = get_sym(lexer);
    expr();
}

void expr() {
    if (sym.type == PLUS || sym.type == MINUS) {
        sym = get_sym(lexer);
    }
    term();
    while (sym.type == PLUS || sym.type == MINUS) {
        sym = get_sym(lexer);
        term();
    }
}

void term() {
    factor();
    while (sym.type == MULTIPLY || sym.type == DIVIDE) {
        sym = get_sym(lexer);
        factor();
    }
}

void factor() {
    switch (sym.type) {
        case IDENT:
            if (position(sym.lexeme) == -1) error(46);
            sym = get_sym(lexer);
            if (sym.type == LPAREN) {
                func_call(table);
            } 
            break;
        case NUMBER:
            sym = get_sym(lexer);
            break;
        case LPAREN:
            sym = get_sym(lexer);
            expr(table);
            if (sym.type != RPAREN) error(47);
            sym = get_sym(lexer);
            break;
        default:
            error(48);
    }
}