/* 主程序 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

FILE* src;                  // 源文件            

// Global variables
Lexer* lexer = NULL;
Symble sym;
extern SymbolTable table;
extern instruction code[200];  // pcode instruction array
extern int insptr;            // pcode instruction pointer

// Function to print pcode instructions
void print_pcode() {
    printf("\nPcode Instructions:\n");
    printf("----------------------------------------\n");
    printf("Line\tOperation\tValue\n");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < insptr; i++) {
        const char* op_str;
        switch (code[i].op) {
            case lit: op_str = "LIT"; break;
            case opr: op_str = "OPR"; break;
            case lod: op_str = "LOD"; break;
            case sto: op_str = "STO"; break;
            case cal: op_str = "CAL"; break;
            case ini: op_str = "INI"; break;
            case jmp: op_str = "JMP"; break;
            case jpc: op_str = "JPC"; break;
            case ret: op_str = "RET"; break;
            default: op_str = "UNKNOWN"; break;
        }
        
        // For OPR instructions, print the operation type
        if (code[i].op == opr) {
            const char* opr_type;
            switch (code[i].value) {
                case 0: opr_type = "END"; break;      // 无返回值结束
                case 1: opr_type = "RET"; break;      // 返回值
                case 2: opr_type = "NEG"; break;      // 栈顶取负
                case 3: opr_type = "ADD"; break;      
                case 4: opr_type = "SUB"; break;
                case 5: opr_type = "MUL"; break;
                case 6: opr_type = "DIV"; break;
                case 7: opr_type = "MOD"; break;
                case 8: opr_type = "EQL"; break;
                case 9: opr_type = "NEQ"; break;
                case 10: opr_type = "LSS"; break;
                case 11: opr_type = "LEQ"; break;
                case 12: opr_type = "GTR"; break;
                case 13: opr_type = "GEQ"; break;
                case 14: opr_type = "WRITE"; break;
                case 15: opr_type = "READ"; break;
                case 16: opr_type = "PASS PARAM"; break;
                default: opr_type = "UNKNOWN"; break;
            }
            printf("%d\t%s\t\t%s\n", i, op_str, opr_type);
        } else {
            printf("%d\t%s\t\t%d\n", i, op_str, code[i].value);
        }
    }
    printf("----------------------------------------\n");
    printf("Total instructions: %d\n", insptr);
}

// Function to print symbol table
void print_symbol_table() {
    printf("\nSymbol Table:\n");
    printf("----------------------------------------\n");
    printf("Name\t\tType\t\tAddress\n");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < table.size; i++) {
        const char* type_str;
        switch (table.entries[i].kind) {
            case FUNC: type_str = "Function"; break;
            case VAR: type_str = "Variable"; break;
            case PARAM: type_str = "Parameter"; break;
            default: type_str = "Unknown"; break;
        }
        printf("%-16s\t%-16s\t%d\n", table.entries[i].name, type_str, table.entries[i].address);
    }
    printf("----------------------------------------\n");
    printf("Total entries: %d\n", table.size);
}

// Function to print stack data
void print_stack_data(int base, int top, int* stack_data) {
    printf("%d\t%d\n", base, top);
    printf("\nFinal Stack State:\n");
    printf("----------------------------------------\n");
    printf("Index\tValue\tDescription\n");
    printf("----------------------------------------\n");
    
    // 打印栈帧信息
    if (base > 0) {
        printf("%d\t%d\tStatic Link\n", base, stack_data[base]);
        printf("%d\t%d\tReturn Address\n", base+1, stack_data[base+1]);
        printf("%d\t%d\tParameter Count\n", base+2, stack_data[base+2]);
    }
    
    // 打印局部变量和临时值
    for (int i = base + 3; i <= top; i++) {
        printf("%d\t%d\t", i, stack_data[i]);
        if (i == top) {
            printf("Top of Stack");
        } else if (i == base + 3) {
            printf("First Local Variable");
        } else {
            printf("Stack Value");
        }
        printf("\n");
    }
    printf("----------------------------------------\n");
}

void program();
void interpret();
void get_stack_data(int* base, int* top, int* stack_data);

int main()
{
    // 文件名长度 + 文件名
    char filename[32];
    // 从可执行文件所在目录开始
    strcpy(filename, "../../../../test/");
    printf("Input file: ");
    (void)scanf("%s", filename + 17);
    printf("%s\n", filename);
    
    src = fopen(filename, "r");
    if (src == NULL) {
        printf("Fail to open %s\n", filename);
        exit(1);
    }

    // Initialize lexer
    lexer = lexer_init(src);
    if (lexer == NULL) {
        printf("Failed to initialize lexer\n");
        fclose(src);
        return 1;
    }

    // Initialize instruction pointer
    insptr = 0;

    // Get first token
    sym = get_sym(lexer);

    printf("\nStarting syntax analysis:\n");
    printf("----------------------------------------\n");

    // Start parsing
    program();

    // Print symbol table and pcode
    print_symbol_table();
    print_pcode();

    // Execute the program
    printf("\nProgram Execution:\n");
    printf("----------------------------------------\n");
    interpret();
    
    // 获取并打印最终的栈状态
    int base, top;
    int stack_data[1024];  // 假设最大栈大小为1024
    get_stack_data(&base, &top, stack_data);
    print_stack_data(base, top, stack_data);
    
    printf("----------------------------------------\n");
    printf("Program execution completed.\n");

    // Cleanup
    lexer_free(lexer);
    fclose(src);
    return 0;
}