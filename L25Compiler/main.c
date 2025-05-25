/* ���س��� */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

FILE* src;                  // 源文件            

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
    Lexer* lexer = lexer_init(src);

    // Tokenize the input
    Symble symble;
    printf("\nTokenizing input file:\n");
    printf("Type\t\tLexeme\t\tLine\tColumn\n");
    printf("----------------------------------------\n");

    do {
        symble = get_sym(lexer);
        printf("%-12s\t%-12s\t%d\t%d\n",
            symble_type_to_string(symble.type),
            symble.lexeme,
            symble.line,
            symble.column);
    } while (symble.type != TOKENEOF && symble.type != ERROR);
    
    // Initialize parser
    
    lexer_free(lexer);
    fclose(src);
    return 0;
}