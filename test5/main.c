#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __DEBUG__ 1
#if __DEBUG__ == 1
#	define DEBUG(msg) { printf("%sDEBUG(): %s.%s\n", "\x1b[31m", msg, "\x1b[0m"); }
#else
#	define DEBUG(msg)
#endif

typedef struct {
	char* value;
	int length;
} str;

void sinit(str* s) {
	s->value = malloc(0);
	s->length = 0;
}

void sfree(str* s) { free(s->value); }

void sadd(str* s, char c) {
	s->value = realloc(s->value, s->length+1);
	s->value[s->length++] = c;
}

void scat(str* s, char* a) {
	for (int i=0;i<strlen(a);i++) {
		sadd(s,a[i]);
	}
}

char sat(str* s, int i) {
	if (i >= 0 && i < s->length) { return s->value[i]; }
	return -1;
}

char* sget(str* s) {
	char* c = malloc(s->length+1);
	strcpy(c,s->value);
	strcat(c,"\0");
	return c;
}

typedef struct {
	void** value;
	int length;
} vec;

void vecinit(vec* v) {
	v->value = malloc(0);
	v->length = 0;
}                     

void vecpush(vec* v, void* value) {
	v->value = realloc(v->value, v->length+1);
	v->value[v->length++] = value;
}

void* vecget(vec* v, int i) {
	if (i >= 0 && i < v->length) { return v->value[i]; }
	return NULL;
}

void usage(char* prog)
{
	fprintf(stderr, "%sUsage: %s <filename>%s\n", "\x1b[32m", prog, "\x1b[0m");
	exit(1);
}

enum l_state {
	S_ORG,
	S_NUM,
	S_STR,
	S_VAR,
	S_CMT
};

char* binops[] = { "+","-","*","/","==","<=",">=","!=","=","<",">","|","&" };
char* punc = "{}();,";
char* keywords[] = { "if","else" };

vec lex(str buffer) {
	vec tokens;
	vecinit(&tokens);

	int pos = 0;
	int lineno = 1;

	enum l_state state = S_ORG;

	while (pos < buffer.length) {
		while (pos < buffer.length && isspace(sat(&buffer,pos))) {
			if (sat(&buffer,pos) == '\n') { lineno++; pos++; }
			pos++;
		}
		switch (state) {
			case S_ORG:
				{
					for (int i=0;i<sizeof(binops)/sizeof(char*);i++) {
						if (buffer.length - pos >= strlen(binops[i]) && strncmp(sget(&buffer) + pos, binops[i], strlen(binops[i]))) {
							str lexeme; sinit(&lexeme);
							scat(&lexeme,binops[i]);
							vecpush(&tokens,&lexeme);
							pos += strlen(binops[i]) - 1;
							break;
						}
					}
					if (pos < buffer.length)
					{
						if (strchr(punc,sat(&buffer,pos)) != NULL) {
							str lexeme; sinit(&lexeme);
							sadd(&lexeme,sat(&buffer,pos));
							vecpush(&tokens,&lexeme);
							break;
						}
						if (isdigit(sat(&buffer,pos))) {
							pos--;
							state = S_NUM;
							break;
						}
						if (sat(&buffer,pos) == '\'' || sat(&buffer,pos) == '\"') {
							pos--;
							state = S_STR;
							break;
						}
						if (isalpha(sat(&buffer,pos)) || sat(&buffer,pos) == '_') {
							pos--;
							state = S_VAR;
							break;
						}
					}
					break;
				}
			case S_NUM:
				{
					str lexeme;
					sinit(&lexeme);
					while (pos < buffer.length && isdigit(sat(&buffer,pos))) {
						sadd(&lexeme,sat(&buffer,pos++));
					}
					vecpush(&tokens,&lexeme);
					state = S_ORG;
					pos--;
					break;
				}
			case S_STR:
				{
					char end = sat(&buffer,pos++);
					str lexeme;
					sinit(&lexeme);
					while (pos < buffer.length && sat(&buffer,pos) != end) {
						sadd(&lexeme,sat(&buffer,pos++));
					}
					vecpush(&tokens,&lexeme);
					state = S_ORG;
					break;
				}
			case S_VAR:
				{
					str lexeme;
					sinit(&lexeme);
					while (pos < buffer.length && (isalnum(sat(&buffer,pos)) || sat(&buffer,pos) == '_')) {
						sadd(&lexeme,sat(&buffer,pos++));
					}
					vecpush(&tokens,&lexeme);
					state = S_ORG;
					break;
				}
			case S_CMT:
				{
					while (pos < buffer.length && sat(&buffer,pos) != '\n') { pos++; }
					pos++;
					lineno++;
					break;
				}
			default:
				break;
		}
		pos++;
	}

	return tokens;
}

int main(int argc, char** argv) {
	if (argc < 2) { usage(argv[0]); }
	FILE* fp = fopen(argv[1], "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = malloc(length);
		fread(buffer, 1, length, fp);
		fclose(fp);

		str input;
		sinit(&input);
		scat(&input,buffer);

		vec v = lex(input);

		for (int i=0;i<v.length;i++) {
			str* s = (str*)(vecget(&v,i));
			printf("%s\n", sget(s));
		}

		sfree(&input);
		free(buffer);
	} else { usage(argv[0]); }
	return 0;
}
