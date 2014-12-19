#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
//#include <bits/sigaction.h>

#include "sw_pthread_signal.h"
/*
 * 使用场景
 *	通过在主线程中阻塞一些信号，其它的线程会继承信号掩码，
 *	然后专门用一个线程使用sigwait函数来同步的处理信号，
 *	使其它的线程不受到信号的影响.
 * */
PthreadSignal::PthreadSignal(vector<int> &sig)
{
	sigmask = new sigset_t;
	assert(sigmask);
	sigemptyset(sigmask);

	vector<int>::iterator iter = sig.begin();
	//a.push_back(23);                                                                                           
	while(iter != sig.end()){
		if (sigaddset(sigmask, *iter) != 0) {
			perror("sigaddset");
			assert(0);
		}
		iter++;
	}

	if (pthread_sigmask(SIG_BLOCK, sigmask, NULL) != 0) {
		perror("pthread_sigmask");
		assert(0);
	}
	signo = sig;
}

PthreadSignal::PthreadSignal(int signo)
{
	sigmask = new sigset_t;
	assert(sigmask);
	sigemptyset(sigmask);
	
	if (sigaddset(sigmask, signo) != 0) {
		perror("sigaddset");
		assert(0);
	}

	if (pthread_sigmask(SIG_BLOCK, sigmask, NULL) != 0) {
		perror("pthread_sigmask");
		assert(0);
	}
}

PthreadSignal::~PthreadSignal()
{
	delete sigmask;
	sigmask = NULL;
}
 /*
  * 向某个线程发送信号
  * */
bool PthreadSignal::kill_signal(pthread_t thread_id, int signo)
{
	if (pthread_kill(thread_id, signo) != 0) {
		perror("pthread_kill");
		return -1;
	}
	return 0;
}

/*
 * 等待某个信号集的信号
 */
int PthreadSignal:: wait_signal()
{
	int sig;
	if (sigwait(sigmask, &sig) != 0) {
		return -1;
	}
	return sig;
}

