#ifndef SRC_STR_H
#define SRC_STR_H

typedef struct {
	char* text;
	int len;
} str;

str snew(char*);
void sfree(str*);
void sadd(str*,char);
void scat(str*,str*);
char sat(str*,int);
char* sget(str*);
int scmp(str*,char*);
int sncmp(str*,char*,int);

#endif
