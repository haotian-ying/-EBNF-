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

// Function declarations of lexer
Lexer* lexer_init(FILE* source);
void lexer_free(Lexer* lexer);
Symble get_sym(Lexer* lexer);
const char* symble_type_to_string(SymbleType type);

#endif // COMMON_H 