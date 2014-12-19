#ifndef _CREATE_B_H_
#define _CREATE_B_H_

#include <string>
bool get_cur_month_newest_Bfile(string &Bname, char *Bfile_path);

void send_create_Bfile_signal();

void set_globle_Bfile(char *Bname, int Bindex);
void read_globle_Bfile(string &Bname, int &Bindex);
void *create_Bfile_thread(void *arg);
#endif
