#ifndef COMMON2_H
#define COMMON2_H

#include <pthread.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

char* strdupx(const char *s);
void pthread_mutex_initx(pthread_mutex_t *restrict mutex);
void pthread_mutex_destroyx(pthread_mutex_t *mutex);
void pthread_mutex_lockx(pthread_mutex_t *mutex);
void pthread_mutex_unlockx(pthread_mutex_t *mutex);
void pthread_cond_initx(pthread_cond_t *restrict cond);
void pthread_cond_destroyx(pthread_cond_t *cond);
void pthread_cond_waitx(pthread_cond_t *cond, pthread_mutex_t *lock);
void pthread_cond_signalx(pthread_cond_t *cond);

#endif
