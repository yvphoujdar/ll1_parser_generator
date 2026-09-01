#ifndef TABLE_H
#define TABLE_H

#include "grammar.h"
#include "firstfollow.h"

#define MAX_TERMINALS 15

typedef struct {
    int nonterminal_index;
    int production_index;
    int filled;
} TableCell;

extern char terminals[MAX_TERMINALS][10];
extern int num_terminals;

void build_terminal_list(Grammar *g);
int terminal_index(const char *symbol);
void build_table(Grammar *g, SymbolSet first[], SymbolSet follow[], TableCell table[][MAX_TERMINALS]);
void print_table(Grammar *g, TableCell table[][MAX_TERMINALS]);

#endif
