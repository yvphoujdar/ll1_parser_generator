
// ll1 parsing table 

void productionToString(int p, char *out) {
    sprintf(out, "%s -> ", prod[p].lhs);
    for (int i = 0; i < prod[p].rhsLen; i++) { strcat(out, prod[p].rhs[i]); strcat(out, " "); }
}

void buildTable(void) {
    for (int i = 0; i < numNT; i++) for (int j = 0; j < numT; j++) table[i][j] = -1;

    for (int p = 0; p < numProd; p++) {
        int A = ntIndex(prod[p].lhs);
        SymSet firsts; int fCount;
        int nullable = firstOfSequence(prod[p].rhs, prod[p].rhsLen, firsts, &fCount);

        for (int i = 0; i < fCount; i++) {
            if (isEps(firsts[i])) continue;
            int t = termIndex(firsts[i]);
            if (table[A][t] != -1 && table[A][t] != p) printf("Conflict at [%s,%s]\n", NT[A], firsts[i]);
            table[A][t] = p;
        }
        if (nullable)
            for (int k = 0; k < followCount[A]; k++) {
                int t = termIndex(FOLLOWSET[A][k]);
                if (table[A][t] != -1 && table[A][t] != p) printf("Conflict at [%s,%s]\n", NT[A], FOLLOWSET[A][k]);
                table[A][t] = p;
            }
    }
}

void printProductions(const char *label) {
    printf("%s\n", label);
    char buf[100];
    for (int p = 0; p < numProd; p++) { productionToString(p, buf); printf("  P%d: %s\n", p, buf); }
    printf("\n");
}

void printTable(void) {
    printf("LL(1) Parsing Table (cell = production number, - = error):\n\n");
    printf("%-8s", "NT\\T");
    for (int t = 0; t < numT; t++) printf("%-8s", T[t]);
    printf("\n");
    for (int i = 0; i < numNT; i++) {
        printf("%-8s", NT[i]);
        for (int t = 0; t < numT; t++) {
            if (table[i][t] >= 0) printf("P%-7d", table[i][t]);
            else printf("%-8s", "-");
        }
        printf("\n");
    }
    printf("\n");
}