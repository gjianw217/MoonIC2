/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2014-04-27     Bernard      make code cleanup. 
 */

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
//#include "stm32_eth.h"
#include "stm32f4xx_eth.h"
#endif
#include "spi_fpga.h"


#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif

#include "net2dev.h"
#include <rthw.h>
#include <rtdevice.h>
#include <lwip/sockets.h>
#include "mprotocol.h"
#include "update.h"

void led_idle_hook(void)
{
	static uint8_t led=0;
	static uint32_t led_delay=0;
	
	led_delay++;
	if(led_delay>=0x100000)
	{
		rt_pin_write(0,led);		
		led=!led;
		led_delay=0;
	}
}
extern  void stm32f4xx_spi_bus_init(void);
void rt_init_thread_entry(void* parameter)
{
	    rt_thread_t tid;
			rt_thread_t tid_update;
    /* GDB STUB */
#ifdef RT_USING_GDB
    gdb_set_device("uart6");
    gdb_start();
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        /* register ethernetif device */
        eth_system_device_init();

        rt_hw_stm32_eth_init();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif
		

		
		
		rt_thread_idle_sethook(led_idle_hook);
		
		


		
		{
			int sock_udp;
			struct sockaddr_in saddr;
			struct sockaddr_in caddr;
			int len;
			
			socklen_t alen;
			
			unsigned char *udp_buff;

			udp_buff=rt_malloc(1024);
			if(udp_buff==RT_NULL)
			{
				rt_kprintf("err:malloc sockets buff!\r\n");
				return;
			}
	
			
			saddr.sin_family=AF_INET;
			saddr.sin_port=htons(DEFAULT_UDP_PORT);
			saddr.sin_addr.s_addr=INADDR_ANY;
			sock_udp = socket(AF_INET,SOCK_DGRAM,0);
			if(sock_udp<0)
			{
				rt_kprintf("err:udp sockets!\r\n");
				return;
			}
			
			if(bind(sock_udp,(struct sockaddr*)&saddr,sizeof(saddr))<0)
			{
				rt_kprintf("err:bind listen sockets!\r\n");
				return;
			}
			
		rt_thread_delay(250);
		stm32f4xx_spi_bus_init();
		stm32_dev_fpga_init("fpga", "spifpga");//在board.c中注册会失败，可能是configure调用系统一些功能。
			
    tid = rt_thread_create("net2dev",
        rt_net2dev_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3-1, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
			while(1)
			{
				len=recvfrom(sock_udp, udp_buff, 1024, /*MSG_DONTWAIT*/0,(struct sockaddr *)&caddr, &alen);
				if(len>0)
				{
					int plen=0;
					int tlen=0;
					mp_head_t *head=headFind(udp_buff,len);
					if(head==NULL)
						continue;
					plen=(unsigned char *)head-udp_buff;
					plen=len-plen;
					while(headIntegrity(head,plen)!=MP_TRUE)
					{
						tlen=recvfrom(sock_udp, &udp_buff[len], 1024-len, /*MSG_DONTWAIT*/0,(struct sockaddr *)&caddr, &alen);
						if(tlen<=0)
							continue;
						plen=plen+tlen;
						len=len+tlen;
					}
					if(headValidity(head)!=MP_TRUE)
						continue;
					while(dataIntegrity(head,plen-sizeof(mp_head_t))!=MP_TRUE)
					{
						tlen=recvfrom(sock_udp, &udp_buff[len], 1024-len, /*MSG_DONTWAIT*/0,(struct sockaddr *)&caddr, &alen);
						if(tlen<=0)
							continue;
						plen=plen+tlen;
					}
					if(dataValidity(head)!=MP_TRUE)
						continue;
					
					head->ret=1;
					switch(head->cmd)
					{
						case CMD_SERCH_ALL:
						case CMD_SERCH_BY_IP:
						{
								extern struct netif *netif_default;
								mp_search_ack_data_t *one=(mp_search_ack_data_t*)((unsigned char*)head+sizeof(mp_head_t));
								one->dhcp=1;
								rt_memcpy(one->frimver,FRIM_VER,rt_strlen(FRIM_VER)>6?6:rt_strlen(FRIM_VER));
								//rt_get_mac_addr((unsigned char *)one->mac);
								memcpy(one->mac,netif_default->hwaddr,netif_default->hwaddr_len>6?6:netif_default->hwaddr_len);
								one->ip=netif_default->ip_addr.addr;
								one->mod=1;
								one->type=1;
								one->port=DEFAULT_PORT;
							
								head->len=sizeof(mp_search_ack_data_t);
								head->crc=crc((unsigned char*)one,head->len);
								
						}
							break;
						case CMD_UPDATE_SERVER:
						{	
							update_server_t *serv=(update_server_t*)((unsigned char*)head+sizeof(mp_head_t));
							
							tid_update = rt_thread_create("update",
									rt_update_thread_entry, serv,
									2048, RT_THREAD_PRIORITY_MAX/3-2, 21);

							if (tid_update != RT_NULL)
									rt_thread_startup(tid_update);
							
							
						}break;			
						
						default:
							head->ret=2;
							head->crc=0;
							head->len=0;
							break;
						
					}
					head->sum=sum((unsigned char*)head,sizeof(mp_head_t)-2);
					sendto(sock_udp, (unsigned char*)head, sizeof(mp_head_t)+head->len, /*MSG_DONTWAIT*/0,(struct sockaddr *)&caddr, alen);
					
				}
				//rt_pin_write(22,led);
				//rt_thread_delay(100);
				//led=!led;
				rt_thread_delay(2);
			}
		}
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}


/*@}*/
