#include <stdio.h>
#include <string.h>
#include "grammar.h"
#include "firstfollow.h"
#include "table.h"
#include "parser.h"

static void run_test(Grammar *g, TableCell table[][MAX_TERMINALS], const char *label, char tokens[][10], int n) {
    printf("\n--- %s ---\n", label);
    parse(g, table, tokens, n);
}

int main() {
    Grammar g;
    build_demo_grammar(&g);

    SymbolSet first[MAX_NONTERMINALS];
    SymbolSet follow[MAX_NONTERMINALS];
    compute_first(&g, first);
    compute_follow(&g, first, follow);

    printf("FIRST sets\n");
    for (int i = 0; i < g.num_nonterminals; i++) {
        char label[20];
        sprintf(label, "FIRST(%s)", g.nonterminals[i].name);
        print_set(label, &first[i]);
    }

    printf("\nFOLLOW sets\n");
    for (int i = 0; i < g.num_nonterminals; i++) {
        char label[20];
        sprintf(label, "FOLLOW(%s)", g.nonterminals[i].name);
        print_set(label, &follow[i]);
    }

    build_terminal_list(&g);

    static TableCell table[MAX_NONTERMINALS][MAX_TERMINALS];
    build_table(&g, first, follow, table);

    printf("\nParsing Table\n");
    print_table(&g, table);

    char t1[][10] = {"id", "+", "id", "$"};
    run_test(&g, table, "id + id", t1, 4);

    char t2[][10] = {"id", "*", "id", "$"};
    run_test(&g, table, "id * id", t2, 4);

    char t3[][10] = {"id", "+", "id", "*", "id", "$"};
    run_test(&g, table, "id + id * id", t3, 6);

    char t4[][10] = {"(", "id", "+", "id", ")", "*", "id", "$"};
    run_test(&g, table, "( id + id ) * id", t4, 8);

    char t5[][10] = {"id", "+", "+", "id", "$"};
    run_test(&g, table, "id + + id", t5, 5);

    char t6[][10] = {"id", "+", "$"};
    run_test(&g, table, "id +", t6, 3);

    char t7[][10] = {"num", "*", "(", "num", "+", "id", ")", "$"};
    run_test(&g, table, "num * ( num + id )", t7, 8);

    return 0;
}
