#include "../websocket-server/common.h"
#include "../websocket-server/server.h"
#include "../websocket-server/websocket.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

struct queue_item {
	struct queue_item *next;
	char *msg;
	FILE *client;
	struct queue *sq;
};

struct queue {
	struct queue_item *head;
	struct queue_item *tail;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

void queue_init(struct queue *q) {
	pthread_mutex_init_x(&q->lock);
	pthread_cond_init_x(&q->cond);
}

void queue_add(struct queue *q, char *msg, FILE *client, struct queue *sq) {
	pthread_mutex_lock_x(&q->lock);

	int addAlloc = 0;
	if(msg != NULL) {
		addAlloc = strlen(msg) + 1;
	}
	struct queue_item *item = calloc_x(1, sizeof(struct queue_item) + addAlloc);
	item->client = client;
	item->sq = sq;

	if(msg != NULL) {
		item->msg = (char*) item + sizeof(struct queue_item);
		strcpy(item->msg, msg);
	}

	if (q->head == NULL) {
		q->head = item;
	} else {
		q->tail->next = item;
	}
	q->tail = item;

	pthread_cond_signal_x(&q->cond);
	pthread_mutex_unlock_x(&q->lock);
}

struct queue_item *queue_rem(struct queue *q) {
	pthread_mutex_lock_x(&q->lock);
	while (q->head == NULL) {
		pthread_cond_wait_x(&q->cond, &q->lock);
	}

	struct queue_item *ret = q->head;
	q->head = q->head->next;

	pthread_mutex_unlock_x(&q->lock);
	return ret;
}

void* sender_thread(void *qa) {
	struct queue *q = qa;
	while (1) {
		struct queue_item *item = queue_rem(q);
		if (!item->msg) {
			fclose(item->client);
			free(item);
			free(q);
			return NULL;
		}
		send_to_client(item->client, 1, item->msg, strlen(item->msg));
		free(item);
	}
}

void *connected(void *userobj1, FILE *client) {
	struct queue *sq = calloc_x(1, sizeof(struct queue));
	queue_init(sq);

	pthread_t thread;
	pthread_create_x(&thread, NULL, sender_thread, sq);
	return sq;
}

void disconnected(void *userobj1, FILE *client, void *userobj2) {
	queue_add(userobj2, NULL, client, userobj2);
}

void received(void *userobj1, FILE *client, void *userobj2, char *msg) {
	queue_add(userobj1, msg, client, userobj2);
}

void* start_server_thread(void *q) {
	struct websocket_callbacks callbacks = {&connected, &disconnected, &received};
	start_server(1100, callbacks, q);
	return NULL;
}

int main() {
	struct queue q = {0};
	queue_init(&q);

	pthread_t thread;
	pthread_create_x(&thread, NULL, start_server_thread, &q);

	while (1) {
		struct queue_item *item = queue_rem(&q);
		printf("RECEIVED: %s\n", item->msg);

		queue_add(item->sq, item->msg, item->client, item->sq);

		free(item);
	}
}
