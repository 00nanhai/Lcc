#include <errno.h>
#include <pthread.h>
#include "sw_cond_mutex.h"

sw_cond_mutex::sw_cond_mutex()
{
	mutex = new pthread_mutex_t;
	pthread_mutex_init(mutex, NULL);
	cond = new pthread_cond_t;
	pthread_cond_init(cond, NULL);
}

sw_cond_mutex::~sw_cond_mutex()
{
	pthread_mutex_destroy(mutex);
	delete mutex;
	pthread_cond_destroy(cond);
	delete cond;

	mutex = NULL;
	cond = NULL;
}

void sw_cond_mutex::lock()
{
	while(1) {
		int ret = pthread_mutex_lock(mutex);
		if(ret == 0 || ret == EDEADLK) {
			return;
		}
	}
}

int sw_cond_mutex::lock(time_t timeout)
{
	if (timeout == 0) {
		lock();
		return 0;
	}

	struct timespec ts;
	ts.tv_sec = timeout / 1000;
	timeout = timeout - ts.tv_sec * 1000;
    ts.tv_nsec = timeout * 1000000;
	/*此函数存在bug*/
    return pthread_mutex_timedlock(mutex, &ts);
}

bool sw_cond_mutex::trylock()
{
	if(0 == pthread_mutex_trylock(mutex)) {
		return true;
	}

	return false;
}

void sw_cond_mutex::unlock()
{
	pthread_mutex_unlock(mutex);
}

int sw_cond_mutex::lock_and_wait()
{
	pthread_mutex_lock(mutex);
	return pthread_cond_wait(cond, mutex);
}

bool sw_cond_mutex::lock_and_wait(time_t timeout)
{
	if(timeout == 0) {
		lock_and_wait();
		return true;
	}

	struct timespec ts;
	ts.tv_sec = timeout / 1000;
	timeout = timeout - ts.tv_sec*1000;
    ts.tv_nsec = timeout*1000000;

	pthread_mutex_lock(mutex);
	if (pthread_cond_timedwait(cond, mutex, &ts) == 0) {
		return true;
	}
	return false;
}

void sw_cond_mutex::send_signal()
{
	pthread_cond_signal(cond);
}

void sw_cond_mutex::send_signal_and_unlock()
{
	pthread_cond_signal(cond);
	pthread_mutex_unlock(mutex);
}

