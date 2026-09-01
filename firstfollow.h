#ifndef FIRSTFOLLOW_H
#define FIRSTFOLLOW_H

#include "grammar.h"

#define MAX_SET_SIZE 20

typedef struct {
    char elements[MAX_SET_SIZE][10];
    int count;
} SymbolSet;

int add_to_set(SymbolSet *set, const char *symbol);
int is_in_set(SymbolSet *set, const char *symbol);
void compute_first(Grammar *g, SymbolSet first[]);
void compute_follow(Grammar *g, SymbolSet first[], SymbolSet follow[]);
void print_set(const char *label, SymbolSet *set);

#endif
