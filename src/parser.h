#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "ast.h"

typedef struct {
    char   message[96];
    size_t pos;             // colonna dell'errore
} ParseError;

// Analizza una riga. In caso di successo restituisce l'albero (da liberare con
// node_free); in caso di errore restituisce NULL e riempie *err.
Node *parse_line(const char *src, ParseError *err);

#endif
