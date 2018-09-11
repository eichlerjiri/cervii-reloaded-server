#include "bqueue.h"

void bqueue_init(struct bqueue *q) {
	llist_init(&q->list);
	pthread_mutex_initx(&q->lock);
	pthread_cond_initx(&q->cond);
}

void bqueue_destroy(struct  bqueue *q) {
	llist_destroy(&q->list);
	pthread_mutex_destroyx(&q->lock);
	pthread_cond_destroyx(&q->cond);
}

void bqueue_add_tail(struct bqueue *q, void *data) {
	pthread_mutex_lockx(&q->lock);

	llist_add_tail(&q->list, data);

	pthread_cond_signalx(&q->cond);
	pthread_mutex_unlockx(&q->lock);
}

void* bqueue_rem_head(struct bqueue *q) {
	pthread_mutex_lockx(&q->lock);
	while (!q->list.head) {
		pthread_cond_waitx(&q->cond, &q->lock);
	}

	void *data = llist_rem_head(&q->list);
	pthread_mutex_unlockx(&q->lock);
	return data;
}
