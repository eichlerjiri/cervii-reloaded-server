#ifndef ALIST_H
#define ALIST_H

#include <stdlib.h>

struct alist {
	unsigned int capacity;
	unsigned int length;
	size_t item_size;
	void *data;
};

void alist_init_ptr(struct alist *list);
void alist_destroy(struct alist *list);
void alist_add_ptr(struct alist *list, void *ptr);
void alist_rem_ptr(struct alist *list, void *ptr);
void *alist_get_ptr(struct alist *list, unsigned int index);

#endif
