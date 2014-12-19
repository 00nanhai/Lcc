/**
 * author: Soli
 * date  : 2012-07-09
 * */
#include <errno.h>
#include <pthread.h>
#include "smutex.h"

namespace slib
{

SMutex::SMutex()
{
	m_mutex = (void*) new pthread_mutex_t;
	pthread_mutex_init((pthread_mutex_t*)m_mutex, NULL);
}

SMutex::~SMutex()
{
	pthread_mutex_destroy((pthread_mutex_t*)m_mutex);
	delete( (pthread_mutex_t*)m_mutex );
	m_mutex = NULL;
}

void SMutex::lock()
{
	while(1)
	{
		int ret = pthread_mutex_lock((pthread_mutex_t*)m_mutex);
		if(ret == 0 || ret == EDEADLK)
		{
			return;
		}
	}
}

#ifdef __USE_XOPEN2K
bool SMutex::lock(time_t timeout)
{
	if(timeout == 0)
	{
		lock();
		return true;
	}

	struct timespec ts;
	ts.tv_sec = timeout/1000;
	timeout = timeout - ts.tv_sec*1000;
    ts.tv_nsec = timeout*1000000;

    if(0 == pthread_mutex_timedlock((pthread_mutex_t*)m_mutex, &ts))
    {
    	return true;
    }

    return false;
}
#endif

bool SMutex::trylock()
{
	if(0 == pthread_mutex_trylock((pthread_mutex_t*)m_mutex))
	{
		return true;
	}

	return false;
}

void SMutex::unlock()
{
	pthread_mutex_unlock((pthread_mutex_t*)m_mutex);
}

} // namespace slib
