#ifndef __UPDATE_H__
#define __UPDATE_H__
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
#include "spi_flash_w25qxx.h"

typedef struct update_server_type{
	unsigned long ip;
	unsigned long port;
}update_server_t;

void rt_update_thread_entry(void* parameter);
#endif
