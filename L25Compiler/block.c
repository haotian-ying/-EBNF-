#include "common.h"
#include <stdlib.h>
#include <string.h>

// Global variables
extern Lexer* lexer;
extern Symble sym;

// Error handling
void error(int n) {
    printf("Error %d at line %d, column %d\t%s\t%d\n", n, sym.line, sym.column,sym.lexeme,sym.type);
    exit(1);
}

// Symbol table operations
void enter(SymbolTable* table, const char* name, ObjectType kind) {
    if (table->size >= 100) {
        error(1); // Symbol table overflow
    }
    strcpy(table->entries[table->size].name, name);
    table->entries[table->size].kind = kind;
    table->size++;
}

int position(SymbolTable* table, const char* name) {
    for (int i = table->size - 1; i >= 0; i--) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Parser implementation
void program(SymbolTable* table) {
    if (sym.type != PROGRAMSYM) error(2);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(3);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(4);
    sym = get_sym(lexer);
    
    while (sym.type == FUNCSYM) {
        func_def(table);
    }
    
    if (sym.type != MAINSYM) error(5);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(6);
    sym = get_sym(lexer);
    
    stmt_list(table);
    
    if (sym.type != RBRACE) error(7);
    sym = get_sym(lexer);
    
    if (sym.type != RBRACE) error(8);
    sym = get_sym(lexer);
}

void func_def(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'func'
    
    if (sym.type != IDENT) error(9);
    char func_name[256];
    strcpy(func_name, sym.lexeme);
    sym = get_sym(lexer);
    
    if (sym.type != LPAREN) error(10);
    sym = get_sym(lexer);
    
    enter(table, func_name, FUNC);
    
    if (sym.type != RPAREN) {
        param_list(table);
    }
    
    if (sym.type != RPAREN) error(11);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(12);
    sym = get_sym(lexer);
    
    stmt_list(table);
    
    if (sym.type != RBRACE) error(15);
    sym = get_sym(lexer);
}

void param_list(SymbolTable* table) {
    if (sym.type != IDENT) error(16);
    enter(table, sym.lexeme, PARAM);
    sym = get_sym(lexer);
    
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        if (sym.type != IDENT) error(17);
        enter(table, sym.lexeme, PARAM);
        sym = get_sym(lexer);
    }
}

void stmt_list(SymbolTable* table) {
    stmt(table);
    if (sym.type != SEMICOLON) error(18);
    sym = get_sym(lexer);
    
    while (sym.type != RBRACE) {
        stmt(table);
        if (sym.type != SEMICOLON) error(19);
        sym = get_sym(lexer);
    }
}

void stmt(SymbolTable* table) {
    switch (sym.type) {
        case LETSYM:
            declare_stmt(table);
            break;
        case IDENT:
            assign_stmt(table);
            break;
        case IFSYM:
            if_stmt(table);
            break;
        case WHILESYM:
            while_stmt(table);
            break;
        case INPUTSYM:
            input_stmt(table);
            break;
        case OUTPUTSYM:
            output_stmt(table);
            break;
        case RETURNSYM:
            return_stmt(table);
            break;
        default:
            error(20);
    }
}

void declare_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'let'
    
    if (sym.type != IDENT) error(21);
    enter(table, sym.lexeme, VAR);
    sym = get_sym(lexer);
    
    if (sym.type == BECOMES) {
        sym = get_sym(lexer);
        expr(table);
    }
}

void assign_stmt(SymbolTable* table) {
    if (position(table, sym.lexeme) == -1) error(22);
    sym = get_sym(lexer);
    
    if (sym.type != BECOMES) error(23);
    sym = get_sym(lexer);
    
    expr(table);
}

void if_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'if'
    
    if (sym.type != LPAREN) error(24);
    sym = get_sym(lexer);
    
    bool_expr(table);
    
    if (sym.type != RPAREN) error(25);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(26);
    sym = get_sym(lexer);
    
    stmt_list(table);
    
    if (sym.type != RBRACE) error(27);
    sym = get_sym(lexer);
    
    if (sym.type == ELSESYM) {
        sym = get_sym(lexer);
        if (sym.type != LBRACE) error(28);
        sym = get_sym(lexer);
        stmt_list(table);
        if (sym.type != RBRACE) error(29);
        sym = get_sym(lexer);
    }
}

void while_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'while'
    
    if (sym.type != LPAREN) error(30);
    sym = get_sym(lexer);
    
    bool_expr(table);
    
    if (sym.type != RPAREN) error(31);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(32);
    sym = get_sym(lexer);
    
    stmt_list(table);
    
    if (sym.type != RBRACE) error(33);
    sym = get_sym(lexer);
}

void func_call(SymbolTable* table) {
    sym = get_sym(lexer);
    
    if (sym.type != RPAREN) {
        arg_list(table);
    }
    
    if (sym.type != RPAREN) error(36);
    sym = get_sym(lexer);
}

void arg_list(SymbolTable* table) {
    expr(table);
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        expr(table);
    }
}

void input_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'input'
    
    if (sym.type != LPAREN) error(37);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(38);
    if (position(table, sym.lexeme) == -1) error(39);
    sym = get_sym(lexer);
    
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        if (sym.type != IDENT) error(40);
        if (position(table, sym.lexeme) == -1) error(41);
        sym = get_sym(lexer);
    }
    
    if (sym.type != RPAREN) error(42);
    sym = get_sym(lexer);
}

void output_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'output'
    
    if (sym.type != LPAREN) error(43);
    sym = get_sym(lexer);
    
    expr(table);
    while (sym.type == COMMA) {
        sym = get_sym(lexer);
        expr(table);
    }
    
    if (sym.type != RPAREN) error(44);
    sym = get_sym(lexer);
}

// Add return statement handling
void return_stmt(SymbolTable* table) {
    sym = get_sym(lexer); // Skip 'return'

    // Check if there's an expression to return
    if (sym.type != SEMICOLON) {
        expr(table);
    }
}


void bool_expr(SymbolTable* table) {
    expr(table);
    if (sym.type != EQL && sym.type != NEQ && 
        sym.type != LSS && sym.type != LEQ && 
        sym.type != GTR && sym.type != GEQ) {
        error(45);
    }
    sym = get_sym(lexer);
    expr(table);
}

void expr(SymbolTable* table) {
    if (sym.type == PLUS || sym.type == MINUS) {
        sym = get_sym(lexer);
    }
    term(table);
    while (sym.type == PLUS || sym.type == MINUS) {
        sym = get_sym(lexer);
        term(table);
    }
}

void term(SymbolTable* table) {
    factor(table);
    while (sym.type == MULTIPLY || sym.type == DIVIDE) {
        sym = get_sym(lexer);
        factor(table);
    }
}

void factor(SymbolTable* table) {
    switch (sym.type) {
        case IDENT:
            if (position(table, sym.lexeme) == -1) error(46);
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