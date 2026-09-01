#ifndef PARSER_H
#define PARSER_H

#include "grammar.h"
#include "table.h"

void parse(Grammar *g, TableCell table[][MAX_TERMINALS], char tokens[][10], int num_tokens);

#endif
