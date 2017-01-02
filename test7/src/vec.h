#ifndef SRC_VEC_H
#define SRC_VEC_H

typedef struct {
	void** data;
	int len;
} vec;

vec vnew(void);
void vfree(vec*);
void vpush(vec*,void*);
void* vget(vec*,int);

#endif
