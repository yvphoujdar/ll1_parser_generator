#include <stdio.h>
#include <string.h>
#include "parser.h"

#define MAX_STACK 100

typedef struct {
    char items[MAX_STACK][10];
    int top;
} Stack;

static void push(Stack *s, const char *symbol) {
    s->top++;
    strcpy(s->items[s->top], symbol);
}

static void pop(Stack *s) {
    s->top--;
}

static char* peek(Stack *s) {
    return s->items[s->top];
}

void parse(Grammar *g, TableCell table[][MAX_TERMINALS], char tokens[][10], int num_tokens) {
    Stack stack;
    stack.top = -1;
    push(&stack, "$");
    push(&stack, g->start_symbol);

    int ip = 0;

    while (strcmp(peek(&stack), "$") != 0) {
        char *top = peek(&stack);
        char *current = tokens[ip];

        if (!is_nonterminal(g, top)) {
            if (strcmp(top, current) == 0) {
                printf("match %s\n", top);
                pop(&stack);
                ip++;
            } else {
                printf("error: expected %s found %s\n", top, current);
                return;
            }
        } else {
            int a = nonterminal_index(g, top);
            int t = terminal_index(current);

            if (t == -1 || !table[a][t].filled) {
                printf("error: no rule for %s on %s\n", top, current);
                return;
            }

            Production *prod = &g->nonterminals[a].productions[table[a][t].production_index];

            printf("%s -> ", top);
            for (int i = 0; i < prod->num_symbols; i++) printf("%s ", prod->symbols[i]);
            printf("\n");

            pop(&stack);

            if (!(prod->num_symbols == 1 && strcmp(prod->symbols[0], "#") == 0)) {
                for (int i = prod->num_symbols - 1; i >= 0; i--) push(&stack, prod->symbols[i]);
            }
        }
    }

    if (strcmp(tokens[ip], "$") == 0) {
        printf("accepted\n");
    } else {
        printf("error: input left over\n");
    }
}
