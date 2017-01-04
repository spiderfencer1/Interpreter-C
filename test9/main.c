#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define USAGE(progname) fprintf(stderr, "%sUsage: %s <filename>%s\n", "\x1b[32m", progname, "\x1b[0m"); exit(-1)

char* newstr(int len) {
	char* str = malloc(len);
	return str;
}

char* makestr(char c) {
	char* str = malloc(1);
	str[0] = c;
	return str;
}

void addchr(char* str, char c) {
	int index = (int)(strlen(str));
	str = realloc(str, index + 1);
	str[index] = c;
}

void addstr(char* a, char* b) {
	a = realloc(a, strlen(a) + strlen(b));
	strcat(a,b);
}

typedef struct {
	char* str;
	int index;
} striter;

striter newiter(char* str) {
	striter si = { str, 0 };
	return si;
}

int eos(striter si) {
	return si.index >= strlen(si.str);
}

void next(striter* si) {
	si->index++;
}

char yield(striter si) {
	return si.str[si.index];
}

typedef struct {
	void** data;
	int len;
} vec;

vec vnew(void) {
	vec v;
	v.data = malloc(0);
	v.len = 0;
	return v;
}

void vadd(vec* v, void* elem) {
	v->len++;
	v->data = realloc(v->data, sizeof(void*) * v->len + sizeof(elem));
	v->data[v->len-1] = elem;
}

void* vget(vec* v, int i) {
	if (i >= 0 && i < v->len) { return v->data[i]; }
	return NULL;
}

enum tokentype {
	T_NUM,
	T_STR,
	T_BOL,
	T_VAR,
	T_PNC,
	T_BOP
};

typedef struct {
	enum tokentype type;
	char* lexeme;
} token;

token* maketok(enum tokentype type, char* lexeme) {
	token* t = malloc(sizeof(token*));
	t->type = type;
	t->lexeme = malloc(strlen(lexeme));
	strcpy(t->lexeme, lexeme);
	return t;
}

vec lex(striter buf) {
	char* punc = "(){};,";

	char look;
	vec tokens = vnew();
	while (1) {
		while (!eos(buf) && isspace(yield(buf))) { next(&buf); }
		if (eos(buf)) { break; }
		look = yield(buf);
		next(&buf);
		if (isdigit(look)) {
			char* num = makestr(look);
			while (!eos(buf) && isdigit(yield(buf))) {
				addchr(num, yield(buf));
				next(&buf);
			}
			token* t = maketok(T_NUM, num);
			vadd(&tokens, t);
		} else if (look == '\'' || look == '\"') {
			char* str = newstr(0);
			while (!eos(buf) && yield(buf) != look) {
				addchr(str,yield(buf));
				next(&buf);
			}
			next(&buf);
			token* t = maketok(T_STR, str);
			vadd(&tokens, t);
		} else if (isalpha(look) || look == '_') {
			char* var = makestr(look);
			while (!eos(buf) && (isalnum(yield(buf)) || yield(buf) == '_')) {
				addchr(var,yield(buf));
				next(&buf);
			}
			token* t = maketok(T_VAR, var);
			if (strcmp(var,"true") == 0 || strcmp(var,"false") == 0) { t->type = T_BOL; }
			vadd(&tokens, t);
		} else if (strchr(punc,look) != NULL) {
			char* p = makestr(look);
			token* t = maketok(T_PNC, p);
			vadd(&tokens, t);
		} else {
			switch (look) {
				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
				case '|':
				case '&':
					{
						char* op = makestr(look);
						token* t = maketok(T_BOP, op);
						vadd(&tokens, t);
					}
					break;
				case '=':
				case '<':
				case '>':
				case '!':
					{
						char* op = makestr(look);
						if (!eos(buf) && yield(buf) == '=') {
							addchr(op,yield(buf));
							next(&buf);
						}
						token* t = maketok(T_BOP, op);
						vadd(&tokens, t);
					}
					break;
				default:
					break;
			}
		}
	}
	return tokens;
}

enum asttype {
	A_NUM,
	A_STR,
	A_BOL,
	A_VAR,
	A_BLK,
	A_IFS,
	A_ASN,
	A_BIN
};

