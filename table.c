#include <stdio.h>
#include <string.h>
#include "table.h"

char terminals[MAX_TERMINALS][10];
int num_terminals = 0;

void build_terminal_list(Grammar *g) {
    num_terminals = 0;
    for (int a = 0; a < g->num_nonterminals; a++) {
        NonTerminal *A = &g->nonterminals[a];
        for (int p = 0; p < A->num_productions; p++) {
            Production *prod = &A->productions[p];
            for (int i = 0; i < prod->num_symbols; i++) {
                char *sym = prod->symbols[i];
                if (is_nonterminal(g, sym)) continue;
                if (strcmp(sym, "#") == 0) continue;
                int found = 0;
                for (int t = 0; t < num_terminals; t++)
                    if (strcmp(terminals[t], sym) == 0) found = 1;
                if (!found) {
                    strcpy(terminals[num_terminals], sym);
                    num_terminals++;
                }
            }
        }
    }
    strcpy(terminals[num_terminals], "$");
    num_terminals++;
}

int terminal_index(const char *symbol) {
    for (int i = 0; i < num_terminals; i++)
        if (strcmp(terminals[i], symbol) == 0) return i;
    return -1;
}

static void fill_cell(Grammar *g, TableCell table[][MAX_TERMINALS], int a, int p, const char *terminal) {
    int t = terminal_index(terminal);
    if (t == -1) return;
    if (table[a][t].filled) {
        printf("conflict: %s on %s\n", g->nonterminals[a].name, terminals[t]);
        return;
    }
    table[a][t].nonterminal_index = a;
    table[a][t].production_index = p;
    table[a][t].filled = 1;
}

void build_table(Grammar *g, SymbolSet first[], SymbolSet follow[], TableCell table[][MAX_TERMINALS]) {
    for (int a = 0; a < g->num_nonterminals; a++)
        for (int t = 0; t < num_terminals; t++)
            table[a][t].filled = 0;

    for (int a = 0; a < g->num_nonterminals; a++) {
        NonTerminal *A = &g->nonterminals[a];
        for (int p = 0; p < A->num_productions; p++) {
            Production *prod = &A->productions[p];
            SymbolSet prod_first;
            prod_first.count = 0;
            int all_epsilon = 1;

            for (int i = 0; i < prod->num_symbols; i++) {
                char *sym = prod->symbols[i];
                if (strcmp(sym, "#") == 0) {
                    break;
                } else if (!is_nonterminal(g, sym)) {
                    add_to_set(&prod_first, sym);
                    all_epsilon = 0;
                    break;
                } else {
                    int idx = nonterminal_index(g, sym);
                    for (int j = 0; j < first[idx].count; j++)
                        if (strcmp(first[idx].elements[j], "#") != 0)
                            add_to_set(&prod_first, first[idx].elements[j]);
                    if (!is_in_set(&first[idx], "#")) {
                        all_epsilon = 0;
                        break;
                    }
                }
            }

            for (int i = 0; i < prod_first.count; i++)
                fill_cell(g, table, a, p, prod_first.elements[i]);

            if (all_epsilon) {
                for (int j = 0; j < follow[a].count; j++)
                    fill_cell(g, table, a, p, follow[a].elements[j]);
            }
        }
    }
}

void print_table(Grammar *g, TableCell table[][MAX_TERMINALS]) {
    printf("%-6s", "");
    for (int t = 0; t < num_terminals; t++) printf("%-14s", terminals[t]);
    printf("\n");

    for (int a = 0; a < g->num_nonterminals; a++) {
        printf("%-6s", g->nonterminals[a].name);
        for (int t = 0; t < num_terminals; t++) {
            if (table[a][t].filled) {
                Production *prod = &g->nonterminals[table[a][t].nonterminal_index].productions[table[a][t].production_index];
                char buf[40] = "";
                for (int i = 0; i < prod->num_symbols; i++) {
                    strcat(buf, prod->symbols[i]);
                    strcat(buf, " ");
                }
                printf("%-14s", buf);
            } else {
                printf("%-14s", "-");
            }
        }
        printf("\n");
    }
}
