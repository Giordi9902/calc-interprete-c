#ifndef AST_H
#define AST_H

#include "lexer.h"

typedef enum { N_NUM, N_VAR, N_NEG, N_BIN, N_ASSIGN } NodeKind;

// Un nodo dell'albero sintattico: una tagged union, come nell'articolo sulle struct.
typedef struct Node Node;
struct Node {
    NodeKind kind;
    union {
        double num;                                     // N_NUM
        char   name[NAME_MAX_LEN];                      // N_VAR
        struct { Node *operand; }              neg;     // N_NEG
        struct { char op; Node *l, *r; }       bin;     // N_BIN
        struct { char name[NAME_MAX_LEN]; Node *value; } assign;   // N_ASSIGN
    } as;
};

void node_free(Node *n);     // libera ricorsivamente tutto il sotto-albero

#endif
