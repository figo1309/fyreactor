
#include <thread>
#include <chrono>
#include <stdarg.h>		/* va_start */
#include <iostream>		/* vsprintf */
#include <sstream>
#include <util/logger.hpp>


void print_log(QLOG_LEVEL level, const char* filename, int line, const char* fmt, ...)
{
	va_list ap;

	switch (level)
	{
	case QLOG_LEVEL_DEBUG:
		fprintf(stdout, "[DEBUG]");
		break;
	case QLOG_LEVEL_INFO:
		fprintf(stdout, "[INFO]");
		break;
	case QLOG_LEVEL_WARN:
		fprintf(stdout, "[WARNING]");
		break;
	case QLOG_LEVEL_ERROR:
		fprintf(stdout, "[ERROR]");
		break;
	default:
		fprintf(stdout, "[INFO]");
		break;
	}

	fprintf(stdout, "[%s:%d] ", filename, line);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
	fflush(stdout);
	va_end(ap);
}

void print_log_local(QLOG_LEVEL level, const char* filename, int line, const char* fmt, ...)
{
    std::stringstream ss;
    std::string logname;
    switch (level)
    {
    case QLOG_LEVEL_DEBUG:
        logname = "[DEBUG].log";
        break;
    case QLOG_LEVEL_INFO:
        logname = "[INFO].log";
        break;
    case QLOG_LEVEL_WARN:
        logname = "[WARNING].log";
        break;
    case QLOG_LEVEL_ERROR:
        logname = "[ERROR].log";
        break;
    default:
        logname = "[INFO].log";
        break;
    }

#ifdef _WIN32
    va_list ap;
    FILE *stream;
    errno_t err;
    if ((err = fopen_s(&stream, logname.c_str(), "a+")) != NULL) // C4996
        return;
	fprintf(stream, "[%s:%d] ", filename, line);
    va_start(ap, fmt);
    vfprintf(stream, fmt, ap);
    fprintf(stream, "\n");
    fflush(stream);
    fclose(stream);
    va_end(ap);
#else
	va_list ap;
	FILE *stream;

	stream = fopen(logname.c_str(), "a+");
	fprintf(stream, "[%s:%d] ", filename, line);
	va_start(ap, fmt);
	vfprintf(stream, fmt, ap);
	fprintf(stream, "\n");
	fflush(stream);
	fclose(stream);
	va_end(ap);
#endif
}