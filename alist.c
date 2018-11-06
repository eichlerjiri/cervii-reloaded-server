#include "alist.h"
#include "common.h"
#include "../websocket-server/common.h"

void alist_init(struct alist *list) {
	list->capacity = 8;
	list->size = 0;
	list->data = c_malloc(list->capacity * sizeof(void*));
}

void alist_destroy(struct alist *list) {
	c_free(list->data);
}

void alist_add(struct alist *list, void *ptr) {
	if (list->capacity <= list->size) {
		list->capacity *= 2;
		list->data = c_realloc(list->data, list->capacity * sizeof(void*));
	}
	list->data[list->size++] = ptr;
}

void alist_rem(struct alist *list, void *ptr) {
	void **data = list->data;
	for (size_t i = 0; i < list->size; i++) {
		if (data[i] == ptr) {
			list->size--;
			for (; i < list->size; i++) {
				data[i] = data[i + 1];
			}
			return;
		}
	}
}
