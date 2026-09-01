#include <string.h>
#include "grammar.h"

static void add_nonterminal(Grammar *g, const char *name) {
    strcpy(g->nonterminals[g->num_nonterminals].name, name);
    g->nonterminals[g->num_nonterminals].num_productions = 0;
    g->num_nonterminals++;
}

int nonterminal_index(Grammar *g, const char *symbol) {
    for (int i = 0; i < g->num_nonterminals; i++)
        if (strcmp(g->nonterminals[i].name, symbol) == 0) return i;
    return -1;
}

int is_nonterminal(Grammar *g, const char *symbol) {
    return nonterminal_index(g, symbol) != -1;
}

static void add_production(Grammar *g, const char *ntname, char symbols[][10], int count) {
    int idx = nonterminal_index(g, ntname);
    NonTerminal *nt = &g->nonterminals[idx];
    Production *p = &nt->productions[nt->num_productions];
    for (int i = 0; i < count; i++) strcpy(p->symbols[i], symbols[i]);
    p->num_symbols = count;
    nt->num_productions++;
}

void build_demo_grammar(Grammar *g) {
    g->num_nonterminals = 0;
    strcpy(g->start_symbol, "E");

    add_nonterminal(g, "E");
    add_nonterminal(g, "E'");
    add_nonterminal(g, "T");
    add_nonterminal(g, "T'");
    add_nonterminal(g, "F");

    char p1[][10] = {"T", "E'"};
    add_production(g, "E", p1, 2);

    char p2[][10] = {"+", "T", "E'"};
    add_production(g, "E'", p2, 3);
    char p3[][10] = {"-", "T", "E'"};
    add_production(g, "E'", p3, 3);
    char p4[][10] = {"#"};
    add_production(g, "E'", p4, 1);

    char p5[][10] = {"F", "T'"};
    add_production(g, "T", p5, 2);

    char p6[][10] = {"*", "F", "T'"};
    add_production(g, "T'", p6, 3);
    char p7[][10] = {"/", "F", "T'"};
    add_production(g, "T'", p7, 3);
    char p8[][10] = {"#"};
    add_production(g, "T'", p8, 1);

    char p9[][10] = {"(", "E", ")"};
    add_production(g, "F", p9, 3);
    char p10[][10] = {"id"};
    add_production(g, "F", p10, 1);
    char p11[][10] = {"num"};
    add_production(g, "F", p11, 1);
}
