#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#define NAME_MAX_LEN 32

typedef enum {
    T_NUM, T_IDENT,
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_LPAREN, T_RPAREN, T_ASSIGN,
    T_END,          // fine della riga
    T_ERROR         // carattere che non conosciamo
} TokenType;

typedef struct {
    TokenType type;
    double    num;                  // valido se type == T_NUM
    char      name[NAME_MAX_LEN];   // valido se type == T_IDENT
    size_t    pos;                  // colonna nel sorgente, per i messaggi d'errore
} Token;

typedef struct {
    const char *src;
    size_t      pos;
} Lexer;

void  lexer_init(Lexer *lx, const char *src);
Token lexer_next(Lexer *lx);

#endif
