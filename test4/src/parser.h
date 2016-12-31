#ifndef SRC_PARSER_H
#define SRC_PARSER_H

#include <stdio.h>

FILE* out;

void p_init(void);
void p_close(void);
void parse(char*);
char* expr(char*);
char* binary(char*,char*);
char* atom(char*);
char* parseIf(char*);
char* parseBrackets(char*);
char* parseParentheses(char*);

#endif
