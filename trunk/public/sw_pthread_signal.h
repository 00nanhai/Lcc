#ifndef _SW_PTHREAD_SIGNAL_H_
#define _SW_PTHREAD_SIGNAL_H_

#include <signal.h>
#include <pthread.h>
#include <vector>

using namespace std;

class PthreadSignal
{
public:
	PthreadSignal(vector<int> &sig);
	PthreadSignal(int signo);
	virtual ~PthreadSignal();
	bool kill_signal(pthread_t thread_id, int signo);
	int wait_signal();

private:
	sigset_t *sigmask;
	vector<int> signo;

private:	// no copyable
	PthreadSignal(const PthreadSignal&);
	PthreadSignal& operator=(const PthreadSignal&);
};

#endif
