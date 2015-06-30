#pragma once

#include <string.h>

#include <iostream>
#include <memory>
#include <functional>

#if defined(_WIN32) || defined(WIN32)
#define HAVE_WINDOWS 1
#include <process.h>	// for _getpid()
#endif

#if defined(__osf__)
// Tru64 lacks stdint.h, but has inttypes.h which defines a superset of
// what stdint.h would define.
#include <inttypes.h>
#elif !defined(_MSC_VER)
#include <stdint.h>
#endif

typedef unsigned int uint;
typedef unsigned char uchar;

#ifdef _MSC_VER
typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8  uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#endif

#ifdef __linux__
#define HAVE_LINUX 1
#endif

#ifdef __FreeBSD__
#define HAVE_FREEBSD 1
#endif

#ifdef __FreeBSD_kernel__
#define HAVE_KFREEBSD 1
#endif

#ifdef __NetBSD__
#define HAVE_NETBSD 1
#endif

#ifdef __OpenBSD__
#define HAVE_OPENBSD 1
#endif

#ifdef __APPLE__
#define HAVE_DARWIN 1
#endif

#ifdef ANDROID
#define HAVE_ANDROID 1
#endif

#if !defined(HAVE_WINDOWS) \
	&& !defined(HAVE_LINUX) \
	&& !defined(HAVE_FREEBSD) \
	&& !defined(HAVE_KFREEBSD) \
	&& !defined(HAVE_NETBSD) \
	&& !defined(HAVE_OPENBSD) \
	&& !defined(HAVE_DARWIN) \
	&& !defined(HAVE_ANDROID)
#error Operating system does not support.
#endif

/* 统一定义基本的数据类型 */
typedef unsigned int		uint32_t;
typedef unsigned long		ulong_t;
typedef long				long_t;

namespace fyreactor
{
	class nocopyable
	{
	protected:
		nocopyable() {}
		~nocopyable() {}
	private:
		nocopyable(const nocopyable&);
		nocopyable& operator=(const nocopyable&);
	};
}

#if defined(HAVE_WINDOWS)
#define __QING_FUNC__ __FUNCTION__
#define QING_TRACE()	printf("[TRACE] [%s:%d]\n", __QING_FUNC__, __LINE__);
#else
#define __QING_FUNC__ __PRETTY_FUNCTION__
#define QING_TRACE()	printf("[TRACE] [%s:%d]\n", __QING_FUNC__, __LINE__);
#endif

#ifdef HAVE_WINDOWS
#ifndef snprintf
#define snprintf		_snprintf_s
#endif // !snprintf
#define strcat_safe		strcat_s
#define strtok_safe		strtok_s
#define get_pid			_getpid
#else
#define strcat_safe		strcat
#define strtok_safe		strtok_r
#define get_pid			getpid
#endif


#ifndef HAVE_WINDOWS
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef unsigned long DWORD;
typedef int BOOL;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr *LPSOCKADDR;
#else

#endif