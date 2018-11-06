#ifndef ALIST_H
#define ALIST_H

#include <stdlib.h>

struct alist {
	size_t capacity;
	size_t size;
	void **data;
};

void alist_init(struct alist *list);
void alist_destroy(struct alist *list);
void alist_add(struct alist *list, void *ptr);
void alist_rem(struct alist *list, void *ptr);

#endif
