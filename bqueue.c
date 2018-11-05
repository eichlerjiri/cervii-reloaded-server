#include "bqueue.h"
#include "../websocket-server/common.h"
#include <string.h>

void bqueue_init(struct bqueue *q, size_t item_size) {
	q->item_size = item_size;
	q->head = NULL;
	q->tail = NULL;
	c_pthread_mutex_init(&q->lock);
	c_pthread_cond_init(&q->cond);
}

void bqueue_destroy(struct bqueue *q) {
	c_pthread_mutex_destroy(&q->lock);
	c_pthread_cond_destroy(&q->cond);
}

void bqueue_add(struct bqueue *q, void *data) {
	c_pthread_mutex_lock(&q->lock);

	struct bqueue_item *item = c_malloc(sizeof(struct bqueue_item) + q->item_size);
	item->next = NULL;
	memcpy(item + 1, data, q->item_size);

	if (q->tail) {
		q->tail->next = item;
	} else {
		q->head = item;
	}
	q->tail = item;

	c_pthread_cond_signal(&q->cond);
	c_pthread_mutex_unlock(&q->lock);
}

void bqueue_rem(struct bqueue *q, void *data) {
	c_pthread_mutex_lock(&q->lock);

	struct bqueue_item *item;
	while (!(item = q->head)) {
		c_pthread_cond_wait(&q->cond, &q->lock);
	}

	memcpy(data, item + 1, q->item_size);

	q->head = item->next;
	if (!item->next) {
		q->tail = NULL;
	}

	c_free(item);

	c_pthread_mutex_unlock(&q->lock);
}
