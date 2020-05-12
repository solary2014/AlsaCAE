#include "android_cpu.h"

#define APR_USE_THREAD_SCHED 1

#include <pthread.h>
#include <errno.h>
#if APR_USE_THREAD_SCHED
#include <sched.h>
#endif 
#include <android/log.h>

#define ALOGE(...) 	__android_log_print(ANDROID_LOG_ERROR, "android_cpu", __VA_ARGS__)
#define ALOGI(...) 	__android_log_print(ANDROID_LOG_INFO, "android_cpu",  __VA_ARGS__)
#define ALOGD(...) 	__android_log_print(ANDROID_LOG_DEBUG, "android_cpu", __VA_ARGS__)

#define TD_NUM 3
pthread_t tds[TD_NUM];
int running = 1;

static void *dummy_worker(void *opaque)
{
	long tid = tid = pthread_self();
	
	ALOGI("dummy_worker begin %5lu", tid);
	while(running) {
        ALOGI("thread id: %5lu", tid);
	}
	ALOGI("dummy_worker begin %5lu", tid);
	return NULL;
}


int android_cpu_create()
{
	int rv;
	pthread_attr_t attr;
	int i;
	rv = pthread_attr_init(&attr);
	if (rv) {
		ALOGE("pthread_attr_init error, rv = %d", rv);
		return rv;
	}
	rv = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rv) {
		ALOGE("pthread_attr_setdetachstate error, rv = %d", rv);
		pthread_attr_destroy(&attr);
		return rv;
	}
#if APR_USE_THREAD_SCHED
	{
		struct sched_param sched_p;
		int policy = 0;

		pthread_attr_getschedpolicy(&attr, &policy);
		ALOGI("thread sched policy 0 = %d\n", policy);
		//policy =   SCHED_FIFO; 
		policy = SCHED_RR;
		pthread_attr_setschedpolicy(&attr, policy);
		pthread_attr_getschedpolicy(&attr, &policy);
		ALOGI("thread sched policy 1 = %d\n", policy);

		pthread_attr_getschedparam(&attr, &sched_p);
		ALOGI("thread sched priority 0 = %d\n", sched_p.sched_priority);
		sched_p.sched_priority = sched_get_priority_max(policy);
		pthread_attr_setschedparam(&attr, &sched_p);
		pthread_attr_getschedparam(&attr, &sched_p);
		ALOGI("thread sched priority 1 = %d\n", sched_p.sched_priority);
	}
#endif 
	for ( i = 0; i < TD_NUM; ++i) {
		rv = pthread_create(&tds[i], &attr, dummy_worker, NULL);
		if (rv) {
			ALOGE("pthread_create error, rv = %d", rv);
			return rv;
		}
	}
	
	running = 1;
	
	pthread_attr_destroy(&attr);

	return 0;
}

int android_cpu_destory()
{
	int i;
	int *thread_stat;
	if (running == 0) {
		ALOGI("no running");
		return 0;
	}
	running = 0;
	for (i = 0; i < TD_NUM; ++i) {
		pthread_join(tds[i], (void *)&thread_stat);
	}
}