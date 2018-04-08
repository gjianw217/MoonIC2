#ifndef __NET2DEV_H__
#define __NET2DEV_H__
#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#endif
#include <lwip/sockets.h>
#include <lwip/tcp.h>
#include "mprotocol.h"
#include <rtdevice.h>
#define DEFAULT_PORT 3572
#define DEFAULT_UDP_PORT 3572

void rt_net2dev_thread_entry(void* parameter);
#endif
