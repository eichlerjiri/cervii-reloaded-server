#include "llist.h"
#include "../websocket-server/common.h"
#include <stdlib.h>

void llist_init(struct llist *list) {
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void llist_add_tail(struct llist *list, void *data) {
	struct llist_item *item = mallocx(sizeof(struct llist_item));
	item->next = NULL;
	item->data = data;
	list->size++;

	if (list->tail) {
		list->tail->next = item;
		list->tail = item;
	} else {
		list->head = item;
		list->tail = item;
	}
}

void* llist_rem_head(struct llist *list) {
	if (!list->head) {
		return NULL;
	}

	struct llist_item *item = list->head;
	if (item->next) {
		list->head = item->next;
	} else {
		list->head = NULL;
		list->tail = NULL;
	}

	void *data = item->data;
	freex(item);
	list->size--;
	return data;
}

void llist_rem_item(struct llist *list, void *data) {
	struct llist_item *prev = NULL;
	struct llist_item *cur = list->head;
	while (cur) {
		if (cur->data == data) {
			if (prev) {
				prev->next = cur->next;
			} else {
				list->head = cur->next;
			}
			if (!cur->next) {
				list->tail = prev;
			}

			freex(cur);
			list->size--;
			return;
		}

		prev = cur;
		cur = cur->next;
	}
}
