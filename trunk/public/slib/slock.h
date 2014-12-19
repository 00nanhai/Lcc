/**
 * author: Soli
 * date  : 2012-07-09
 * */
#ifndef __SLIB_SLOCK_H__
#define __SLIB_SLOCK_H__

#include "smutex.h"

namespace slib
{

class SLock
{
public:
	SLock(SMutex* m);
	virtual ~SLock();

private:	// no copyable
	SLock(const SLock&);
	SLock& operator=(const SLock&);

private:
	SMutex* m_mutex;
};

} // namespace slib

#endif	// #ifndef __SLIB_SLOCK_H__
