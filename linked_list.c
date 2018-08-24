#include "linked_list.h"
#include <stdlib.h>

struct linked_list_item* fetch_item(struct linked_list *list, void *d) {
	return (struct linked_list_item*) ((char*) d + list->item_offset);
}

void ll_init(struct linked_list *list, int item_offset) {
	list->head = NULL;
	list->tail = NULL;
	list->item_offset = item_offset;
}

void ll_add_tail(struct linked_list *list, void *d) {
	struct linked_list_item *item = fetch_item(list, d);
	item->prev = NULL;
	item->next = NULL;

	if (list->tail) {
		item->prev = list->tail;
		fetch_item(list, list->tail)->next = d;
	} else {
		list->head = d;
	}
	list->tail = d;
}

void* ll_remove_head(struct linked_list *list) {
	if (!list->head) {
		return NULL;
	}

	struct linked_list_item *item = fetch_item(list, list->head);

	if (item->next) {
		fetch_item(list, item->next)->prev = NULL;
	} else {
		list->tail = NULL;
	}

	void *ret = list->head;
	list->head = item->next;
	return ret;
}