typedef struct astnode {
	enum asttype type;
	union {
		long numval;
		char* strval;
		int boolval;
		char* name;
		vec blockval;
		struct ifstruct {
			struct astnode* cond;
			struct astnode* body;
		}* ifval;
		struct binstruct {
			struct astnode* left;
			struct astnode* right;
			char* op;
		}* binval;
	};
} astnode;

typedef struct {
	vec v;
	int pos;
} veciter;

veciter* vinew(vec v) {
	veciter* vi;
	vi->v = vnew();
	for (int i=0;i<v.len;i++) {
		vadd(&(vi->v), vget(&v,i));
	}
	vi->pos = 0;
	return vi;
}

void viadd(veciter* vi, void* v) {
	vadd(&(vi->v), v);
}

void* viget(veciter* vi, int i) {
	return vget(&(vi->v), vi->pos+i);
}

int viend(veciter* vi) {
	return viget(vi, 0) == NULL;
}

astnode* parseAtom(veciter*);
astnode* parseBinary(veciter*,astnode*,int);

int prec(char* lexeme) {
	return 1;
}

astnode* parseIf(veciter* tokiter) {
	astnode* cond = parseBinary(tokiter, parseAtom(tokiter), 0);
	astnode* body = parseBinary(tokiter, parseAtom(tokiter), 0);
	astnode* ifs;
	ifs->type = A_IFS;
	ifs->ifval = (struct ifstruct*)(malloc(sizeof(struct ifstruct)));
	ifs->ifval->cond = cond;
	ifs->ifval->body = body;
	return ifs;
}

astnode* parseAtom(veciter* tokiter) {
	token* t = (token*)(viget(tokiter,0));
	tokiter->pos++;
	switch (t->type) {
		case T_NUM:
			{
				astnode* n;
				n->type = A_NUM;
				n->numval = atol(t->lexeme);
				return n;
			}
		case T_STR:
			{
				astnode* n;
				n->type = A_STR;
				n->strval = malloc(strlen(t->lexeme));
				strcpy(n->strval,t->lexeme);
				return n;
			}
		case T_BOL:
			{
				astnode* n;
				n->type = A_BOL;
				n->boolval = (strcmp(t->lexeme,"true") == 0);
				return n;
			}
		case T_VAR:
			{
				if (strcmp(t->lexeme,"if") == 0) { return parseIf(tokiter); }
				astnode* n;
				n->type = A_VAR;
				n->name = malloc(strlen(t->lexeme));
				strcpy(n->name, t->lexeme);
				return n;
				break;
			}
		case T_PNC:
			{
				break;
			}
		default:
			break;
	}
	fprintf(stderr, "%sError: cannot parse lexeme: %s.%s\n", "\x1b[31m", t->lexeme, "\x1b[0m");
	exit(-1);
}

astnode* parseBinary(veciter* tokiter, astnode* left, int lastPrec) {
	token* t = (token*)(viget(tokiter, 0));
	if (t->type = T_BOP) {
		if (prec(t->lexeme) > lastPrec) {
			tokiter->pos++;
			astnode* n;
			n->type = (strcmp(t->lexeme,"=") == 0 ? A_ASN : A_BIN);
			n->binval = malloc(sizeof(struct binstruct));
			n->binval->left = left;
			n->binval->right = parseBinary(tokiter, parseAtom(tokiter), prec(t->lexeme));
			n->binval->op = malloc(strlen(t->lexeme));
			strcpy(n->binval->op, t->lexeme);
			return parseBinary(tokiter, n, lastPrec);
		}
	}
	return left;
}

void parse(vec tokens) {
	int pos = 0;
	vec ast = vnew();
	veciter* tokiter = vinew(tokens);
	while (!viend(tokiter)) {
		viadd(tokiter, parseBinary(tokiter, parseAtom(tokiter), 0));
	}
}

int main(int argc, char** argv) {
	if (argc < 2) { USAGE(argv[0]); }
	FILE* fp = fopen(argv[1], "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		char* buffer = newstr(length + 1);
		fread(buffer, 1, length, fp);
		buffer[length] = 0;
		vec tokens = lex(newiter(buffer));
		parse(tokens);
	} else { USAGE(argv[0]); }
	return 0;
}
