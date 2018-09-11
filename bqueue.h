#ifndef BQUEUE_H
#define BQUEUE_H

#include "llist.h"
#include "common.h"

struct bqueue {
	struct llist list;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

void bqueue_init(struct bqueue *q);
void bqueue_destroy(struct  bqueue *q);
void bqueue_add_tail(struct bqueue *q, void *data);
void* bqueue_rem_head(struct bqueue *q);

#endif
