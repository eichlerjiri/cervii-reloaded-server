#include "common.h"
#include "../websocket-server/common.h"
#include "../websocket-server/trace.h"
#include <string.h>
#include <errno.h>

char* strdupx(const char *s) {
	char *ret = strdup(s);
	if (!ret) {
		fatal("Cannot strdup: %s", strerror(errno));
	}
#ifdef TRACE_H
	trace_start("MEM", ret, "strdup");
#endif
	return ret;
}

void pthread_mutex_initx(pthread_mutex_t *restrict mutex) {
	int ret = pthread_mutex_init(mutex, NULL);
	if (ret) {
		fatal("Cannot pthread_mutex_init: %s", strerror(ret));
	}
#ifdef TRACE_H
	trace_start("MTX", mutex, "pthread_mutex_init");
#endif
}

void pthread_mutex_destroyx(pthread_mutex_t *mutex) {
#ifdef TRACE_H
	trace_end("MTX", mutex, "pthread_mutex_destroy");
#endif
	int ret = pthread_mutex_destroy(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_destroy: %s", strerror(ret));
	}
}

void pthread_mutex_lockx(pthread_mutex_t *mutex) {
	int ret = pthread_mutex_lock(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_lock: %s", strerror(ret));
	}
}

void pthread_mutex_unlockx(pthread_mutex_t *mutex) {
	int ret = pthread_mutex_unlock(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_unlock: %s", strerror(ret));
	}
}

void pthread_cond_initx(pthread_cond_t *restrict cond) {
	int ret = pthread_cond_init(cond, NULL);
	if (ret) {
		fatal("Cannot pthread_cond_init: %s", strerror(ret));
	}
#ifdef TRACE_H
	trace_start("CND", cond, "pthread_cond_init");
#endif
}

void pthread_cond_destroyx(pthread_cond_t *cond) {
#ifdef TRACE_H
	trace_end("CND", cond, "pthread_cond_destroy");
#endif
	int ret = pthread_cond_destroy(cond);
	if (ret) {
		fatal("Cannot pthread_cond_destroy: %s", strerror(ret));
	}
}

void pthread_cond_waitx(pthread_cond_t *cond, pthread_mutex_t *lock) {
	int ret = pthread_cond_wait(cond, lock);
	if (ret) {
		fatal("Cannot pthread_cond_wait: %s", strerror(ret));
	}
}

void pthread_cond_signalx(pthread_cond_t *cond) {
	int ret = pthread_cond_signal(cond);
	if (ret) {
		fatal("Cannot pthread_cond_signal: %s", strerror(ret));
	}
}
