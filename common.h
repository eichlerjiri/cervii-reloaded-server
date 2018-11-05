#ifndef COMMON2_H
#define COMMON2_H

#include <pthread.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void *c_realloc(void *ptr, size_t size);
void c_pthread_mutex_init(pthread_mutex_t *restrict mutex);
void c_pthread_mutex_destroy(pthread_mutex_t *mutex);
void c_pthread_mutex_lock(pthread_mutex_t *mutex);
void c_pthread_mutex_unlock(pthread_mutex_t *mutex);
void c_pthread_cond_init(pthread_cond_t *restrict cond);
void c_pthread_cond_destroy(pthread_cond_t *cond);
void c_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *lock);
void c_pthread_cond_signal(pthread_cond_t *cond);

#endif
