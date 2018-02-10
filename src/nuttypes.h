#ifndef NUTTYPES_H

#define NUTTYPES_H

#if (defined(WIN32) || defined(_WIN32_WCE))
	#include "nuttypes_win.h"
#else
	#include "nuttypes_unix.h"
#endif // WIN32 || _WIN32_WCE

#endif // NUTTYPES_H

