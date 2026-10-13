# Architettura di calc

Questo documento raccoglie gli schemi del progetto: le strutture dati, il percorso di una riga attraverso i moduli,
la gestione della memoria e degli errori. Per l'uso, vedi il [README](../README.md).

## Gli schemi dell'articolo

| Le tre fasi | I sei moduli | L'albero di `x = 3 + 4 * 2` |
|---|---|---|
| ![Pipeline: lexer, parser, eval](img/pipeline.png) | ![Dipendenze tra i moduli](img/moduli.png) | ![Albero sintattico](img/albero_ast.png) |

## Le strutture dati

```mermaid
classDiagram
    direction LR
    class Token {
        TokenType type
        double num
        char name[32]
        size_t pos
    }
    class Lexer {
        const char* src
        size_t pos
        lexer_next() Token
    }
    class Node {
        NodeKind kind
        union as
    }
    class Env {
        <<opaco>>
        Var* vars
        env_get(name, *value) bool
        env_set(name, value) int
    }
    class Var {
        char name[32]
        double value
        Var* next
    }
    class ParseError {
        char message[96]
        size_t pos
    }
    Lexer ..> Token : produce
    Node --> Node : figli (neg, bin.l, bin.r, assign.value)
    Env *-- Var : lista concatenata
    Var --> Var : next
```

### Node: una tagged union

`kind` dice quale campo della union `as` è valido. Ogni tipo di nodo usa solo la sua parte:

| `kind` | Campo valido | Esempio | Figli |
|---|---|---|---|
| `N_NUM` | `as.num` | `3` | nessuno (foglia) |
| `N_VAR` | `as.name` | `x` | nessuno (foglia) |
| `N_NEG` | `as.neg.operand` | `-x` | 1 |
| `N_BIN` | `as.bin.op`, `as.bin.l`, `as.bin.r` | `a + b` | 2 |
| `N_ASSIGN` | `as.assign.name`, `as.assign.value` | `x = 1 + 2` | 1 |

`node_free` e `eval` sono entrambi uno `switch` su `kind` con un caso per riga della tabella.

### Env: un tipo opaco

`env.h` dichiara solo `typedef struct Env Env;`: fuori da `env.c` nessuno conosce i campi, quindi la lista concatenata
si potrebbe sostituire con una tabella hash senza toccare `main.c` né `eval.c`. Un nuovo nome viene inserito in testa
alla lista; assegnare un nome già esistente aggiorna il valore senza allocare.

## Il percorso di una riga

Che cosa succede quando l'utente scrive `x = 3 + 4 * 2`:

```mermaid
sequenceDiagram
    autonumber
    participant M as main
    participant P as parser
    participant L as lexer
    participant E as eval
    participant V as env

    M->>P: parse_line("x = 3 + 4 * 2", &err)
    P->>L: lexer_next() → IDENT x
    P->>L: copia del lexer: lexer_next() → '='
    Note over P: è un'assegnazione
    loop expr / term / unario / primario
        P->>L: lexer_next()
        L-->>P: NUM, '+', NUM, '*', NUM, END
    end
    P-->>M: albero (N_ASSIGN)
    M->>E: eval(albero, env, &valore, err)
    E->>E: eval(+) → eval(3), eval(*) → eval(4), eval(2)
    E->>V: env_set("x", 11)
    E-->>M: true, valore = 11
    M->>M: printf("x = 11")
    M->>M: node_free(albero)
```

Il passo 3 è l'unico punto in cui il parser guarda avanti di un token: `x = ...` e `x + ...` iniziano allo stesso modo,
e solo il token successivo dice quale delle due regole di `riga` applicare. Il lexer è una semplice `struct`
(testo + posizione), quindi «guardare avanti» vuol dire copiarlo e chiamare `lexer_next` sulla copia.

### La valutazione dell'albero

`eval` visita l'albero in **post-ordine**: prima i figli, poi il nodo. I numeri tra parentesi sono l'ordine in cui
i nodi producono il loro valore:

```mermaid
flowchart TD
    A["(6) x = 11"] --> B["(5) 3 + 8 = 11"]
    B --> C["(1) 3"]
    B --> D["(4) 4 * 2 = 8"]
    D --> E["(2) 4"]
    D --> F["(3) 2"]
```

## Errori

Gli errori non interrompono il programma: ogni riga sbagliata viene segnalata su `stderr` e si passa alla successiva.
Alla fine, se c'è stato almeno un errore, il codice d'uscita è 1.

```mermaid
flowchart TD
    R["riga letta"] --> LUN{"più di 255<br/>caratteri?"}
    LUN -- sì --> E1["errore: riga troppo lunga"]
    LUN -- no --> P["parse_line"]
    P --> VUOTA{"riga vuota<br/>o commento?"}
    VUOTA -- sì --> NEXT["riga successiva"]
    VUOTA -- no --> OK{"albero<br/>costruito?"}
    OK -- no --> E2["errore di sintassi<br/>con la freccia sulla colonna"]
    OK -- sì --> EV{"eval riuscito?"}
    EV -- no --> E3["divisione per zero<br/>o variabile non definita"]
    EV -- sì --> OUT["stampa il valore"]
    E1 --> NEXT
    E2 --> NEXT
    E3 --> FREE["node_free"]
    OUT --> FREE
    FREE --> NEXT
```

Due regole tengono puliti i messaggi:

- **vale il primo errore.** Quando il parser fallisce, le funzioni che lo hanno chiamato falliscono a catena;
  `fail()` registra il messaggio solo se è ancora vuoto, così gli «echi» successivi non coprono la causa vera;
- **ogni errore ha una posizione.** Ogni token porta con sé la sua colonna (`pos`), e `main` la usa per disegnare
  la freccia `^` sotto la riga.

## Memoria

Ogni riga alloca il suo albero e lo libera prima di passare alla successiva; le variabili vivono nell'ambiente
fino alla fine del programma.

| Chi alloca | Che cosa | Chi libera |
|---|---|---|
| `parser.c` (`node_new`) | i nodi dell'albero di una riga | `main.c`, con `node_free` dopo `eval` |
| `parser.c`, se fallisce a metà | i sotto-alberi già costruiti | il parser stesso, prima di restituire `NULL` |
| `env.c` (`env_set`) | una `Var` per ogni nome nuovo | `env_free`, alla fine di `main` |
| `env.c` (`env_new`) | l'ambiente | `env_free` |

`make debug` compila con AddressSanitizer: `make debug && make test` esegue tutti gli esempi, compresi quelli con
errori a metà riga, senza perdite né accessi fuori dai blocchi.
