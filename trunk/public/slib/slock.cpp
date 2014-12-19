/**
 * author: Soli
 * date  : 2012-07-09
 * */
#include <stddef.h>
#include "slock.h"

namespace slib
{

SLock::SLock(SMutex* m)
{
	m_mutex = m;
	if(m_mutex != NULL)
	{
		m_mutex->lock();
	}
}

SLock::~SLock()
{
	if(m_mutex != NULL)
	{
		m_mutex->unlock();
	}
}

} // namespace slib

