#ifndef _SW_FLOCK_H_
#define _SW_FLOCK_H_
int sw_ex_flock(int fd);
int sw_sh_flock(int fd);
int sw_un_flock(int fd);
int sw_nb_ex_flock(int fd);
int sw_nb_sh_flock(int fd);
int is_locked(int fd);
#endif
