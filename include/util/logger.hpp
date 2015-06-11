
#pragma once

#include "../define.hpp"		/* __func__ */

enum QLOG_LEVEL
{
	QLOG_LEVEL_DEBUG,
	QLOG_LEVEL_INFO,
	QLOG_LEVEL_WARN,
	QLOG_LEVEL_ERROR,
	QLOG_LEVEL_FATAL
};

void print_log(QLOG_LEVEL level, const char* filename, int line, const char* fmt, ...);
void print_log_local(QLOG_LEVEL level, const char* filename, int line, const char* fmt, ...);

#define QLOG_DEBUG(...) print_log(QLOG_LEVEL_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_INFO(...) print_log(QLOG_LEVEL_INFO, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_WARN(...) print_log(QLOG_LEVEL_WARN, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_ERROR(...) print_log(QLOG_LEVEL_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_FATAL(...) print_log(QLOG_LEVEL_FATAL, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_DEBUG_LOCAL(...) print_log_local(QLOG_LEVEL_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_INFO_LOCAL(...) print_log_local(QLOG_LEVEL_INFO, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_WARN_LOCAL(...) print_log_local(QLOG_LEVEL_WARN, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_ERROR_LOCAL(...) print_log_local(QLOG_LEVEL_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)

#define QLOG_FATAL_LOCAL(...) print_log_local(QLOG_LEVEL_FATAL, __FILE__, __LINE__, ##__VA_ARGS__)