#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global variables
extern Lexer* lexer;
extern Symble sym;
SymbolTable table = { 0 };  // Initialize symbol table
extern int insptr;          // pcode instruction pointer
extern instruction code[200];  // pcode instruction array

// Error handling
void error(int n) {
    printf("Error %d at line %d, column %d\t%s\t%d\n", n, sym.line, sym.column, sym.lexeme, sym.type);
    exit(1);
}

// Symbol table operations
void enter(const char* name, ObjectType kind) {
    if (table.size >= 100) {
        error(1); // Symbol table overflow
    }
    strcpy(table.entries[table.size].name, name);
    table.entries[table.size].kind = kind;
    table.entries[table.size].address = table.size; // Use table index as address
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

// Block handling - Core function for recursive processing
void block(int dx, int tx, bool is_main) {
    int insptr0 = insptr;
    
    // Generate jump instruction for function entry
    code_gen(jmp, 0);
    
    // Process declarations
    declare(&dx);
    
    // Backpatch jump instruction
    code[insptr0].value = insptr;
    
    // Set function entry point
    if (!is_main) {
        table.entries[tx].address = insptr;
    }
    
    // Initialize stack frame
    code_gen(ini, dx);
    
    // Process statements
    stmt_list();
    
    // Generate return instruction
    code_gen(ret, 0);
}

// Declaration handling
void declare(int* pdx) {
    while (sym.type == LETSYM) {
        sym = get_sym(lexer); // Skip 'let'
        
        if (sym.type != IDENT) error(21);
        enter(sym.lexeme, VAR);
        sym = get_sym(lexer);
        
        if (sym.type == BECOMES) {
            sym = get_sym(lexer);
            expr();
            code_gen(sto, table.size - 1);
        }
        
        if (sym.type != SEMICOLON) error(18);
        sym = get_sym(lexer);
    }
}

// Function definition
void func_def() {
    sym = get_sym(lexer); // Skip 'func'
    
    if (sym.type != IDENT) error(9);
    char func_name[256];
    strcpy(func_name, sym.lexeme);
    sym = get_sym(lexer);
    
    if (sym.type != LPAREN) error(10);
    sym = get_sym(lexer);
    
    // Enter function in symbol table
    enter(func_name, FUNC);
    int func_pos = table.size - 1;
    
    // Process parameters
    int param_count = 0;
    if (sym.type != RPAREN) {
        do {
            if(sym.type == COMMA) sym = get_sym(lexer);
            if (sym.type != IDENT) error(16);
            enter(sym.lexeme, PARAM);
            param_count++;
            sym = get_sym(lexer);
        } while (sym.type == COMMA);
    }
    
    if (sym.type != RPAREN) error(11);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(12);
    sym = get_sym(lexer);
    
    // Process function body using block
    block(param_count + 3, func_pos, false); // +3 for SL, DL, RA
    
    if (sym.type != RBRACE) error(15);
    sym = get_sym(lexer);
}

// Statement list
void stmt_list() {
    while (sym.type != RBRACE) {
        stmt();
        if (sym.type != SEMICOLON) error(19);
        sym = get_sym(lexer);
    }
}

// Statement handling
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

// Declaration statement
void declare_stmt() {
    sym = get_sym(lexer); // Skip 'let'
    
    if (sym.type != IDENT) error(21);
    enter(sym.lexeme, VAR);
    sym = get_sym(lexer);
    
    if (sym.type == BECOMES) {
        sym = get_sym(lexer);
        expr();
        code_gen(sto, table.size - 1);
    }
}

// Assignment statement
void assign_stmt() {
    int pos = position(sym.lexeme);
    if (pos == -1) error(22);
    sym = get_sym(lexer);
    
    if (sym.type != BECOMES) error(23);
    sym = get_sym(lexer);
    
    expr();
    code_gen(sto, pos);
}

// Input statement
void input_stmt() {
    sym = get_sym(lexer); // Skip 'input'
    
    if (sym.type != LPAREN) error(37);
    sym = get_sym(lexer);
    
    do {
        if (sym.type != IDENT) error(38);
        int pos = position(sym.lexeme);
        if (pos == -1) error(39);
        sym = get_sym(lexer);
        
        // Generate input instruction
        code_gen(opr, 15); // Input operation
        code_gen(sto, pos); // Store input value
        
    } while (sym.type == COMMA);
    
    if (sym.type != RPAREN) error(40);
    sym = get_sym(lexer);
}

// Output statement
void output_stmt() {
    sym = get_sym(lexer); // Skip 'output'
    
    if (sym.type != LPAREN) error(41);
    sym = get_sym(lexer);
    
    do {
        expr(); // Evaluate expression
        code_gen(opr, 14); // Output operation
        
    } while (sym.type == COMMA);
    
    if (sym.type != RPAREN) error(42);
    sym = get_sym(lexer);
}

// If statement
void if_stmt() {
    sym = get_sym(lexer); // Skip 'if'
    
    if (sym.type != LPAREN) error(24);
    sym = get_sym(lexer);
    
    bool_expr();
    
    int jpc_addr = insptr;
    code_gen(jpc, 0);
    
    if (sym.type != RPAREN) error(25);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(26);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(27);
    sym = get_sym(lexer);
    
    int jmp_addr = insptr;
    code_gen(jmp, 0);
    
    code[jpc_addr].value = insptr;
    
    if (sym.type == ELSESYM) {
        sym = get_sym(lexer);
        if (sym.type != LBRACE) error(28);
        sym = get_sym(lexer);
        stmt_list();
        if (sym.type != RBRACE) error(29);
        sym = get_sym(lexer);
    }
    
    code[jmp_addr].value = insptr;
}

// While statement
void while_stmt() {
    sym = get_sym(lexer); // Skip 'while'
    
    int loop_start = insptr;
    
    if (sym.type != LPAREN) error(30);
    sym = get_sym(lexer);
    
    bool_expr();
    
    int jpc_addr = insptr;
    code_gen(jpc, 0);
    
    if (sym.type != RPAREN) error(31);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(32);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(33);
    sym = get_sym(lexer);
    
    code_gen(jmp, loop_start);
    code[jpc_addr].value = insptr;
}

// Function call
void func_call(int pos) {
    if (sym.type != LPAREN) error(35);
    sym = get_sym(lexer);
    
    int param_count = 0;
    if (sym.type != RPAREN) {
        do {
            if(sym.type == COMMA)
                sym = get_sym(lexer);
            expr();
            param_count++;
        } while (sym.type == COMMA);
    }
    
    if (sym.type != RPAREN) error(36);
    sym = get_sym(lexer);
    
    code_gen(cal, table.entries[pos].address);
}

// Return statement
void return_stmt() {
    sym = get_sym(lexer); // Skip 'return'
    
    expr();
    code_gen(ret, 0);
}

// Boolean expression
void bool_expr() {
    expr();
    SymbleType op = sym.type;
    if (op != EQL && op != NEQ && op != LSS && op != LEQ && op != GTR && op != GEQ) {
        error(45);
    }
    sym = get_sym(lexer);
    expr();
    
    switch (op) {
        case EQL: code_gen(opr, 8); break;
        case NEQ: code_gen(opr, 9); break;
        case LSS: code_gen(opr, 10); break;
        case LEQ: code_gen(opr, 13); break;
        case GTR: code_gen(opr, 12); break;
        case GEQ: code_gen(opr, 11); break;
        default: error(45);
    }
}

// Expression
void expr() {
    if (sym.type == PLUS || sym.type == MINUS) {
        SymbleType op = sym.type;
        sym = get_sym(lexer);
        term();
        if (op == MINUS) {
            code_gen(opr, 2); // Negate
        }
    } else {
        term();
    }
    
    while (sym.type == PLUS || sym.type == MINUS) {
        SymbleType op = sym.type;
        sym = get_sym(lexer);
        term();
        if (op == PLUS) {
            code_gen(opr, 3); // Add
        } else {
            code_gen(opr, 4); // Subtract
        }
    }
}

// Term
void term() {
    factor();
    while (sym.type == MULTIPLY || sym.type == DIVIDE) {
        SymbleType op = sym.type;
        sym = get_sym(lexer);
        factor();
        if (op == MULTIPLY) {
            code_gen(opr, 5); // Multiply
        } else {
            code_gen(opr, 6); // Divide
        }
    }
}

// Factor
void factor() {
    switch (sym.type) {
        case IDENT: {
            int pos = position(sym.lexeme);
            if (pos == -1) error(46);
            sym = get_sym(lexer);
            
            if (sym.type == LPAREN && table.entries[pos].kind == FUNC) {
                func_call(pos);
            } else {
                code_gen(lod, pos);
            }
            break;
        }
        case NUMBER:
            code_gen(lit, atoi(sym.lexeme));
            sym = get_sym(lexer);
            break;
        case LPAREN:
            sym = get_sym(lexer);
            expr();
            if (sym.type != RPAREN) error(47);
            sym = get_sym(lexer);
            break;
        default:
            error(48);
    }
}

// Program entry point
void program() {
    if (sym.type != PROGRAMSYM) error(2);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(3);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(4);
    sym = get_sym(lexer);
    
    // Process function definitions
    while (sym.type == FUNCSYM) {
        func_def();
    }
    
    // Process main function
    if (sym.type != MAINSYM) error(5);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(6);
    sym = get_sym(lexer);
    
    // Enter main in symbol table
    enter("main", FUNC);
    int main_pos = table.size - 1;
    
    // Process main body using block
    block(3, main_pos, true); // +3 for SL, DL, RA
    
    if (sym.type != RBRACE) error(7);
    sym = get_sym(lexer);
    
    if (sym.type != RBRACE) error(8);
    sym = get_sym(lexer);
}