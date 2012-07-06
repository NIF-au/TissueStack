#ifndef __MINC_INFO__
	#define __MINC_INFO__

	#include "core.h"
	#include "utils.h"
	#include <sys/socket.h>

	void *init(void *args);
	void *start(void *args);
	void *unload(void *args);

#endif		/* __MINC_INFO__ */
