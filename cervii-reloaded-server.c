#include "common.c"
#include "alist.c"
#include "bqueue.c"

struct client_data {
	// connection tokens
	struct websocket_client *client;
	struct bqueue senderq;

	// client status
	char status;

	unsigned long long ping_start;
	unsigned long long ping;

	// game status
	unsigned char num;
	double x;
	double y;
	double angle;
	struct client_data *opponent;
	unsigned char *game;
};

struct queued_msg {
	struct client_data *cd;
	int type;
	char *text;
};

static void queue_add(struct bqueue *q, struct client_data *cd, int type, const char *text) {
	struct queued_msg *msg = c_malloc(sizeof(struct queued_msg));
	msg->cd = cd;
	msg->type = type;
	if (text) {
		msg->text = c_strdup(text);
	} else {
		msg->text = NULL;
	}
	bqueue_add(q, msg);
}

static void* sender_thread(void *ptr) {
	struct client_data *cd = ptr;
	while (1) {
		struct queued_msg *msg = bqueue_rem(&cd->senderq);
		if (msg->type == 3) {
			c_free(msg->text);
			c_free(msg);
			websocket_close(cd->client);
			bqueue_destroy(&cd->senderq);
			c_free(cd);
			return NULL;
		}
		websocket_send(cd->client->out, msg->text);
		c_free(msg->text);
		c_free(msg);
	}
}

static void connected(struct websocket_client *client) {
	struct client_data *cd = c_malloc(sizeof(struct client_data));
	cd->client = client;
	bqueue_init(&cd->senderq);
	cd->status = 'C';

	client->ex = cd;

	queue_add(client->ctx->ex, cd, 1, NULL);
	c_pthread_create(sender_thread, cd);
}

static void disconnected(struct websocket_client *client) {
	queue_add(client->ctx->ex, client->ex, 3, NULL);
}

static void received(struct websocket_client *client, const char *text) {
	queue_add(client->ctx->ex, client->ex, 2, text);
}

static void csend(struct client_data *cd, const char *text) {
	queue_add(&cd->senderq, cd, 2, text);
}

static void* listen_thread(void *ptr) {
	websocket_listen(1100, ptr);
	return NULL;
}

static void end_game(struct client_data *cd) {
	char end[16];
	sprintf(end, "E %i", cd->opponent->num);
	csend(cd, end);
	csend(cd->opponent, end);

	cd->status = 'C';
	cd->opponent->status = 'C';
	c_free(cd->game);
}

static void advance_player(struct client_data *cd) {
	double vsin = sin(cd->angle);
	double vcos = cos(cd->angle);

	int idx_draw[4 * 5 * 2 * 2];
	int pos = 0;

	for (int i = 0; i < 4; i++) {
		cd->x += vcos;
		cd->y += vsin;

		for (int j = -2; j <= 2; j++) {
			int mx = (int) round(cd->x + j * vsin);
			int my = (int) round(cd->y - j * vcos);
			for (int k = 0; k < 2; k++) {
				int wx = mx + k;
				for (int l = 0; l < 2; l++) {
					int wy = my + l;
					if (wx < 0 || wy < 0 || wx >= 800 || wy >= 800) {
						end_game(cd);
						return;
					}

					int idx = 800 * wx + wy;
					if (i == 3 && cd->game[idx]) {
						end_game(cd);
						return;
					}
					idx_draw[pos++] = idx;
				}
			}
		}
	}

	for (int i = 0; i < sizeof(idx_draw) / sizeof(int); i++) {
		cd->game[idx_draw[i]] = cd->num;
	}
}

// C = connected
// R = ready
// W = waiting
// P = ping
// A = ping ack
// S = game started
// E = game ended
// 0 = move straight
// 1 = move left
// 2 = move right
int main() {
	srand((unsigned int) time(NULL));

	struct bqueue q;
	bqueue_init(&q);

	struct websocket_context ctx = {&connected, &disconnected, &received, &q};
	c_pthread_create(listen_thread, &ctx);

	struct alist waiting;
	alist_init(&waiting);

	unsigned long long ping = 0;

	while (1) {
		struct queued_msg *msg = bqueue_rem(&q);

		struct client_data *cd = msg->cd;
		int type = msg->type;
		char *text = msg->text;

		//printf("RECEIVED: %s\n", text);

		if (type == 3) {
			if (cd->status == 'S') {
				end_game(cd);
			}
			alist_rem(&waiting, cd);
			queue_add(&cd->senderq, cd, 3, NULL); // stop sender
		} else if (type == 2) {
			if (cd->status == 'C' && !strcmp(text, "R")) {
				cd->status = 'W';
				alist_add(&waiting, cd);

				cd->ping_start = ping;
				cd->ping = ping;

				// ping to all waiting
				for (size_t i = 0; i < waiting.size; i++) {
					csend(waiting.data[i], "P");
				}
				ping++;
			} else if (cd->status == 'W' && !strcmp(text, "A")) {
				cd->ping++;

				for (size_t i = 0; i < waiting.size; i++) {
					struct client_data *curcd = waiting.data[i];
					if (curcd != cd && MIN(curcd->ping, cd->ping) >
					MAX(curcd->ping_start, cd->ping_start)) {
						struct client_data* players[] = {curcd, cd};
						players[0]->opponent = players[1];
						players[1]->opponent = players[0];

						size_t game_size = 800 * 800 * sizeof(char);
						unsigned char *game = c_malloc(game_size);
						memset(game, 0, game_size);

						char start[256] = "S  ";
						for (int i = 0; i < 2; i++) {
							int x = rand() % 800;
							int y = rand() % 800;
							double dangle = atan2(y - 400, x - 400);
							int angle = (int) round(dangle * 180 / M_PI) + 180;

							char startp[64];
							sprintf(startp, " %i %i %i", x, y, angle);
							strcat(start, startp);

							struct client_data *p = players[i];
							alist_rem(&waiting, p);

							p->status = 'S';
							p->num = (unsigned char) (i + 1);
							p->x = x;
							p->y = y;
							p->angle = angle * M_PI / 180;
							p->game = game;
						}
						for (int i = 0; i < 2; i++) {
							start[2] = (char) ('1' + i);
							csend(players[i], start);
						}

						break;
					}
				}
			} else if (cd->status == 'S' && (!strcmp(text, "0") ||
			!strcmp(text, "1") || !strcmp(text, "2"))) {
				if (text[0] == '1') {
					cd->angle -= 0.16;
				} else if (text[0] == '2') {
					cd->angle += 0.16;
				}

				char move[256];
				sprintf(move, "%s %i", text, cd->num);
				csend(cd->opponent, move);

				advance_player(cd);
			}
		}

		c_free(msg->text);
		c_free(msg);
	}
}
