#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_PROCESS_NAME_LEN	256
#define BUFFER_SIZE (MAX_PROCESS_NAME_LEN + 32)
#define PROCESS_FILE_NAME_LEN	256
/*
 * 参数:
 * process_pid  : 检查进程的pid
 * process_name : 要检查程序的名字
 * 返回值：
 * 0		改程序没有重复运行
 * >0		程序重复运行,返回重复运行程序的pid
 * */
unsigned int is_process_repeat_run(unsigned int process_pid, char *process_name)
{
	assert((process_pid > 0) && (process_name != NULL));
    DIR *dir = opendir("/proc");
    if (!dir) {
		fprintf(stderr, "Cannot open /proc");
		assert(0);
    }

	FILE *fp;
    struct dirent *next;
	char buffer[BUFFER_SIZE] = {0};
	char process_file[PROCESS_FILE_NAME_LEN] = {0};
	unsigned int read_process_pid;
	char read_process_name[MAX_PROCESS_NAME_LEN] = {0};
    while ((next = readdir(dir)) != NULL) {
        /* Must skip ".." since that is outside /proc */
        if (strcmp(next->d_name, "..") == 0) /* FIXME 不用处理也没事 */
            continue;

        /* proc中进程状态文件,都是数字命名 */
        if (!isdigit(*next->d_name))
			continue;

        sprintf(process_file, "/proc/%s/status", next->d_name);
        if (!(fp = fopen(process_file, "r")))
            continue;
		
		memset(buffer, 0, BUFFER_SIZE);
        if (fgets(buffer, BUFFER_SIZE - 1, fp) == NULL) {
            fclose(fp);
            continue;
        }
        fclose(fp);

		/* /proc/\* 文件格式 Name:	binary_name */
		memset(read_process_name, 0, MAX_PROCESS_NAME_LEN);
        sscanf(buffer, "%*s %s", read_process_name);
		/* 程序名称是否相同 */
        if (strcmp(read_process_name, process_name) == 0) {
            read_process_pid = strtol(next->d_name, NULL, 0);
			/* 是否要判断进程本身 */
            if(read_process_pid != process_pid) {
				return read_process_pid;
            }
        }
    }
	return 0;
}
