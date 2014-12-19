#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <unistd.h>

#include "common.h"

void joint_path_by_dir_and_name(char *path, 
								const char *dir, 
								const char *filename)
{
	/* 检查路径的最后是否包含'/'字符,没有处理'\\'转义字符情况 */
	Assert(path && dir && filename);
	const char *p = dir + strlen(dir);
	if (*(p - 1) == '/') {
		sprintf(path, "%s%s", dir, filename);
	} else {
		sprintf(path, "%s/%s", dir, filename);
	}
}

int is_file_execable(const char *path)
{
	return (access(path, X_OK) == 0);
}

int is_file_exist(const char *path)
{
	return (access(path, F_OK) == 0);
}

int is_file_readable(const char *path)
{
	return (access(path, R_OK) == 0);
}

int is_file_writeable(const char *path)
{
	return (access(path, W_OK) == 0);
}

int touch_afile(const char *path)
{
	/*FIXME 要创建的文件已经存在 */
	char cmd[512] = {0};
	if (is_file_exist(path)) {
		snprintf(cmd, 512, "rm -f %s", path);
		if (system(cmd) == -1) {
			Perror(cmd);
			return -1;
		}
		//return 0;
	}
	/*TODO　查看该路径是否存在,不存在创建该路径*/
	snprintf(cmd, 512, "touch %s", path);
	if (system(cmd) == -1) {
		Perror(cmd);
		return -1;
	}

	/* 修改文件权限 */
	snprintf(cmd, 512, "chmod 666 %s", path);
	if (system(cmd) == -1) {
		Perror(cmd);
		return -2;
	}
	return 0;
}

int rm_file(const char *dir, const char *file)
{
	char path[1024] = {0};
	joint_path_by_dir_and_name(path, dir, file);
	char rm_cmd[1024] = {0};
	snprintf(rm_cmd, 1024, "rm -f %s", path);
	if (system(rm_cmd) != -1) {
		return 0;
	} else {
		Perror("rm_cmd");
		return -1;
	}
}

int pack_file(const char *src_dir, 
				const char *src_file, 
				const char *dest_dir, 
				const char *dest_file)
{
	char src_path[1024] = {0};
	joint_path_by_dir_and_name(src_path, src_dir, src_file);
	if (!is_file_exist(src_path)) {
		printf("pack file %s isn't exist.\n", src_path);
		return -1;
	}
	char dest_path[1024] = {0};
	joint_path_by_dir_and_name(dest_path, dest_dir, dest_file);

	/*为去掉tar打包文件,在解压时包含多层目录.
	 *eg tar -zcvf /xx/xx.tar.gz /xx/xx.txt 解压之后./xx/xx.txt
	 *需要使用tar -zcvf /xx/xx.tar.gz xx.txt(不知道tar中有没有相应的选项)
	 */
	char cur_path[1024] = {0};
	getcwd(cur_path, 1024);
	chdir(src_dir);

	char cmd[1024] = {0};
	Assert(strlen(src_path) + strlen(dest_path) <= 1000);
	/*FIXME 这种方式不是很好. 防止在打包文件中,为完全生成的文件被访问.
		.x.temp的临时文件 */
	char temppath[1024] = {0};
	snprintf(temppath, 1024, "%s.tgz.xFenguang.temp", dest_path);
	snprintf(cmd, 1024, "tar -zcf %s %s", temppath, src_file);
	/* TODO 所有system命令均需要重定向错误信息 */
	int ret = system(cmd);
	if (ret == -1) {
		perror("tar");
	}
	
	char realpath[1024] = {0};
	snprintf(realpath, 1024, "%s.tgz", dest_path);
	/* 改为要要打包的文件名 */
	if ((ret = rename(temppath, realpath)) == -1) {
		perror("rename");
	}
	
	/*FIXME return path*/
	chdir(cur_path);
	return ret;
}


