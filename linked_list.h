#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct linked_list {
	void *head;
	void *tail;
	int item_offset;
};

struct linked_list_item {
	void *prev;
	void *next;
};

void ll_init(struct linked_list *list, int item_offset);
void ll_add_tail(struct linked_list *list, void *d);
void* ll_remove_head(struct linked_list *list);

#endif
