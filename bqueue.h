#ifndef BQUEUE_H
#define BQUEUE_H

#include "common.h"

struct bqueue_item {
	struct bqueue_item *next;
};

struct bqueue {
	size_t item_size;
	struct bqueue_item *head;
	struct bqueue_item *tail;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

void bqueue_init(struct bqueue *q, size_t item_size);
void bqueue_destroy(struct bqueue *q);
void bqueue_add(struct bqueue *q, void *data);
void bqueue_rem(struct bqueue *q, void *data);

#endif
