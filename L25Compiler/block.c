#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 全局变量
extern Lexer* lexer;
extern Symble sym;
SymbolTable table = { 0 };  // 初始化符号表
extern int insptr;         // 指令指针
extern instruction code[200];  

// 当前函数变量+参量总数
int param_count = 0;
// 是否有函数定义
bool has_func = false;
// 函数初始化空间的pcode指令位置
int ini_pos = -1;

// 错误处理
void error(int n) {
    printf("Error %d at line %d, column %d\t%s\t%d\n", n, sym.line, sym.column, sym.lexeme, sym.type);
    exit(1);
}

void enter(const char* name, ObjectType kind) {
    if (table.size >= 100) {
        error(1); // Symbol table overflow
    }
    strcpy(table.entries[table.size].name, name);
    table.entries[table.size].kind = kind;
    // adddress为本级偏移量
    // 相对base偏移量为1时保存返回地址, 偏移量为2时保存返回值
    table.entries[table.size].address = param_count + 3; 
    table.size++;
}

// TODO:类型检查
int position(char* name) {
    for (int i = table.size - 1; i >= 0; i--) {
        if (strcmp(table.entries[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// 函数定义
void func_def() {
    static bool is_first_func = true;  // 标记是否是第一个函数定义
    int insptr0 = insptr;  // 保存当前指令位置
    
    sym = get_sym(lexer); //  跳过'func'
    
    if (sym.type != IDENT) error(4);
    char func_name[256];
    strcpy(func_name, sym.lexeme);
    sym = get_sym(lexer);
    
    if (sym.type != LPAREN) error(5);
    sym = get_sym(lexer);
    
    enter(func_name, FUNC);
    int func_pos = table.size - 1;
    
    // 只在第一个函数定义时生成jmp指令
    if (is_first_func) {
        code_gen(jmp, 0);
        is_first_func = false;
    }
    
    // 传参
    param_count = 0;
    if (sym.type != RPAREN) {
        do {
            if(sym.type == COMMA) sym = get_sym(lexer);
            if (sym.type != IDENT) error(6);
            enter(sym.lexeme, PARAM);
            param_count++;
            sym = get_sym(lexer);
        } while (sym.type == COMMA);
    }
    
    if (sym.type != RPAREN) error(7);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(8);
    sym = get_sym(lexer);
    
    // 设置函数入口点
    table.entries[func_pos].address = insptr;
    
    // 初始化栈帧
    ini_pos = insptr;
    code_gen(ini, param_count + 3);  // +3：DL, RA，返回值
    
    // 处理函数体
    while (sym.type != RBRACE) {
        stmt();
        sym = get_sym(lexer);
    }

    // 回填初始化栈帧空间
    code[ini_pos].value = param_count + 2;
    ini_pos = -1;
    param_count = 0;

    // 生成无返回值的返回指令
    code_gen(opr, 0);
    
    if (sym.type != RBRACE) error(10);
    sym = get_sym(lexer);
}

// 语句集合
void stmt_list() {
    while (sym.type != RBRACE) {
        stmt();
        if (sym.type != SEMICOLON) error(11);
        sym = get_sym(lexer);
    }
}

// Statement处理
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
            error(12);
    }
}

// Declaration statement
void declare_stmt() {
    sym = get_sym(lexer); // Skip 'let'
    
    if (sym.type != IDENT) error(13);
 
    // 增加变量数
    enter(sym.lexeme, VAR);
    param_count++;

    sym = get_sym(lexer);
    
    if (sym.type == BECOMES) {
        sym = get_sym(lexer);
        expr();
        code_gen(sto, table.entries[table.size - 1].address);
    }
    if (sym.type != SEMICOLON) error(9);
}

// Assignment statement
void assign_stmt() {
    int pos = position(sym.lexeme);
    if (pos == -1) error(14);
    sym = get_sym(lexer);
    
    if (sym.type != BECOMES) error(15);
    sym = get_sym(lexer);
    
    expr();
    code_gen(sto, table.entries[pos].address);
    if (sym.type != SEMICOLON) error(9);
}

// Input statement
void input_stmt() {
    sym = get_sym(lexer); // Skip 'input'
    
    if (sym.type != LPAREN) error(16);
    sym = get_sym(lexer);
    
    do {
        if (sym.type == COMMA) sym = get_sym(lexer);
        if (sym.type != IDENT) error(17);
        int pos = position(sym.lexeme);
        if (pos == -1) error(18);
        sym = get_sym(lexer);
        
        // Generate input instruction
        code_gen(opr, 15); // Input operation
        code_gen(sto, table.entries[pos].address); // Store input value
        
    } while (sym.type == COMMA);
    
    if (sym.type != RPAREN) error(19);
    sym = get_sym(lexer);
    if (sym.type != SEMICOLON) error(9);
}

// Output statement
void output_stmt() {
    sym = get_sym(lexer); // Skip 'output'
    
    if (sym.type != LPAREN) error(20);
    sym = get_sym(lexer);
    
    do {
        if (sym.type == COMMA) sym = get_sym(lexer);
        expr(); // Evaluate expression
        code_gen(opr, 14); // Output operation
        
    } while (sym.type == COMMA);
    
    if (sym.type != RPAREN) error(21);
    sym = get_sym(lexer);
    if (sym.type != SEMICOLON) error(9);
}

// If statement
void if_stmt() {
    sym = get_sym(lexer); // Skip 'if'
    
    if (sym.type != LPAREN) error(22);
    sym = get_sym(lexer);
    
    bool_expr();
    
    int jpc_addr = insptr;
    code_gen(jpc, 0);
    
    if (sym.type != RPAREN) error(23);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(24);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(25);
    sym = get_sym(lexer);
    
    int jmp_addr = insptr;
    code_gen(jmp, 0);
    
    code[jpc_addr].value = insptr;
    
    if (sym.type == ELSESYM) {
        sym = get_sym(lexer);
        if (sym.type != LBRACE) error(26);
        sym = get_sym(lexer);
        stmt_list();
        if (sym.type != RBRACE) error(27);
        sym = get_sym(lexer);
    }
    
    code[jmp_addr].value = insptr;
    if (sym.type != SEMICOLON) error(9);
}

// While statement
void while_stmt() {
    sym = get_sym(lexer); // Skip 'while'
    
    int loop_start = insptr;
    
    if (sym.type != LPAREN) error(28);
    sym = get_sym(lexer);
    
    bool_expr();
    
    int jpc_addr = insptr;
    code_gen(jpc, 0);
    
    if (sym.type != RPAREN) error(29);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(30);
    sym = get_sym(lexer);
    
    stmt_list();
    
    if (sym.type != RBRACE) error(31);
    sym = get_sym(lexer);
    
    code_gen(jmp, loop_start);
    code[jpc_addr].value = insptr;
    if (sym.type != SEMICOLON) error(9);
}

// Function call
void func_call(int pos) {
    if (sym.type != LPAREN) error(32);
    sym = get_sym(lexer);
    
    if (sym.type != RPAREN) {
        do {
            if(sym.type == COMMA)
                sym = get_sym(lexer);
            expr();
            // 生成参数传递指令 (opr 16)
            code_gen(opr, 16);
        } while (sym.type == COMMA);
    }
    
    if (sym.type != RPAREN) error(33);
    sym = get_sym(lexer);
    
    // 指定被调用函数的指令起始地址
    code_gen(cal, table.entries[pos].address);
}

// Return statement
void return_stmt() {
    sym = get_sym(lexer); // Skip 'return'
    
    if (sym.type == SEMICOLON) {
        // 无返回值的返回
        code_gen(opr, 0);
    } else {
        // 有返回值的返回
        expr();
        // 把表达式的值放到返回值的位置(相对base偏移两位)
        code_gen(sto, 2);
        // 生成有返回值的返回指令
        code_gen(opr, 1);
    }
    
    if (sym.type != SEMICOLON) error(34);
}

// Boolean expression
void bool_expr() {
    expr();
    SymbleType op = sym.type;
    if (op != EQL && op != NEQ && op != LSS && op != LEQ && op != GTR && op != GEQ) {
        error(35);
    }
    sym = get_sym(lexer);
    expr();
    
    switch (op) {
        case EQL: code_gen(opr, 8); break;
        case NEQ: code_gen(opr, 9); break;
        case LSS: code_gen(opr, 10); break;
        case LEQ: code_gen(opr, 11); break;
        case GTR: code_gen(opr, 12); break;
        case GEQ: code_gen(opr, 13); break;
        default: error(36);
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
            if (pos == -1) error(37);
            sym = get_sym(lexer);
            
            if (sym.type == LPAREN && table.entries[pos].kind == FUNC) {
                func_call(pos);
                // 返回值处理已经放在栈顶
            } else {
                code_gen(lod, table.entries[pos].address);
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
            if (sym.type != RPAREN) error(38);
            sym = get_sym(lexer);
            break;
        default:
            error(39);
    }
}

// Program entry point
void program() {
    if (sym.type != PROGRAMSYM) error(40);
    sym = get_sym(lexer);
    
    if (sym.type != IDENT) error(41);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(42);
    sym = get_sym(lexer);
    
    // Process function definitions
    while (sym.type == FUNCSYM) {
        func_def();
        has_func = true;
    }
    
    // Process main function
    if (sym.type != MAINSYM) error(43);
    sym = get_sym(lexer);
    
    if (sym.type != LBRACE) error(44);
    sym = get_sym(lexer);
    
    // Enter main in symbol table
    enter("main", FUNC);
    int main_pos = table.size - 1;
    
    // 设置main函数入口点
    table.entries[main_pos].address = insptr;
    // 在main函数前有函数定义时回填 jmp 指令
    if (has_func)
        code[0].value = insptr;
    
    // 初始化main函数的栈帧
    ini_pos = insptr;
    param_count = 0;
    code_gen(ini,0);  // +3 for DL + RA
    
    // 处理main函数体
    while (sym.type != RBRACE) {
        stmt();
        // printf("param_count:%d\n", param_count);
        if (sym.type != SEMICOLON) error(45);
        sym = get_sym(lexer);
    }

    // 回填栈帧初始化变量总数
    code[ini_pos].value = param_count + 3;
    printf("param_count:%d\n", param_count);
    ini_pos = -1;
    param_count = 0;
    
    // 生成无返回值的返回指令
    code_gen(opr, 0);
    
    if (sym.type != RBRACE) error(46);
    sym = get_sym(lexer);
    
    if (sym.type != RBRACE) error(47);
    sym = get_sym(lexer);
}