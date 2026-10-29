#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SYM_LEN 20
#define MAX_RHS     10
#define MAX_ALTS    10
#define MAX_PROD    30
#define MAX_NT      15
#define MAX_T       20
#define MAX_STACK   100
#define MAX_TOKENS  60
#define MAX_STEPS   100
#define MAX_LINE    200

typedef char SymSet[MAX_T][MAX_SYM_LEN];

typedef struct {
    char lhs[MAX_SYM_LEN];
    char rhs[MAX_RHS][MAX_SYM_LEN];
    int  rhsLen;
} Production;

Production prod[MAX_PROD]; int numProd = 0;
char NT[MAX_NT][MAX_SYM_LEN]; int numNT = 0;
char T[MAX_T][MAX_SYM_LEN];   int numT = 0;
char startSym[MAX_SYM_LEN];

SymSet FIRSTSET[MAX_NT]; int firstCount[MAX_NT];
SymSet FOLLOWSET[MAX_NT]; int followCount[MAX_NT];
int table[MAX_NT][MAX_T];

int ntIndex(const char *s) { for (int i = 0; i < numNT; i++) if (!strcmp(NT[i], s)) return i; return -1; }
int termIndex(const char *s) { for (int i = 0; i < numT; i++) if (!strcmp(T[i], s)) return i; return -1; }
int isNonTerminal(const char *s) { return ntIndex(s) >= 0; }
int isEps(const char *s) { return !strcmp(s, "eps"); }

int addToSet(char set[][MAX_SYM_LEN], int *count, const char *sym) {
    for (int i = 0; i < *count; i++) if (!strcmp(set[i], sym)) return 0;
    strcpy(set[*count], sym); (*count)++; return 1;
}
int inSet(char set[][MAX_SYM_LEN], int count, const char *sym) {
    for (int i = 0; i < count; i++) if (!strcmp(set[i], sym)) return 1;
    return 0;
}

// grammer files
 
void loadGrammar(const char *filename) {
    numProd = 0; numNT = 0; numT = 0;
    FILE *fp = fopen(filename, "r");
    if (!fp) { printf("Could not open %s\n", filename); exit(1); }

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char *arrow = strstr(line, "->");
        char lhsName[MAX_SYM_LEN]; sscanf(line, "%s", lhsName);
        if (ntIndex(lhsName) < 0) strcpy(NT[numNT++], lhsName);

        char *saveOuter;
        char *alt = strtok_r(arrow + 2, "|", &saveOuter);
        while (alt) {
            char altCopy[MAX_LINE]; strcpy(altCopy, alt);
            char *saveInner;
            char *tok = strtok_r(altCopy, " \t", &saveInner);
            prod[numProd].rhsLen = 0;
            strcpy(prod[numProd].lhs, lhsName);
            while (tok) {
                strcpy(prod[numProd].rhs[prod[numProd].rhsLen++], tok);
                tok = strtok_r(NULL, " \t", &saveInner);
            }
            numProd++;
            alt = strtok_r(NULL, "|", &saveOuter);
        }
    }
    fclose(fp);
    strcpy(startSym, NT[0]);

    for (int p = 0; p < numProd; p++)
        for (int i = 0; i < prod[p].rhsLen; i++) {
            char *s = prod[p].rhs[i];
            if (!isEps(s) && !isNonTerminal(s) && termIndex(s) < 0) strcpy(T[numT++], s);
        }
    strcpy(T[numT++], "$");
}

// left recursion and factoring removing 

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


// first set

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