#include "common.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// Helper function to check if a character is a letter
static int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Helper function to check if a character is a digit
static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Initialize lexer
Lexer* lexer_init(FILE* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->source = source;
    lexer->line = 1;
    lexer->column = 0;
    lexer->has_read_char = 0;
    lexer->current_char = ' ';
    
    return lexer;
}

// Free lexer resources
void lexer_free(Lexer* lexer) {
    if (lexer) {
        free(lexer);
    }
}

// Get next character from input
static char lexer_advance(Lexer* lexer) {
    lexer->current_char = fgetc(lexer->source);
    lexer->column++;
    
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 0;
    }
    
    return lexer->current_char;
}

// Skip whitespace and comments
static void lexer_skip_whitespace(Lexer* lexer) {
    while (isspace(lexer->current_char)) {
        lexer_advance(lexer);
    }
}

// Get next token from input
Symble get_sym(Lexer* lexer) {
    Symble symble;
    symble.line = lexer->line;
    symble.column = lexer->column;
    symble.lexeme[0] = '\0';
    
    // Skip whitespace
    lexer_skip_whitespace(lexer);
    
    // Check for EOF
    if (lexer->current_char == EOF) {
        symble.type = TOKENEOF;
        return symble;
    }
    
    // Handle identifiers and keywords
    if (is_letter(lexer->current_char)) {
        int i = 0;
        while (is_letter(lexer->current_char) || is_digit(lexer->current_char)) {
            symble.lexeme[i++] = lexer->current_char;
            lexer_advance(lexer);
        }
        symble.lexeme[i] = '\0';
        
        // Check for keywords
        if (strcmp(symble.lexeme, "program") == 0) symble.type = PROGRAMSYM;
        else if (strcmp(symble.lexeme, "func") == 0) symble.type = FUNCSYM;
        else if (strcmp(symble.lexeme, "main") == 0) symble.type = MAINSYM;
        else if (strcmp(symble.lexeme, "let") == 0) symble.type = LETSYM;
        else if (strcmp(symble.lexeme, "if") == 0) symble.type = IFSYM;
        else if (strcmp(symble.lexeme, "else") == 0) symble.type = ELSESYM;
        else if (strcmp(symble.lexeme, "while") == 0) symble.type = WHILESYM;
        else if (strcmp(symble.lexeme, "return") == 0) symble.type = RETURNSYM;
        else if (strcmp(symble.lexeme, "input") == 0) symble.type = INPUTSYM;
        else if (strcmp(symble.lexeme, "output") == 0) symble.type = OUTPUTSYM;
        else symble.type = IDENT;
        
        return symble;
    }
    
    // Handle numbers
    if (is_digit(lexer->current_char)) {
        int i = 0;
        while (is_digit(lexer->current_char)) {
            symble.lexeme[i++] = lexer->current_char;
            lexer_advance(lexer);
        }
        symble.lexeme[i] = '\0';
        symble.type = NUMBER;
        return symble;
    }
    
    // Handle operators and punctuation
    switch (lexer->current_char) {
        case '+': symble.type = PLUS; symble.lexeme[0] = '+'; symble.lexeme[1] = '\0'; break;
        case '-': symble.type = MINUS; symble.lexeme[0] = '-'; symble.lexeme[1] = '\0'; break;
        case '*': symble.type = MULTIPLY; symble.lexeme[0] = '*'; symble.lexeme[1] = '\0'; break;
        case '/': symble.type = DIVIDE; symble.lexeme[0] = '/'; symble.lexeme[1] = '\0'; break;
        case '(': symble.type = LPAREN; symble.lexeme[0] = '('; symble.lexeme[1] = '\0'; break;
        case ')': symble.type = RPAREN; symble.lexeme[0] = ')'; symble.lexeme[1] = '\0'; break;
        case '{': symble.type = LBRACE; symble.lexeme[0] = '{'; symble.lexeme[1] = '\0'; break;
        case '}': symble.type = RBRACE; symble.lexeme[0] = '}'; symble.lexeme[1] = '\0'; break;
        case ';': symble.type = SEMICOLON; symble.lexeme[0] = ';'; symble.lexeme[1] = '\0'; break;
        case ',': symble.type = COMMA; symble.lexeme[0] = ','; symble.lexeme[1] = '\0'; break;
        case '=':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                symble.type = EQL;
                symble.lexeme[0] = '='; symble.lexeme[1] = '='; symble.lexeme[2] = '\0';
                lexer_advance(lexer);
            } else {
                symble.type = BECOMES;
                symble.lexeme[0] = '='; symble.lexeme[1] = '\0';
            }
            break;
        case '!':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                symble.type = NEQ;
                symble.lexeme[0] = '!'; symble.lexeme[1] = '='; symble.lexeme[2] = '\0';
                lexer_advance(lexer);
            } else {
                symble.type = ERROR;
                symble.lexeme[0] = '!'; symble.lexeme[1] = '\0';
            }
            break;
        case '<':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                symble.type = LEQ;
                symble.lexeme[0] = '<'; symble.lexeme[1] = '='; symble.lexeme[2] = '\0';
                lexer_advance(lexer);
            } else {
                symble.type = LSS;
                symble.lexeme[0] = '<'; symble.lexeme[1] = '\0';
            }
            break;
        case '>':
            lexer_advance(lexer);
            if (lexer->current_char == '=') {
                symble.type = GEQ;
                symble.lexeme[0] = '>'; symble.lexeme[1] = '='; symble.lexeme[2] = '\0';
                lexer_advance(lexer);
            } else {
                symble.type = GTR;
                symble.lexeme[0] = '>'; symble.lexeme[1] = '\0';
            }
            break;
        default:
            symble.type = ERROR;
            symble.lexeme[0] = lexer->current_char;
            symble.lexeme[1] = '\0';
            break;
    }
    
    lexer_advance(lexer);
    return symble;
}

// Convert token type to string for debugging
const char* symble_type_to_string(SymbleType type) {
    switch (type) {
        case PROGRAMSYM: return "PROGRAM";
        case FUNCSYM: return "FUNC";
        case MAINSYM: return "MAIN";
        case LETSYM: return "LET";
        case IFSYM: return "IF";
        case ELSESYM: return "ELSE";
        case WHILESYM: return "WHILE";
        case RETURNSYM: return "RETURN";
        case INPUTSYM: return "INPUT";
        case OUTPUTSYM: return "OUTPUT";
        case IDENT: return "IDENT";
        case NUMBER: return "NUMBER";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MULTIPLY: return "MULTIPLY";
        case DIVIDE: return "DIVIDE";
        case EQL: return "EQL";
        case NEQ: return "NEQ";
        case LSS: return "LSS";
        case LEQ: return "LEQ";
        case GTR: return "GTR";
        case GEQ: return "GEQ";
        case BECOMES: return "BECOMES";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case SEMICOLON: return "SEMICOLON";
        case COMMA: return "COMMA";
        case TOKENEOF: return "EOF";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
} 