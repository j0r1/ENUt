#ifndef NUTCONFIG_H

#define NUTCONFIG_H

#if (defined(WIN32) || defined(_WIN32_WCE))
	#include "nutconfig_win.h"
#else
	#include "nutconfig_unix.h"
#endif // WIN32 || _WIN32_WCE

#define NUTDEBUG

#endif // NUTCONFIG_H
