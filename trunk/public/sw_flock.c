#include <stdio.h>
#include <errno.h>
#include <sys/file.h>

/*
 *flock 文件上锁注意事项
 *单一文件无法同时建立共享锁定和互斥锁定.
 *LOCK_SH:表示要创建一个共享锁，在任意时间内，
 *		  一个文件的共享锁可以被多个进程拥有
 *LOCK_EX：表示创建一个排他锁，在任意时间内，
 *	一个文件的排他锁只能被一个进程拥有
 **/
int sw_flock(int fd, int operation)
{
	if (flock(fd, operation) != 0) {
		perror("sw_flock");
		return -1;
	}
	return 0;
}

/* 阻塞的方式强制上锁 */
int sw_ex_flock(int fd)
{
	 return sw_flock(fd, LOCK_EX);
}

/* 阻塞的方式共享上锁 */
int sw_sh_flock(int fd)
{
	return sw_flock(fd, LOCK_SH);
}

/* 解锁 */
int sw_un_flock(int fd)
{
	return sw_flock(fd, LOCK_UN);
}

/* 以非阻塞的方式上锁 */
int sw_nb_ex_flock(int fd)
{
	 return sw_flock(fd, LOCK_EX | LOCK_NB);
}

/* 以非阻塞的方式上锁 */
int sw_nb_sh_flock(int fd)
{
	return sw_flock(fd, LOCK_SH | LOCK_NB);
}

/* 
 * 测试文件是否上锁 
 * 0  未上锁
 * 1  已经上锁
 * -1 出现错误
 * */
int is_locked(int fd) 
{
	if (sw_nb_ex_flock(fd) == 0) {
		sw_un_flock(fd);
		return 0;
	} 
	else if (errno == EWOULDBLOCK) {
		return 1;
	} else {
		return -1;
	}
}
