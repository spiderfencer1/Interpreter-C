#include <stdio.h>
#include <stdlib.h>
#include "vec.h"

vec vnew(void) {
	vec v;
	v.data = malloc(0);
	v.len = 0;
	return v;
}

void vfree(vec* v) { free(v->data); }

void vpush(vec* v,void* e) {
	v->len++;
	v->data = realloc(v->data,sizeof(v->data) + sizeof(e));
	v->data[v->len-1] = e;
}

void* vget(vec* v,int pos) {
	if (pos >= 0 && pos < v->len) {
		return v->data[pos];
	}
	return NULL;
}
