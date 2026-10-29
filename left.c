void generateNewName(const char *base, char *out) {
    strcpy(out, base); strcat(out, "'");
    while (ntIndex(out) >= 0) strcat(out, "'");
}

void addProduction(const char *lhs, char rhs[][MAX_SYM_LEN], int rhsLen) {
    strcpy(prod[numProd].lhs, lhs);
    prod[numProd].rhsLen = rhsLen;
    for (int i = 0; i < rhsLen; i++) strcpy(prod[numProd].rhs[i], rhs[i]);
    numProd++;
}

void removeMarked(int *marked) {
    int w = 0;
    for (int i = 0; i < numProd; i++) if (!marked[i]) prod[w++] = prod[i];
    numProd = w;
}

void removeLeftRecursion(void) {
    int originalNumNT = numNT;
    for (int ai = 0; ai < originalNumNT; ai++) {
        char A[MAX_SYM_LEN]; strcpy(A, NT[ai]);
        char alphaList[MAX_ALTS][MAX_RHS][MAX_SYM_LEN]; int alphaLen[MAX_ALTS]; int alphaCount = 0;
        char betaList[MAX_ALTS][MAX_RHS][MAX_SYM_LEN];  int betaLen[MAX_ALTS];  int betaCount = 0;
        int marked[MAX_PROD] = {0};

        for (int p = 0; p < numProd; p++) {
            if (strcmp(prod[p].lhs, A)) continue;
            if (prod[p].rhsLen >= 1 && !strcmp(prod[p].rhs[0], A)) {
                alphaLen[alphaCount] = prod[p].rhsLen - 1;
                for (int k = 0; k < alphaLen[alphaCount]; k++) strcpy(alphaList[alphaCount][k], prod[p].rhs[k + 1]);
                alphaCount++;
            } else {
                betaLen[betaCount] = prod[p].rhsLen;
                for (int k = 0; k < betaLen[betaCount]; k++) strcpy(betaList[betaCount][k], prod[p].rhs[k]);
                betaCount++;
            }
            marked[p] = 1;
        }
        if (alphaCount == 0) continue;

        char newName[MAX_SYM_LEN]; generateNewName(A, newName);
        strcpy(NT[numNT++], newName);
        removeMarked(marked);

        for (int i = 0; i < betaCount; i++) {
            char rhs[MAX_RHS][MAX_SYM_LEN]; int len = 0;
            for (int k = 0; k < betaLen[i]; k++) strcpy(rhs[len++], betaList[i][k]);
            strcpy(rhs[len++], newName);
            addProduction(A, rhs, len);
        }
        for (int i = 0; i < alphaCount; i++) {
            char rhs[MAX_RHS][MAX_SYM_LEN]; int len = 0;
            for (int k = 0; k < alphaLen[i]; k++) strcpy(rhs[len++], alphaList[i][k]);
            strcpy(rhs[len++], newName);
            addProduction(newName, rhs, len);
        }
        char epsRhs[1][MAX_SYM_LEN]; strcpy(epsRhs[0], "eps");
        addProduction(newName, epsRhs, 1);
    }
}

int tryFactorOnePass(void) {
    for (int ai = 0; ai < numNT; ai++) {
        char A[MAX_SYM_LEN]; strcpy(A, NT[ai]);
        int idxs[MAX_ALTS], cnt = 0;
        for (int p = 0; p < numProd; p++) if (!strcmp(prod[p].lhs, A)) idxs[cnt++] = p;
        if (cnt < 2) continue;

        for (int i = 0; i < cnt; i++) {
            char *first = prod[idxs[i]].rhs[0];
            int group[MAX_ALTS], gcnt = 0;
            for (int j = 0; j < cnt; j++) if (!strcmp(prod[idxs[j]].rhs[0], first)) group[gcnt++] = idxs[j];
            if (gcnt < 2) continue;

            int prefixLen = 1, extend = 1;
            while (extend) {
                if (prod[group[0]].rhsLen <= prefixLen) break;
                char *cand = prod[group[0]].rhs[prefixLen];
                for (int k = 1; k < gcnt; k++)
                    if (prod[group[k]].rhsLen <= prefixLen || strcmp(prod[group[k]].rhs[prefixLen], cand)) { extend = 0; break; }
                if (extend) prefixLen++;
            }

            char prefixTokens[MAX_RHS][MAX_SYM_LEN];
            for (int t = 0; t < prefixLen; t++) strcpy(prefixTokens[t], prod[group[0]].rhs[t]);

            char remRhs[MAX_ALTS][MAX_RHS][MAX_SYM_LEN]; int remLen[MAX_ALTS];
            for (int k = 0; k < gcnt; k++) {
                int p = group[k];
                int len = prod[p].rhsLen - prefixLen;
                if (len <= 0) { strcpy(remRhs[k][0], "eps"); remLen[k] = 1; }
                else { for (int t = 0; t < len; t++) strcpy(remRhs[k][t], prod[p].rhs[prefixLen + t]); remLen[k] = len; }
            }

            char newName[MAX_SYM_LEN]; generateNewName(A, newName);
            strcpy(NT[numNT++], newName);

            int marked[MAX_PROD] = {0};
            for (int k = 0; k < gcnt; k++) marked[group[k]] = 1;
            removeMarked(marked);

            char newRhs[MAX_RHS][MAX_SYM_LEN]; int len = 0;
            for (int t = 0; t < prefixLen; t++) strcpy(newRhs[len++], prefixTokens[t]);
            strcpy(newRhs[len++], newName);
            addProduction(A, newRhs, len);

            for (int k = 0; k < gcnt; k++) addProduction(newName, remRhs[k], remLen[k]);
            return 1;
        }
    }
    return 0;
}

void leftFactor(void) { while (tryFactorOnePass()); }
