#include <stdlib.h>
#include "ast.h"

void node_free(Node *n) {
    if (n == NULL) {
        return;
    }
    switch (n->kind) {
        case N_NEG:
            node_free(n->as.neg.operand);
            break;
        case N_BIN:
            node_free(n->as.bin.l);
            node_free(n->as.bin.r);
            break;
        case N_ASSIGN:
            node_free(n->as.assign.value);
            break;
        case N_NUM:
        case N_VAR:
            break;              // foglie: nessun figlio
    }
    free(n);
}
