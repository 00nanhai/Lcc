#ifndef EEIWANT_MY_LOG_H
#define EEIWANT_MY_LOG_H

#ifdef __cplusplus
extern "C"{
#endif

#ifdef OLD_VERSION
#include <xLog.h>
#define LOG_HELPER(instance, TYPE, fmt, ...)  log4c_category_log(instance, LOG4C_PRIORITY_##TYPE, fmt, ##__VA_ARGS__)
xLog_category_t *INSTANCE;
#define LOG_INFO(fmt, data...) LOG_HELPER(INSTANCE, INFO, "%s() "fmt"\n", __FUN__, ##data)
#define LOG_DEBU(fmt, data...) LOG_HELPER(INSTANCE, DEBUG, "%s() "fmt"\n", __FUN__, ##data)
#define LOG_WARN(fmt, data...) LOG_HELPER(INSTANCE, WARN, "%s() "fmt"\n", __FUN__, ##data)
#define LOG_ERRO(fmt, data...) LOG_HELPER(INSTANCE, ERROR, "%s() "fmt"\n", __FUN__, ##data)

#else
#include <xLog.h>
#ifndef _SW_DEBUG_
#define LOG_HELPER(TYPE, fmt, ...) xLog_R(RUN_##TYPE, __FUNCTION__, "() "fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, data...) LOG_HELPER(INFO, fmt, ##data)
#define LOG_WARN(fmt, data...) LOG_HELPER(WARN, fmt, ##data)
#define LOG_ERRO(fmt, data...) LOG_HELPER(ERRO, fmt, ##data)
#define LOG_DEBU(fmt, data...) LOG_HELPER(DEBU, fmt, ##data)
#else
#define	LOG_HELPER(level, fmt, ...) printf("[%s] "fmt"    File:%s,Line:%d\n",level, ##__VA_ARGS__, __FILE__, __LINE__)
#define LOG_INFO(fmt, data...) LOG_HELPER("\033[;32mINFO\033[0m", fmt, ##data)
#define LOG_WARN(fmt, data...) LOG_HELPER("\033[;33mWARN\033[0m", fmt, ##data)
#define LOG_ERRO(fmt, data...) LOG_HELPER("\033[;31mERRO\033[0m", fmt, ##data)
#define LOG_DEBU(fmt, data...) LOG_HELPER("\033[;34mDEBU\033[0m", fmt, ##data)
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif
