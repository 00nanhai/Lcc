#ifndef _SW_COND_MUTEX_H__
#define _SW_COND_MUTEX_H__

#include <time.h>
#include <pthread.h>

class sw_cond_mutex
{
public:
	sw_cond_mutex();
	virtual ~sw_cond_mutex();

	void lock();
	int lock(time_t timeout);
	int lock_and_wait();
	bool lock_and_wait(time_t timeout);
	void send_signal();
	void send_signal_and_unlock();
	bool trylock();
	void unlock();

private:	// no copyable
	sw_cond_mutex(const sw_cond_mutex&);
	sw_cond_mutex& operator=(const sw_cond_mutex&);

private:
	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
};

#endif	// #ifndef __SLIB_SMUTEX_H__
