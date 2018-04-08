#include "update.h"

#define UPDATE_BUFF_SIZE 8192 //4096+128

#define UPDATE_FILE_SIZE 1024*1024*4 //4M 
#define PACKET_FILE_SIZE  (4096+sizeof(mp_head_t))

void rt_update_thread_entry(void* parameter)
{
	struct sockaddr_in saddr;
	int sock_s;
	update_server_t *srv=(update_server_t*)parameter;
	
	unsigned char *update_buff,*pdata;
	unsigned long file_size_cnt=0;
	int reciv_len;
	int temp_len=0;
	mp_head_t *head=NULL;
	rt_device_t flash_dev=NULL;
	
	rt_pin_mode(50,PIN_MODE_OUTPUT);//nConfig
	rt_pin_mode(51,PIN_MODE_OUTPUT);//nCE
	rt_pin_write(50,0);
	rt_pin_write(51,1);
	
	w25qxx_init("flash", "spi00");
	
	flash_dev=rt_device_find("flash");
	if(flash_dev==NULL)
	{
		rt_kprintf("err:can't find flash device!\r\n");
		return;
	}
	rt_device_open(flash_dev,RT_DEVICE_OFLAG_RDWR);
	
	update_buff=rt_malloc(UPDATE_BUFF_SIZE);
	if(update_buff==RT_NULL)
	{
		rt_kprintf("err:malloc sockets buff!\r\n");
		rt_device_close(flash_dev);
		return;
	}
	
	
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(srv->port);
	saddr.sin_addr.s_addr=srv->ip;
	sock_s = socket(AF_INET,SOCK_STREAM,0);
	
	if(sock_s<0)
	{
		rt_free(update_buff);
		rt_device_close(flash_dev);
		rt_kprintf("err:listen sockets!\r\n");
		return;
	}
	/*if(bind(sock_s,(struct sockaddr*)&saddr,sizeof(saddr))<0)
	{
		rt_kprintf("err:bind listen sockets!\r\n");
		return;
	}*/
	if(connect(sock_s,(struct sockaddr*)&saddr,sizeof(saddr))<0)
	{
		rt_kprintf("err:connect sockets!\r\n");
		goto update_err;
	}
	
	while(file_size_cnt<UPDATE_FILE_SIZE)
	{
			int plen,tlen;
		
		reciv_len=0;
		while(reciv_len<PACKET_FILE_SIZE)
		{
		
			temp_len=recv(sock_s,&update_buff[reciv_len],PACKET_FILE_SIZE-reciv_len,0);
			if(temp_len<0)
			{
				goto update_err;
			}
			reciv_len=reciv_len+temp_len;
		}
		
		head=headFind(update_buff,reciv_len);
		if(head==NULL)
		{
				tlen=protocolAck(update_buff,CMD_UPDATE_PACKET,1);
				tlen=send(sock_s, (unsigned char*)update_buff, tlen,0);
			continue;
		}
		
		plen=(unsigned char *)head-update_buff;
		plen=reciv_len-plen;
		while(headIntegrity(head,plen)!=MP_TRUE)
		{
			tlen=recv(sock_s, &update_buff[reciv_len], PACKET_FILE_SIZE-plen, 0);
			if(tlen<=0)
			{
				goto update_err;
			}
			plen=plen+tlen;
			reciv_len=reciv_len+tlen;
		}
		
		if(headValidity(head)!=MP_TRUE)
		{
				head->len=0;
				head->crc=0;
				head->ret=3;
				head->sum=sum((unsigned char*)head,sizeof(mp_head_t)-2);
				tlen=send(sock_s, (unsigned char*)head, sizeof(mp_head_t)+head->len,0);
				continue;
		}
		
		while(dataIntegrity(head,plen-sizeof(mp_head_t))!=MP_TRUE)
		{
			tlen=recv(sock_s, &update_buff[reciv_len], PACKET_FILE_SIZE-plen, 0);
			if(tlen<=0)
			{
				goto update_err;
			}
			plen=plen+tlen;
		}
		
		if(dataValidity(head)!=MP_TRUE)
		{
			head->len=0;
			head->crc=0;
			head->ret=4;
			head->sum=sum((unsigned char*)head,sizeof(mp_head_t)-2);
			tlen=send(sock_s, (unsigned char*)head, sizeof(mp_head_t)+head->len,0);
			continue;
		}
					
		
		pdata=(unsigned char*)head+sizeof(mp_head_t);
		rt_device_write(flash_dev,head->option,pdata,head->len);
		file_size_cnt=file_size_cnt+head->len;
		
		head->len=0;
		head->crc=0;
		head->ret=1;
		head->sum=sum((unsigned char*)head,sizeof(mp_head_t)-2);
		tlen=send(sock_s, (unsigned char*)head, sizeof(mp_head_t)+head->len,0);
		
	}
	
	update_err:
		rt_free(update_buff);
		closesocket(sock_s);
		rt_device_close(flash_dev);
	
	
	
}

