#ifndef LLIST_H
#define LLIST_H

struct llist_item {
	struct llist_item *next;
	void *data;
};

struct llist {
	struct llist_item *head;
	struct llist_item *tail;
	unsigned int size;
};

void llist_init(struct llist *list);
void llist_add_tail(struct llist *list, void *data);
void* llist_rem_head(struct llist *list);
void llist_rem_item(struct llist *list, void *data);

#endif
