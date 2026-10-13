#include <ctype.h>
#include <stdlib.h>
#include "lexer.h"

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
}

Token lexer_next(Lexer *lx) {
    const char *s = lx->src;

    while (isspace((unsigned char)s[lx->pos])) {
        lx->pos++;
    }

    Token t = { .type = T_ERROR, .pos = lx->pos };
    char c = s[lx->pos];

    if (c == '\0' || c == '#') {            // '#' inizia un commento
        t.type = T_END;
        return t;
    }

    if (isdigit((unsigned char)c) || c == '.') {
        char *end;
        t.num = strtod(s + lx->pos, &end);
        if (end == s + lx->pos) {           // un '.' da solo non è un numero
            lx->pos++;
            return t;
        }
        t.type = T_NUM;
        lx->pos = (size_t)(end - s);
        return t;
    }

    if (isalpha((unsigned char)c) || c == '_') {
        size_t n = 0;
        int troppo_lungo = 0;
        while (isalnum((unsigned char)s[lx->pos]) || s[lx->pos] == '_') {
            if (n < NAME_MAX_LEN - 1) {
                t.name[n++] = s[lx->pos];
            } else {
                troppo_lungo = 1;           // consumo comunque il resto del nome
            }
            lx->pos++;
        }
        t.name[n] = '\0';
        t.type = troppo_lungo ? T_ERROR : T_IDENT;
        return t;
    }

    lx->pos++;
    switch (c) {
        case '+': t.type = T_PLUS;   break;
        case '-': t.type = T_MINUS;  break;
        case '*': t.type = T_STAR;   break;
        case '/': t.type = T_SLASH;  break;
        case '(': t.type = T_LPAREN; break;
        case ')': t.type = T_RPAREN; break;
        case '=': t.type = T_ASSIGN; break;
        default:  break;                    // resta T_ERROR
    }
    return t;
}
