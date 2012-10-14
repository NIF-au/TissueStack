#ifndef __MINC_INFO__
#define __MINC_INFO__

#include "core.h"
#include "utils.h"
#include <sys/socket.h>

void *init(void *args);
void *start(void *args);
void *unload(void *args);

extern  t_log_plugin	log_plugin;

#endif		/* __MINC_INFO__ */
