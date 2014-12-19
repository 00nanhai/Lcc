#ifndef _CREATE_A_H_
#define _CREATE_A_H_

void append_to_Afile(string &data);
int is_complete_Afile();

void get_Afile_prefix_from_Bfile(char *Afile_prefix, 
								const char *Bfile);
int init_Afile();
#endif
