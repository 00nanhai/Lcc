/**
 * author: Soli
 * date  : 2012-07-09
 * */
#ifndef __SLIB_SMUTEX_H__
#define __SLIB_SMUTEX_H__

#include <time.h>

namespace slib
{

class SMutex
{
public:
	SMutex();
	virtual ~SMutex();

	void lock();
#ifdef __USE_XOPEN2K
	bool lock(time_t timeout);
#endif
	bool trylock();
	void unlock();

private:	// no copyable
	SMutex(const SMutex&);
	SMutex& operator=(const SMutex&);

private:
	void* m_mutex;
};

} // namespace slib

#endif	// #ifndef __SLIB_SMUTEX_H__
