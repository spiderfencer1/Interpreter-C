#include <stdlib.h>
#include "str.h"
#include <string.h>

str snew(char* text) {
	str s;
	s.len = strlen(text);
	s.text = malloc(s.len);
	strcpy(s.text,text);
	return s;
}

void sfree(str* s) {
	free(s->text);
}

void sadd(str* s, char c) {
	s->len++;
	s->text = realloc(s->text,s->len);
	s->text[s->len-1] = c;
}

void scat(str* a, str* b) {
	a->len += b->len;
	a->text = realloc(a->text,a->len);
	strcat(a->text,b->text);
}

char sat(str* s, int pos) {
	if (pos >= 0 && pos < s->len) { return s->text[pos]; }
	return -1;
}

char* sget(str* s) {
	char* c = malloc(s->len+1);
	strcpy(c,s->text);
	c[s->len] = 0;
	return c;
}

int scmp(str* s, char* text) {
	return strcmp(sget(s),text) == 0;
}

int sncmp(str* s, char* text, int len) {
	return strncmp(sget(s),text,len) == 0;
}
