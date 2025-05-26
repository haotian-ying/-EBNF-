/* ���س��� */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

FILE* src;                  // 源文件            

// Global variables
Lexer* lexer = NULL;
Symble sym;
SymbolTable table = {0};  // Initialize symbol table

// Function to print symbol table
void print_symbol_table(SymbolTable* table) {
    printf("\nSymbol Table:\n");
    printf("----------------------------------------\n");
    printf("Name\t\tType\n");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < table->size; i++) {
        const char* type_str;
        switch (table->entries[i].kind) {
            case FUNC: type_str = "Function"; break;
            case VAR: type_str = "Variable"; break;
            case PARAM: type_str = "Parameter"; break;
            default: type_str = "Unknown"; break;
        }
        printf("%-16s\t%s\n", table->entries[i].name, type_str);
    }
    printf("----------------------------------------\n");
    printf("Total entries: %d\n", table->size);
}

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

    // Get first token
    sym = get_sym(lexer);

    printf("\nStarting syntax analysis:\n");
    printf("----------------------------------------\n");

    // Start parsing
    program(&table);

    // Print symbol table
    print_symbol_table(&table);
    
    // Cleanup
    lexer_free(lexer);
    fclose(src);
    return 0;
}