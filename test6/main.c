#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C_RED "\x1b[31m"
#define C_GRN "\x1b[32m"
#define C_END "\x1b[0m"
#define USAGE(prog) fprintf(stderr, "%sUsage: %s <filename>%s\n", , prog, "\x1b[0m"); exit(-1)

typedef struct {
	char* text;
	int len;
} string;

string snew(char* text) {
	string s;
	s->text = malloc(strlen(text));
	strcpy(s->text,text);
	s->len = strlen(text);
}

void scat(string* a, string* b) {
	a->text = realloc(a->text, a->len + b->len);
	strcat(a->text, b->text);
}

enum tokentype {
	T_BOP,
	T_PNC,
	T_NUM,
	T_STR,
	T_VAR
};

enum tokentype peek(char* buffer, int pos) {
	if (isdigit(buffer[pos])) { return T_NUM; }
	if (buffer[pos] == '\'' || buffer[pos] == '\"') { return T_STR; }
	if (isalpha(buffer[pos]) || buffer[pos] == '_') { return T_VAR; }
	switch (buffer[pos]) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '=':
		case '!':
		case '<':
		case '>':
		case '|':
		case '&':
			return T_BOP;
		case '(':
		case ')':
		case '{':
		case '}':
		case ';':
		case ',':
			return T_PNC;
		default:
			break;
	}
}

int main(int argc, char** argv) {
	if (argc < 2) { USAGE(argv[0]); }
	FILE* fp = fopen(argv[1], "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = malloc(length + 1);
		fread(buffer, 1, length, fp);
		fclose(fp);
		buffer[length] = 0;

		

		free(buffer);
	} else { USAGE(argv[0]); }
	return 0;
}
