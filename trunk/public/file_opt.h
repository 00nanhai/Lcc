#ifndef _FILE_OPT_H_
#define _FILE_OPT_H_

void joint_path_by_dir_and_name(char *path, const char *dir, const char *filename);
int touch_afile(const char *path);

int is_file_exist(const char *path);
int is_file_readable(const char *path);
int is_file_writeable(const char *path);
int is_file_execable(const char *path);
int rm_file(const char *dir, const char *file);
int pack_file(const char *src_dir, 
				const char *src_file, 
				const char *dest_dir, 
				const char *dest_file);

#endif
