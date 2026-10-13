// Parser a discesa ricorsiva. Grammatica:
//
//   riga    := IDENT '=' expr  |  expr
//   expr    := term   (('+' | '-') term)*
//   term    := unario (('*' | '/') unario)*
//   unario  := '-' unario | primario
//   primario:= NUMERO | IDENT | '(' expr ')'
//
// Ogni regola è una funzione. La precedenza tra gli operatori nasce da sola
// dalla profondità delle chiamate: term è "più dentro" di expr.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

typedef struct {
    Lexer       lx;
    Token       cur;        // il token che stiamo guardando
    ParseError *err;
} Parser;

static void advance(Parser *p) {
    p->cur = lexer_next(&p->lx);
}

static Node *fail(Parser *p, const char *msg) {
    if (p->err->message[0] == '\0') {       // vale il primo errore, non gli echi successivi
        snprintf(p->err->message, sizeof p->err->message, "%s", msg);
        p->err->pos = p->cur.pos;
    }
    return NULL;
}

static Node *node_new(NodeKind kind) {
    Node *n = calloc(1, sizeof *n);
    if (n != NULL) {
        n->kind = kind;
    }
    return n;
}

static Node *parse_expr(Parser *p);

static Node *parse_primary(Parser *p) {
    if (p->cur.type == T_NUM) {
        Node *n = node_new(N_NUM);
        if (n == NULL) {
            return fail(p, "memoria esaurita");
        }
        n->as.num = p->cur.num;
        advance(p);
        return n;
    }
    if (p->cur.type == T_IDENT) {
        Node *n = node_new(N_VAR);
        if (n == NULL) {
            return fail(p, "memoria esaurita");
        }
        snprintf(n->as.name, sizeof n->as.name, "%s", p->cur.name);
        advance(p);
        return n;
    }
    if (p->cur.type == T_LPAREN) {
        advance(p);
        Node *inner = parse_expr(p);
        if (inner == NULL) {
            return NULL;
        }
        if (p->cur.type != T_RPAREN) {
            node_free(inner);
            return fail(p, "manca la parentesi chiusa ')'");
        }
        advance(p);
        return inner;
    }
    if (p->cur.type == T_ERROR) {
        return fail(p, "carattere non riconosciuto");
    }
    return fail(p, "mi aspettavo un numero, una variabile o '('");
}

static Node *parse_unary(Parser *p) {
    if (p->cur.type == T_MINUS) {
        advance(p);
        Node *operand = parse_unary(p);
        if (operand == NULL) {
            return NULL;
        }
        Node *n = node_new(N_NEG);
        if (n == NULL) {
            node_free(operand);
            return fail(p, "memoria esaurita");
        }
        n->as.neg.operand = operand;
        return n;
    }
    return parse_primary(p);
}

static Node *make_bin(Parser *p, char op, Node *l, Node *r) {
    Node *n = node_new(N_BIN);
    if (n == NULL) {
        node_free(l);
        node_free(r);
        return fail(p, "memoria esaurita");
    }
    n->as.bin.op = op;
    n->as.bin.l = l;
    n->as.bin.r = r;
    return n;
}

static Node *parse_term(Parser *p) {
    Node *left = parse_unary(p);
    while (left != NULL && (p->cur.type == T_STAR || p->cur.type == T_SLASH)) {
        char op = p->cur.type == T_STAR ? '*' : '/';
        advance(p);
        Node *right = parse_unary(p);
        if (right == NULL) {
            node_free(left);
            return NULL;
        }
        left = make_bin(p, op, left, right);
    }
    return left;
}

static Node *parse_expr(Parser *p) {
    Node *left = parse_term(p);
    while (left != NULL && (p->cur.type == T_PLUS || p->cur.type == T_MINUS)) {
        char op = p->cur.type == T_PLUS ? '+' : '-';
        advance(p);
        Node *right = parse_term(p);
        if (right == NULL) {
            node_free(left);
            return NULL;
        }
        left = make_bin(p, op, left, right);
    }
    return left;
}

Node *parse_line(const char *src, ParseError *err) {
    err->message[0] = '\0';
    err->pos = 0;

    Parser p = { .err = err };
    lexer_init(&p.lx, src);
    advance(&p);
    if (p.cur.type == T_END) {
        return NULL;                        // riga vuota: nessun errore (message resta vuoto)
    }

    Node *root = NULL;
    if (p.cur.type == T_IDENT) {
        // serve un token di anticipo: "x = ..." oppure solo "x + ..."?
        Lexer ahead = p.lx;                 // copia dello stato: il lexer è un semplice struct
        if (lexer_next(&ahead).type == T_ASSIGN) {
            Token name = p.cur;
            advance(&p);                    // consuma il nome
            advance(&p);                    // consuma '='
            Node *value = parse_expr(&p);
            if (value != NULL) {
                root = node_new(N_ASSIGN);
                if (root == NULL) {
                    node_free(value);
                    return fail(&p, "memoria esaurita");
                }
                snprintf(root->as.assign.name, sizeof root->as.assign.name, "%s", name.name);
                root->as.assign.value = value;
            }
        } else {
            root = parse_expr(&p);
        }
    } else {
        root = parse_expr(&p);
    }

    if (root != NULL && p.cur.type != T_END) {
        node_free(root);
        return fail(&p, "caratteri inattesi dopo l'espressione");
    }
    return root;
}
