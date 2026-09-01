#ifndef GRAMMAR_H
#define GRAMMAR_H

#define MAX_NONTERMINALS 10
#define MAX_PRODUCTIONS 10
#define MAX_SYMBOLS 10

typedef struct {
    char symbols[MAX_SYMBOLS][10];
    int num_symbols;
} Production;

typedef struct {
    char name[10];
    Production productions[MAX_PRODUCTIONS];
    int num_productions;
} NonTerminal;

typedef struct {
    NonTerminal nonterminals[MAX_NONTERMINALS];
    int num_nonterminals;
    char start_symbol[10];
} Grammar;

void build_demo_grammar(Grammar *g);
int is_nonterminal(Grammar *g, const char *symbol);
int nonterminal_index(Grammar *g, const char *symbol);

#endif
