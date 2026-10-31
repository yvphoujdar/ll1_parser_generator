// predictive parser 

char sentential[MAX_STACK][MAX_SYM_LEN]; int sentLen;
char derivation[MAX_STEPS][300]; int numDeriv = 0;

void recordDerivation(void) {
    char line[300] = "";
    for (int i = 0; i < sentLen; i++) { strcat(line, sentential[i]); strcat(line, " "); }
    strcpy(derivation[numDeriv++], line);
}

void expandSentential(int idx, int p) {
    char newSent[MAX_STACK][MAX_SYM_LEN]; int newLen = 0;
    for (int i = 0; i < idx; i++) strcpy(newSent[newLen++], sentential[i]);
    if (!(prod[p].rhsLen == 1 && isEps(prod[p].rhs[0])))
        for (int i = 0; i < prod[p].rhsLen; i++) strcpy(newSent[newLen++], prod[p].rhs[i]);
    for (int i = idx + 1; i < sentLen; i++) strcpy(newSent[newLen++], sentential[i]);
    sentLen = newLen;
    for (int i = 0; i < sentLen; i++) strcpy(sentential[i], newSent[i]);
    recordDerivation();
}

void parseInput(char tokens[][MAX_SYM_LEN], int numTokens) {
    char stack[MAX_STACK][MAX_SYM_LEN]; int top = 0;
    strcpy(stack[top++], "$");
    strcpy(stack[top++], startSym);

    sentLen = 1; strcpy(sentential[0], startSym);
    numDeriv = 0; recordDerivation();

    int ip = 0, step = 1;
    printf("Predictive parsing trace:\n\n");
    printf("%-6s%-30s%-25s%s\n", "Step", "Stack", "Input", "Action");

    while (1) {
        char *topSym = stack[top - 1];
        char *a = tokens[ip];

        char stackText[300] = "";
        for (int i = 0; i < top; i++) { strcat(stackText, stack[i]); strcat(stackText, " "); }
        char inputText[200] = "";
        for (int i = ip; i < numTokens; i++) { strcat(inputText, tokens[i]); strcat(inputText, " "); }

        if (!strcmp(topSym, "$") && !strcmp(a, "$")) {
            printf("%-6d%-30s%-25s%s\n", step, stackText, inputText, "Accept");
            printf("\nAccepted\n\n");
            printf("Leftmost derivation (syntax-directed translation output):\n");
            for (int i = 0; i < numDeriv; i++) printf("  %s%s\n", derivation[i], (i < numDeriv - 1) ? " =>" : "");
            return;
        }
        if (!isNonTerminal(topSym)) {
            if (!strcmp(topSym, a)) {
                printf("%-6d%-30s%-25s%s\n", step, stackText, inputText, "Match");
                top--; ip++;
            } else {
                printf("%-6d%-30s%-25s%s\n", step, stackText, inputText, "Error: mismatch");
                printf("\nRejected\n"); return;
            }
        } else {
            int A = ntIndex(topSym), t = termIndex(a);
            if (t < 0 || table[A][t] < 0) {
                printf("%-6d%-30s%-25s%s\n", step, stackText, inputText, "Error: no rule");
                printf("\nRejected\n"); return;
            }
            int p = table[A][t];
            char pStr[100]; productionToString(p, pStr);
            char actionText[120]; sprintf(actionText, "Output %s(P%d)", pStr, p);
            printf("%-6d%-30s%-25s%s\n", step, stackText, inputText, actionText);

            int sentIdx = 0; while (strcmp(sentential[sentIdx], topSym)) sentIdx++;
            expandSentential(sentIdx, p);

            top--;
            if (!(prod[p].rhsLen == 1 && isEps(prod[p].rhs[0])))
                for (int i = prod[p].rhsLen - 1; i >= 0; i--) strcpy(stack[top++], prod[p].rhs[i]);
        }
        step++;
    }
}

int main(void) {
    printf("Select a grammar:\n");
    printf("1. Arithmetic expressions   (id + id * id)\n");
    printf("2. Boolean expressions      (id and id or not id)\n");
    printf("3. If-else / statements     (if ( id ) { id = id ; } else { id = id ; })\n");
    printf("4. Array declaration        (type id [ num ] [ num ] ;)\n");
    printf("5. Function call / params   (id ( id , id , id ) ;)\n");
    printf("Enter choice (1-5): ");

    int choice; scanf("%d", &choice);
    char filename[20]; sprintf(filename, "grammar%d.txt", choice);

    loadGrammar(filename);
    printf("\n");
    printProductions("Original productions (from grammar file):");

    removeLeftRecursion();
    leftFactor();
    printProductions("Productions after eliminating left recursion and left factoring:");

    computeFirst();
    computeFollow();
    buildTable();

    printFirst();
    printFollow();
    printTable();

    char line[MAX_LINE];
    printf("Enter input string (space-separated tokens, '$' optional): ");
    scanf(" %[^\n]", line);

    char tokens[MAX_TOKENS][MAX_SYM_LEN]; int numTokens = 0;
    char *tok = strtok(line, " \t");
    while (tok) { strcpy(tokens[numTokens++], tok); tok = strtok(NULL, " \t"); }
    if (strcmp(tokens[numTokens - 1], "$")) strcpy(tokens[numTokens++], "$");

    printf("\n");
    parseInput(tokens, numTokens);
    return 0;
}