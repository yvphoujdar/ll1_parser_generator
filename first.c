
int firstOfSequence(char rhs[][MAX_SYM_LEN], int len, SymSet out, int *outCount) {
    *outCount = 0;
    int allNullable = 1;
    for (int k = 0; k < len && allNullable; k++) {
        char *X = rhs[k];
        if (isEps(X)) { addToSet(out, outCount, "eps"); continue; }
        if (isNonTerminal(X)) {
            int ni = ntIndex(X);
            for (int i = 0; i < firstCount[ni]; i++)
                if (!isEps(FIRSTSET[ni][i])) addToSet(out, outCount, FIRSTSET[ni][i]);
            if (!inSet(FIRSTSET[ni], firstCount[ni], "eps")) allNullable = 0;
        } else {
            addToSet(out, outCount, X);
            allNullable = 0;
        }
    }
    if (allNullable) addToSet(out, outCount, "eps");
    return inSet(out, *outCount, "eps");
}

void computeFirst(void) {
    for (int i = 0; i < numNT; i++) firstCount[i] = 0;
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < numProd; p++) {
            int A = ntIndex(prod[p].lhs);
            SymSet tmp; int tmpCount;
            firstOfSequence(prod[p].rhs, prod[p].rhsLen, tmp, &tmpCount);
            for (int i = 0; i < tmpCount; i++)
                if (addToSet(FIRSTSET[A], &firstCount[A], tmp[i])) changed = 1;
        }
    }
}

void printFirst(void) {
    printf("FIRST sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("  FIRST(%s) = { ", NT[i]);
        for (int j = 0; j < firstCount[i]; j++) printf("%s ", FIRSTSET[i][j]);
        printf("}\n");
    }
    printf("\n");
}

// follow 


void computeFollow(void) {
    for (int i = 0; i < numNT; i++) followCount[i] = 0;
    addToSet(FOLLOWSET[ntIndex(startSym)], &followCount[ntIndex(startSym)], "$");

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int p = 0; p < numProd; p++) {
            int A = ntIndex(prod[p].lhs);
            for (int i = 0; i < prod[p].rhsLen; i++) {
                char *Xi = prod[p].rhs[i];
                if (isEps(Xi) || !isNonTerminal(Xi)) continue;
                int ni = ntIndex(Xi);

                SymSet tmp; int tmpCount;
                int nullable = firstOfSequence(prod[p].rhs + i + 1, prod[p].rhsLen - i - 1, tmp, &tmpCount);
                for (int j = 0; j < tmpCount; j++)
                    if (!isEps(tmp[j]) && addToSet(FOLLOWSET[ni], &followCount[ni], tmp[j])) changed = 1;

                if (nullable)
                    for (int k = 0; k < followCount[A]; k++)
                        if (addToSet(FOLLOWSET[ni], &followCount[ni], FOLLOWSET[A][k])) changed = 1;
            }
        }
    }
}

void printFollow(void) {
    printf("FOLLOW sets:\n");
    for (int i = 0; i < numNT; i++) {
        printf("  FOLLOW(%s) = { ", NT[i]);
        for (int j = 0; j < followCount[i]; j++) printf("%s ", FOLLOWSET[i][j]);
        printf("}\n");
    }
    printf("\n");
}
