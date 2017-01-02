#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CRED "\x1b[31m"
#define CGRN "\x1b[32m"
#define CEND "\x1b[0m"
#define USAGE(prog) fprintf(stderr, "%sUsage: %s <filename>%s\n", CGRN, prog, CEND); exit(-1)

enum tokentype {
	T_BOP,
	T_PNC,
	T_NUM,
	T_STR,
	T_BOL,
	T_KWD,
	T_VAR,
	T_EOF
};

char* src;
enum tokentype type;
char lookahead;
char* lexeme;
int lexeme_len;

char* binops = "+-*/%<>=!|&";
char* punc = "{}();,";
char* keywords[] = { "if","else" };

void lexer_init(void) {
	lexeme = malloc(0);
	lexeme_len = 0;
}

void lexer_free(void) { free(lexeme); }

void lexer_peek(void) {
	while (*src && isspace(*src)) { src++; }
	if (*src == '\0') { type = T_EOF; return; }
	lookahead = *src;
	if (strchr(binops,*src) != NULL) { type = T_BOP; return; }
	if (strchr(punc,*src) != NULL) { type = T_PNC; return;  }
	if (isdigit(*src)) { type = T_NUM; return; }
	if (*src == '\'' || *src == '\"') { type = T_STR; return; }
	if (isalpha(*src) || *src == '_') { type = T_VAR; return; }
}

void lexer_clear(void) {
	lexeme = realloc(lexeme, 0);
	lexeme_len = 0;
}

void lexer_append(char chr) {
	lexeme = realloc(lexeme, ++lexeme_len);
	lexeme[lexeme_len-1] = chr;
}

void lexer_scan(void) {
	lexer_peek();
	switch (type) {
		case T_EOF:
			break;
		case T_NUM:
			lexer_clear();
			while (type == T_NUM) {
				lexer_append(lookahead);
				src++;
				lexer_peek();
			}
			break;
		case T_STR:
			lexer_clear();
			char end = lookahead;
			src++;
			while (*src && *src != lookahead) {
				lexer_append(*src);
				src++;
			}
			src++;
			break;
		case T_VAR:
			lexer_clear();
			while (type == T_VAR) {
				lexer_append(lookahead);
				src++;
				lexer_peek();
			}
			if (strcmp(lexeme,"true") == 0 || strcmp(lexeme,"false") == 0) {
				type = T_BOL;
			}
			for (int i=0;i<sizeof(keywords)/sizeof(char*);i++) {
				if (strcmp(lexeme,keywords[i]) == 0) { type = T_KWD; }
			}
			break;
		case T_PNC:
			lexer_clear();
			lexer_append(lookahead);
			src++;
			break;
		case T_BOP:
			lexer_clear();
			lexer_append(lookahead);
			src++;
			switch (lookahead) {
				case '<':
				case '>':
				case '=':
				case '!':
					if (*src && *src == '=') { lexer_append(*src); src++; }
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	lexer_append('\0');
}

typedef enum {
	A_NUM,
	A_STR,
	A_BOL,
	A_VAR,
	A_IFS,
	A_IFE,
	A_BRK,
	A_ASN,
	A_BIN
} astnodetype;

typedef struct {
	astnodetype type;
} node;

typedef struct {
	astnodetype type;
	long value;
} numnode;

typedef struct {
	astnodetype type;
	char* value;
} strnode;

typedef struct {
	astnodetype type;
	int value;
} boolnode;

typedef struct {
	astnodetype type;
	char* name;
} varnode;

typedef struct {
	astnodetype type;
	node* cond;
	node* body;
} ifnode;

typedef struct {
	astnodetype type;
	node* cond;
	node* t_cl;
	node* f_cl;
} ifelsenode;

typedef struct {
	astnodetype type;
	node** body;
} bracketsnode;

typedef struct {
	astnodetype type;
	node* left;
	node* right;
	char* op;
} binarynode;

int prec(char* op) {
	if (strcmp(op, "=") == 0) { return 1; }
	if (strcmp(op, "|") == 0) { return 2; }
	if (strcmp(op, "&") == 0) { return 3; }
	if (strcmp(op, "<") == 0 ||
	    strcmp(op, ">") == 0 ||
	    strcmp(op,"<=") == 0 ||
	    strcmp(op,">=") == 0 ||
	    strcmp(op,"!=") == 0 ||
	    strcmp(op, "!") == 0 ||
	    strcmp(op,"==") == 0) { return 4; }
	if (strcmp(op, "+") == 0 ||
	    strcmp(op, "-") == 0) { return 5; }
	if (strcmp(op, "*") == 0 ||
	    strcmp(op, "/") == 0 ||
	    strcmp(op, "%") == 0) { return 6; }
	fprintf(stderr, "%sError: undefined binary operator: %s.\n%s", CRED, op, CEND);
	exit(-1);
}

node* atom(void) {

}

node* binary(node* left, int lastprec) {
	if (type == T_BOP) {
		if (prec(lexeme) > lastprec) {
			binarynode* bn;
			bn->type = (strcmp(lexeme,"=") == 0 ? A_ASN : A_BOP);
			bn->left = left;
			bn->right = binary(atom(),prec(lexeme));
			bn->op = lexeme;
			return binary((node*)(bn), lastprec);
		}
	}
	return left;
}

node* expr(void) {
	return binary(atom(),0);
}

void parse(void) {
	lexer_init();
	lexer_scan();
	while (type != T_EOF) { expr(); }
	lexer_free();
}

int main(int argc, char** argv) {
	if (argc < 2) { USAGE(argv[0]); }
	FILE* fp = fopen(argv[1], "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		src = malloc(length + 1);
		fread(src, 1, length, fp);
		fclose(fp);
		src[length] = 0;

		lexer_init();
		lexer_scan();

		while (type != T_EOF) {
			printf("Token: %s.\n", lexeme);
			lexer_scan();
		}

		lexer_free();

	} else { USAGE(argv[0]); }
	return 0;
}
