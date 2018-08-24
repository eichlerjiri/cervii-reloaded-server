#include "../websocket-server/common.h"
#include "../websocket-server/server.h"
#include "../websocket-server/websocket.h"
#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

struct queue_item {
	struct linked_list_item list_item;
	char *msg;
	struct client_data *cd;
};

struct queue {
	struct linked_list list;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

struct client_data {
	// connection tokens
	FILE *client;
	struct queue sq;

	// client status
	char status;

	// game status
	int num;
	struct client_data *opponent;
};

void queue_init(struct queue *q) {
	ll_init(&q->list, offsetof(struct queue_item, list_item));
	pthread_mutex_init_x(&q->lock);
	pthread_cond_init_x(&q->cond);
}

void queue_add(struct queue *q, char *msg, struct client_data *cd) {
	int addAlloc = 0;
	if (msg) {
		addAlloc = strlen(msg) + 1;
	}
	struct queue_item *item = calloc_x(1, sizeof(struct queue_item) + addAlloc);
	item->cd = cd;

	if (msg) {
		item->msg = (char*) item + sizeof(struct queue_item);
		strcpy(item->msg, msg);
	}

	pthread_mutex_lock_x(&q->lock);

	ll_add_tail(&q->list, item);

	pthread_cond_signal_x(&q->cond);
	pthread_mutex_unlock_x(&q->lock);
}

struct queue_item *queue_rem(struct queue *q) {
	pthread_mutex_lock_x(&q->lock);
	while (!q->list.head) {
		pthread_cond_wait_x(&q->cond, &q->lock);
	}

	struct queue_item *ret = ll_remove_head(&q->list);
	pthread_mutex_unlock_x(&q->lock);
	return ret;
}

void* sender_thread(void *cda) {
	struct client_data *cd = cda;
	while (1) {
		struct queue_item *item = queue_rem(&cd->sq);
		if (!item->msg) {
			free(item);
			fclose(cd->client);
			free(cd);
			return NULL;
		}
		send_to_client(cd->client, 1, item->msg, strlen(item->msg));
		free(item);
	}
}

void *connected(void *userobj1, FILE *client) {
	struct client_data *cd = calloc_x(1, sizeof(struct client_data));
	cd->client = client;
	queue_init(&cd->sq);

	pthread_t thread;
	pthread_create_x(&thread, NULL, sender_thread, cd);
	return cd;
}

void disconnected(void *userobj1, void *userobj2) {
	queue_add(userobj1, NULL, userobj2);
}

void received(void *userobj1, void *userobj2, char *msg) {
	queue_add(userobj1, msg, userobj2);
}

void* start_server_thread(void *q) {
	struct websocket_callbacks callbacks = {&connected, &disconnected, &received};
	start_server(1100, callbacks, q);
	return NULL;
}

void send(char *msg, struct client_data *cd) {
	queue_add(&cd->sq, msg, cd);
}

// R = ready
// W = waiting
// S = started
// M = moving
// 0 = straight
// 1 = left
// 2 = right
int main() {
	srand(time(NULL));

	struct queue q = {0};
	queue_init(&q);

	pthread_t thread;
	pthread_create_x(&thread, NULL, start_server_thread, &q);

	struct client_data *waiting = NULL;
	while (1) {
		struct queue_item *item = queue_rem(&q);
		char *msg = item->msg;
		struct client_data *cd = item->cd;

		printf("RECEIVED: %s\n", msg);

		if (!msg) {
			if (cd->opponent) {
				cd->opponent->opponent = NULL;
			}
			send(NULL, cd);
		} else if (cd->status == '\0') {
			if (!strcmp(msg, "R")) {
				send("W", cd);
				cd->status = 'W';

				if (waiting) {
					char buffer[256] = "S  ";
					for (int i=0; i<2; i++) {
						int x = rand() % 800;
						int y = rand() % 600;
						int angle = rand() % 360;

						char cur[64];
						sprintf(cur, " %i %i %i", x, y, angle);
						strcat(buffer, cur);
					}

					buffer[2] = '1';
					send(buffer, waiting);
					buffer[2] = '2';
					send(buffer, cd);

					waiting->status = 'S';
					cd->status = 'S';

					waiting->num = 1;
					cd->num = 2;

					waiting = NULL;
				} else {
					waiting = cd;
				}
			}
		} else if (cd->status == 'S') {
			int straight = !strcmp(msg, "0");
			int left = !strcmp(msg, "1");
			int right = !strcmp(msg, "2");
			if (straight || left || right) {
				if (cd->opponent) {
					char buffer[256];
					sprintf(buffer, "M %i %s", cd->num, msg);
					send(buffer, cd->opponent);
				}
			}
		}

		free(item);
	}
}
