#ifndef NUTTYPES_WIN_H

#define NUTTYPES_WIN_H

#include <winsock2.h>	
#include <ws2tcpip.h>
#ifndef _WIN32_WCE
	#include <sys/types.h>
#endif // _WIN32_WCE

#ifndef INTTYPES_DEFINED

#define INTTYPES_DEFINED

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#endif // INTTYPES_DEFINED

#endif // NUTTYPES_WIN_H
