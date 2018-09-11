#include "bqueue.h"
#include "../websocket-server/common.h"
#include "../websocket-server/server.h"
#include "../websocket-server/websocket.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

struct client_data {
	// connection tokens
	struct websocket_client *client;
	struct bqueue senderq;

	// client status
	char status;

	unsigned long long ping_start;
	unsigned long long ping;

	// game status
	int num;
	struct client_data *opponent;
};

struct queued_msg {
	struct client_data *cd;
	int type;
	char *text;
};

struct queued_msg* qitem_prep(struct client_data *cd, int type, char *text) {
	struct queued_msg *msg = callocx(1, sizeof(struct queued_msg));
	msg->cd = cd;
	msg->type = type;
	if (text) {
		msg->text = strdupx(text);
	}
	return msg;
}

void qitem_destroy(struct queued_msg *msg) {
	if (msg->text) {
		freex(msg->text);
	}
	freex(msg);
}

void* sender_thread(void *ptr) {
	struct client_data *cd = ptr;
	while (1) {
		struct queued_msg* msg = bqueue_rem_head(&cd->senderq);
		if (msg->type == 3) {
			qitem_destroy(msg);
			websocket_close(cd->client);
			bqueue_destroy(&cd->senderq);
			freex(cd);
			return NULL;
		}
		websocket_send(cd->client->out, msg->text);
		qitem_destroy(msg);
	}
}

void connected(struct websocket_client *client) {
	struct client_data *cd = callocx(1, sizeof(struct client_data));
	cd->client = client;
	bqueue_init(&cd->senderq);

	client->ex = cd;

	bqueue_add_tail(client->ctx->ex, qitem_prep(cd, 1, NULL));
	pthread_createx(sender_thread, cd);
}

void disconnected(struct websocket_client *client) {
	bqueue_add_tail(client->ctx->ex, qitem_prep(client->ex, 3, NULL));
}

void received(struct websocket_client *client, char *text) {
	bqueue_add_tail(client->ctx->ex, qitem_prep(client->ex, 2, text));
}

void send(struct client_data *cd, char *text) {
	bqueue_add_tail(&cd->senderq, qitem_prep(cd, 2, text));
}

void* listen_thread(void *ptr) {
	websocket_listen(1100, ptr);
	return NULL;
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

	struct bqueue q;
	bqueue_init(&q);

	struct websocket_context ctx = {&connected, &disconnected, &received, &q};
	pthread_createx(listen_thread, &ctx);

	struct llist waiting;
	llist_init(&waiting);

	unsigned long long ping = 0;

	//struct client_data *waiting = NULL;
	while (1) {
		struct queued_msg *msg = bqueue_rem_head(&q);

		struct client_data *cd = msg->cd;
		int type = msg->type;
		char *text = msg->text;

		//printf("RECEIVED: %s\n", text);

		if (type == 3) {
			if (cd->opponent) {
				cd->opponent->opponent = NULL;
			}
			llist_rem_item(&waiting, cd);
			bqueue_add_tail(&cd->senderq, qitem_prep(cd, 3, NULL)); // stop sender
		} else if (type == 2) {
			if (!cd->status && !strcmp(text, "R")) {
				cd->status = 'W';
				llist_add_tail(&waiting, cd);

				cd->ping_start = ping;
				cd->ping = ping;

				// ping to all waiting
				struct llist_item *cur = waiting.head;
				while (cur) {
					send(cur->data, "P");
					cur = cur->next;
				}
				ping++;
			} else if (cd->status == 'W' && !strcmp(text, "A")) {
				cd->ping++;

				struct llist_item *cur = waiting.head;
				while (cur) {
					struct client_data *curcd = cur->data;
					if (curcd != cd && MIN(curcd->ping, cd->ping) >
					MAX(curcd->ping_start, cd->ping_start)) {
						llist_rem_item(&waiting, curcd);
						llist_rem_item(&waiting, cd);

						curcd->status = 'S';
						cd->status = 'S';

						curcd->num = 1;
						cd->num = 2;

						curcd->opponent = cd;
						cd->opponent = curcd;

						char start[256] = "S  ";
						for (int i=0; i<2; i++) {
							int x = rand() % 800;
							int y = rand() % 600;
							int angle = rand() % 360;

							char startp[64];
							sprintf(startp, " %i %i %i", x, y, angle);
							strcat(start, startp);
						}
						start[2] = '1';
						send(curcd, start);
						start[2] = '2';
						send(cd, start);

						break;
					}
					cur = cur->next;
				}
			} else if (cd->status == 'S' && (!strcmp(text, "0") ||
			!strcmp(text, "1") || !strcmp(text, "2"))) {
				if (cd->opponent) {
					char move[256];
					sprintf(move, "M %i %s", cd->num, text);
					send(cd->opponent, move);
				}
			}
		}

		qitem_destroy(msg);
	}
}
