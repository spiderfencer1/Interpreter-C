#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "str.h"
#include <string.h>
#include "vec.h"
#define CRED "\x1b[31m"
#define CGRN "\x1b[32m"
#define CEND "\x1b[0m"
#define USAGE(prog) fprintf(stderr, "%sUsage: %s <filename>%s\n", CGRN, prog, CEND); exit(-1)

char* binops[] = { "+","-","*","/","%","==","<=",">=","!=","<",">","|","&" };
char* punc = "{}();,";
char* keywords[] = { "if","else" };

enum toktype {
	T_BOP,
	T_PNC,
	T_NUM,
	T_STR,
	T_BOL,
	T_KWD,
	T_VAR,
	T_EOF
};

struct token {
	enum toktype type;
	str lexeme;
};

struct token readtok(str s,int* pos) {
	while (sat(&s,*pos) != -1 && isspace(sat(&s,*pos))) { (*pos)++; }
	char c = sat(&s,*pos);
	struct token t;
	if (c == -1) { t.type = T_EOF; }
	else if (isdigit(c)) {
		str num = snew("");
		while (sat(&s,*pos) != -1 && isdigit(sat(&s,*pos))) {
			sadd(&num,sat(&s,*pos));
			(*pos)++;
		}
		t.type = T_NUM;
		t.lexeme = num;
	} else if (c == '\'' || c == '\"') {
		str string = snew("");
		(*pos)++;
		while (sat(&s,*pos) != -1 && sat(&s,*pos) != c) {
			sadd(&string,sat(&s,*pos));
			(*pos)++;
		}
		(*pos)++;
		t.type = T_STR;
		t.lexeme = string;
	} else if (isalpha(c) || c == '_') {
		str word = snew("");
		while (sat(&s,*pos) != -1 && (isalnum(sat(&s,*pos)) || sat(&s,*pos) == '_')) {
			sadd(&word,sat(&s,*pos));
			(*pos)++;
		}
		t.type = T_VAR;
		t.lexeme = word;
		if (scmp(&word,"true") || scmp(&word,"false")) { t.type = T_BOL; }
		for (int i=0;i<sizeof(keywords)/sizeof(char*);i++) {
			if (scmp(&word,keywords[i])) { t.type = T_KWD; break; }
		}
	} else if (strchr(punc,c) != NULL) {
		(*pos)++;
		t.type = T_PNC;
		t.lexeme = snew("");
		sadd(&(t.lexeme), c);
	} else {
		for (int i=0;i<sizeof(binops)/sizeof(char*);i++) {
			if (sncmp(&s,binops[i],strlen(binops[i]))) {
				t.type = T_BOP;
				t.lexeme = snew(binops[i]);
				(*pos) += strlen(binops[i]);
				break;
			}
		}
	}
	return t;
}

void parse(str s) {
	int pos = 0;
	vec tokens = vnew();
	while (1) {
		struct token tok = readtok(s, &pos);
		if (tok.type == T_EOF) { break; }
		vpush(&tokens,&tok);
	}

	for (int i=0;i<tokens.len;i++) {
		struct token* tok = (struct token*)(vget(&tokens,i));
		printf("Token: %s.\n", sget(&(tok->lexeme)));
	}
}

int main(int argc, char** argv) {
	if (argc < 2) { USAGE(argv[0]); }
	FILE* fp = fopen(argv[1], "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		str s = snew("");
		char c;
		while ((c = fgetc(fp)) != EOF) {
			sadd(&s,c);
		}
		fclose(fp);

		parse(s);

		sfree(&s);
	} else { USAGE(argv[0]); }
	return 0;
}
