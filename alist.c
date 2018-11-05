#include "alist.h"
#include "common.h"
#include "../websocket-server/common.h"

void alist_init_ptr(struct alist *list) {
	list->capacity = 8;
	list->length = 0;
	list->item_size = sizeof(void*);
	list->data = c_malloc(list->capacity * list->item_size);
}

void alist_destroy(struct alist *list) {
	c_free(list->data);
}

void alist_add_ptr(struct alist *list, void *ptr) {
	if (list->capacity <= list->length) {
		list->capacity *= 2;
		list->data = c_realloc(list->data, list->capacity * list->item_size);
	}
	void **data = list->data;
	data[list->length++] = ptr;
}

void alist_rem_ptr(struct alist *list, void *ptr) {
	void **data = list->data;
	for (int i = 0; i < list->length; i++) {
		if (data[i] == ptr) {
			list->length--;
			for (; i < list->length; i++) {
				data[i] = data[i + 1];
			}
			return;
		}
	}
}

void *alist_get_ptr(struct alist *list, unsigned int index) {
	void **data = list->data;
	return data[index];
}
