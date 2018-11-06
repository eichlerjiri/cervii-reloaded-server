#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include "../websocket-server/websocket-server.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static void *c_realloc(void *ptr, size_t size) {
	void *ret = realloc(ptr, size);
	if (!ret) {
		fatal("Cannot realloc %li: %s", size, strerror(errno));
	}
#if ENABLE_TRACE
	if (ptr != ret) {
		if (ptr) {
			trace_end("MEM", ptr, "realloc");
		}
		trace_start("MEM", ret, "realloc");
	}
#endif
	return ret;
}

static void c_pthread_mutex_init(pthread_mutex_t *restrict mutex) {
	int ret = pthread_mutex_init(mutex, NULL);
	if (ret) {
		fatal("Cannot pthread_mutex_init: %s", strerror(ret));
	}
#if ENABLE_TRACE
	trace_start("MTX", mutex, "pthread_mutex_init");
#endif
}

static void c_pthread_mutex_destroy(pthread_mutex_t *mutex) {
#if ENABLE_TRACE
	trace_end("MTX", mutex, "pthread_mutex_destroy");
#endif
	int ret = pthread_mutex_destroy(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_destroy: %s", strerror(ret));
	}
}

static void c_pthread_mutex_lock(pthread_mutex_t *mutex) {
	int ret = pthread_mutex_lock(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_lock: %s", strerror(ret));
	}
}

static void c_pthread_mutex_unlock(pthread_mutex_t *mutex) {
	int ret = pthread_mutex_unlock(mutex);
	if (ret) {
		fatal("Cannot pthread_mutex_unlock: %s", strerror(ret));
	}
}

static void c_pthread_cond_init(pthread_cond_t *restrict cond) {
	int ret = pthread_cond_init(cond, NULL);
	if (ret) {
		fatal("Cannot pthread_cond_init: %s", strerror(ret));
	}
#if ENABLE_TRACE
	trace_start("CND", cond, "pthread_cond_init");
#endif
}

static void c_pthread_cond_destroy(pthread_cond_t *cond) {
#if ENABLE_TRACE
	trace_end("CND", cond, "pthread_cond_destroy");
#endif
	int ret = pthread_cond_destroy(cond);
	if (ret) {
		fatal("Cannot pthread_cond_destroy: %s", strerror(ret));
	}
}

static void c_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock) {
	int ret = pthread_cond_wait(cond, lock);
	if (ret) {
		fatal("Cannot pthread_cond_wait: %s", strerror(ret));
	}
}

static void c_pthread_cond_signal(pthread_cond_t *cond) {
	int ret = pthread_cond_signal(cond);
	if (ret) {
		fatal("Cannot pthread_cond_signal: %s", strerror(ret));
	}
}
