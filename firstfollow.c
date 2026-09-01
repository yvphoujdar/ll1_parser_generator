#include <stdio.h>
#include <string.h>
#include "firstfollow.h"

int add_to_set(SymbolSet *set, const char *symbol) {
    for (int i = 0; i < set->count; i++)
        if (strcmp(set->elements[i], symbol) == 0) return 0;
    strcpy(set->elements[set->count], symbol);
    set->count++;
    return 1;
}

int is_in_set(SymbolSet *set, const char *symbol) {
    for (int i = 0; i < set->count; i++)
        if (strcmp(set->elements[i], symbol) == 0) return 1;
    return 0;
}

static int first_of_sequence(Grammar *g, SymbolSet first[], char symbols[][10], int start, int count, SymbolSet *result) {
    int all_epsilon = 1;
    for (int i = start; i < count; i++) {
        char *sym = symbols[i];
        if (strcmp(sym, "#") == 0) {
            break;
        } else if (!is_nonterminal(g, sym)) {
            add_to_set(result, sym);
            all_epsilon = 0;
            break;
        } else {
            int idx = nonterminal_index(g, sym);
            for (int j = 0; j < first[idx].count; j++) {
                if (strcmp(first[idx].elements[j], "#") != 0)
                    add_to_set(result, first[idx].elements[j]);
            }
            if (!is_in_set(&first[idx], "#")) {
                all_epsilon = 0;
                break;
            }
        }
    }
    if (all_epsilon) add_to_set(result, "#");
    return all_epsilon;
}

void compute_first(Grammar *g, SymbolSet first[]) {
    for (int i = 0; i < g->num_nonterminals; i++) first[i].count = 0;

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int a = 0; a < g->num_nonterminals; a++) {
            NonTerminal *A = &g->nonterminals[a];
            for (int p = 0; p < A->num_productions; p++) {
                Production *prod = &A->productions[p];
                SymbolSet temp;
                temp.count = 0;
                first_of_sequence(g, first, prod->symbols, 0, prod->num_symbols, &temp);
                for (int i = 0; i < temp.count; i++) {
                    if (add_to_set(&first[a], temp.elements[i])) changed = 1;
                }
            }
        }
    }
}

void compute_follow(Grammar *g, SymbolSet first[], SymbolSet follow[]) {
    for (int i = 0; i < g->num_nonterminals; i++) follow[i].count = 0;

    int start_idx = nonterminal_index(g, g->start_symbol);
    add_to_set(&follow[start_idx], "$");

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int a = 0; a < g->num_nonterminals; a++) {
            NonTerminal *A = &g->nonterminals[a];
            for (int p = 0; p < A->num_productions; p++) {
                Production *prod = &A->productions[p];
                for (int i = 0; i < prod->num_symbols; i++) {
                    char *Xi = prod->symbols[i];
                    if (!is_nonterminal(g, Xi)) continue;
                    int xi_idx = nonterminal_index(g, Xi);

                    SymbolSet beta_first;
                    beta_first.count = 0;
                    int beta_epsilon = first_of_sequence(g, first, prod->symbols, i + 1, prod->num_symbols, &beta_first);

                    for (int j = 0; j < beta_first.count; j++) {
                        if (strcmp(beta_first.elements[j], "#") != 0)
                            if (add_to_set(&follow[xi_idx], beta_first.elements[j])) changed = 1;
                    }

                    if (i + 1 == prod->num_symbols || beta_epsilon) {
                        for (int j = 0; j < follow[a].count; j++)
                            if (add_to_set(&follow[xi_idx], follow[a].elements[j])) changed = 1;
                    }
                }
            }
        }
    }
}

void print_set(const char *label, SymbolSet *set) {
    printf("%s = { ", label);
    for (int i = 0; i < set->count; i++) printf("%s ", set->elements[i]);
    printf("}\n");
}
