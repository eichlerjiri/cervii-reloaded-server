#include "bqueue.h"
#include "../websocket-server/common.h"
#include <string.h>

void bqueue_init(struct bqueue *q) {
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
	struct bqueue_item *item = c_malloc(sizeof(struct bqueue_item));
	item->next = NULL;
	item->data = data;

	c_pthread_mutex_lock(&q->lock);

	if (q->tail) {
		q->tail->next = item;
	} else {
		q->head = item;
	}
	q->tail = item;

	c_pthread_cond_signal(&q->cond);
	c_pthread_mutex_unlock(&q->lock);
}

void *bqueue_rem(struct bqueue *q) {
	c_pthread_mutex_lock(&q->lock);

	struct bqueue_item *item;
	while (!(item = q->head)) {
		c_pthread_cond_wait(&q->cond, &q->lock);
	}

	q->head = item->next;
	if (!item->next) {
		q->tail = NULL;
	}

	c_pthread_mutex_unlock(&q->lock);

	void *ret = item->data;
	c_free(item);
	return ret;
}
