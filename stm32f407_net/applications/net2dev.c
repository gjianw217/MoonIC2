#include "net2dev.h"
#include "motor_ctl.h"

#define DEV_BUFF_SIZE	4096+64
unsigned long g_pin_mode_mask0=0;//0输入
unsigned long g_pin_mode_mask1=0;//1输出
unsigned long g_def_pin_mode_mask0=0x00FFFFFF;//0输入
unsigned long g_def_pin_mode_mask1=0;//1输出
unsigned long g_rev_pin_mode_mask0=0x00000600;//1保留IO
unsigned long g_rev_pin_mode_mask1=0;//

void rt_net2dev_thread_entry(void* parameter)
{
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	int sock_listen;
	int sock_client=-1;
	socklen_t a_len;
	int len;
	int i=0;
	int val=0;
	rt_device_t spidev=NULL;
	unsigned char *net2dev_buff;
	
	net2dev_buff=rt_malloc(DEV_BUFF_SIZE);
	if(net2dev_buff==RT_NULL)
	{
		rt_kprintf("err:malloc sockets buff!\r\n");
		return;
	}
	
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(DEFAULT_PORT);
	saddr.sin_addr.s_addr=INADDR_ANY;
	sock_listen = socket(AF_INET,SOCK_STREAM,0);
	if(sock_listen<0)
	{
		rt_kprintf("err:listen sockets!\r\n");
		return;
	}
	if(bind(sock_listen,(struct sockaddr*)&saddr,sizeof(saddr))<0)
	{
		rt_kprintf("err:bind listen sockets!\r\n");
		return;
	}
	if(listen(sock_listen, 0)<0)
	{
		rt_kprintf("err:bind listen sockets!\r\n");
		return;
	}
	
	g_pin_mode_mask0=0;
	g_pin_mode_mask1=0;
	for(i=1;i<=48;i++)
	{
		rt_pin_mode(i,PIN_MODE_INPUT_PULLUP);
	}
	spidev=rt_device_find("fpga");
	rt_device_open(spidev,RT_DEVICE_OFLAG_RDWR);
		while(1)
		{
			int keepAlive=1;//设定keepAlive
			int keepIdle = 2;//首次KeepAlive探测TCP时间
			int keepInterval = 2;//每两次KeepAlive探测时间
			int keepCount = 3;//探测KeepAlive次数
			
			sock_client=accept(sock_listen,(struct sockaddr*)&caddr,&a_len);
			if(sock_client<0)
					continue;
			
			/****************************************** keepAlive ****************************************/
			if(setsockopt(sock_client,SOL_SOCKET,SO_KEEPALIVE,&keepAlive,sizeof(keepAlive))<0)
			{
				rt_kprintf("war:con't set keepAlive!\r\n");
			}
			
			if(setsockopt(sock_client,IPPROTO_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle)) < -1)//IPPROTO_TCP-->SOL_TCP
			{
					rt_kprintf("war:con't set keepIdle!\r\n");
			}

			if(setsockopt(sock_client,IPPROTO_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval))< -1)
			{
					rt_kprintf("war:con't set keepInterval!\r\n");
			}

			if(setsockopt(sock_client,IPPROTO_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount)) < -1)
			{
					rt_kprintf("war:con't set keepCount!\r\n");
			}
			
			/****************************************** end keepAlive ****************************************/
			while(1)
			{
				int plen=0;
				int tlen=0;
				mp_head_t *head=NULL;
				len=recv(sock_client,net2dev_buff,DEV_BUFF_SIZE,0);
				if(len<=0)
				{
					goto sockets_err;
				}

					head=headFind(net2dev_buff,len);
					if(head==NULL)
						continue;
					plen=(unsigned char *)head-net2dev_buff;
					plen=len-plen;
					while(headIntegrity(head,plen)!=MP_TRUE)
					{
						tlen=recv(sock_client, &net2dev_buff[len], DEV_BUFF_SIZE-len, 0);
						if(tlen<=0)
							goto sockets_err;
						plen=plen+tlen;
						len=len+tlen;
					}
					if(headValidity(head)!=MP_TRUE)
						continue;
					while(dataIntegrity(head,plen-sizeof(mp_head_t))!=MP_TRUE)
					{
						tlen=recv(sock_client, &net2dev_buff[len], DEV_BUFF_SIZE-len, 0);
						if(tlen<=0)
							goto sockets_err;
						plen=plen+tlen;
					}
					if(dataValidity(head)!=MP_TRUE)
						continue;
					
					head->ret=1;
					switch(head->cmd)
					{
						case CMD_SET_DATA_OUT:
						{
								mp_data_out_t *out=(mp_data_out_t*)head;
							
							if(g_rev_pin_mode_mask0&out->pin_mask0)
							{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
							}
							if(g_rev_pin_mode_mask1&out->pin_mask1)
							{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
							}
								for(i=0;i<32;i++)
								{
									if((out->pin_mask0&(1<<i)))
									{
										if((g_def_pin_mode_mask0&(1<<i))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								for(i=0;i<16;i++)
								{
									if((out->pin_mask1&(1<<i)))
									{
										if((g_def_pin_mode_mask1&(1<<i))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								
								
								for(i=0;i<32;i++)
								{
									if((out->pin_mask0&(1<<i)))
									{
										if((g_pin_mode_mask0&(1<<i))==0)
										{
											rt_pin_mode(i+1,PIN_MODE_OUTPUT);
											g_pin_mode_mask0=g_pin_mode_mask0|(1<<i);
										}
										val=(out->val0&(1<<i))?1:0;
										rt_pin_write(i+1,val);
									}
								}
								for(i=0;i<16;i++)
								{
									if((out->pin_mask1&(1<<i)))
									{
										if((g_pin_mode_mask1&(1<<i))==0)
										{
											rt_pin_mode(33+i,PIN_MODE_OUTPUT);
											g_pin_mode_mask0=g_pin_mode_mask0|(1<<i);
										}
										
										val=(out->val1&(1<<i))?1:0;
										rt_pin_write(33+i,val);
									}
								}
								head->len=0;
								head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}break;
						case CMD_GET_DATA_IN:
						{
							mp_data_in_t *in=(mp_data_in_t*)head;
							mp_data_out_t *out=(mp_data_out_t*)head;
							
							if(g_rev_pin_mode_mask0&in->pin_mask0)
							{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
							}
							if(g_rev_pin_mode_mask1&in->pin_mask1)
							{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
							}
							
								for(i=0;i<32;i++)
								{
									if((out->pin_mask0&(1<<i)))
									{
										if((g_def_pin_mode_mask0&(1<<i)))//默认输出
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								for(i=0;i<16;i++)
								{
									if((out->pin_mask1&(1<<i)))
									{
										if((g_def_pin_mode_mask1&(1<<i)))//默认输出
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								for(i=0;i<32;i++)
								{
									if((in->pin_mask0&(1<<i)))
									{
											if((g_pin_mode_mask0&(1<<i)))
											{
												rt_pin_mode(1+i,PIN_MODE_INPUT_PULLUP);
												g_pin_mode_mask0=g_pin_mode_mask0&(~(1<<i));
											}
											if(rt_pin_read(1+i))
											{
												out->val0=out->val0|(1<<i);
											}
											else
											{
												out->val0=out->val0&(~(1<<i));
											}	
									}
								}
								
								for(i=0;i<16;i++)
								{
									if((in->pin_mask1&(1<<i)))
									{
											if((g_pin_mode_mask1&(1<<i)))
											{
												rt_pin_mode(33+i,PIN_MODE_INPUT_PULLUP);
												g_pin_mode_mask1=g_pin_mode_mask1&(~(1<<i));
											}
											
											if(rt_pin_read(33+i))
											{
												out->val1=out->val1|(1<<i);
											}
											else
											{
												out->val1=out->val1&(~(1<<i));
											}	
									}
								}				
								head->len=sizeof(mp_data_out_t)-sizeof(mp_head_t);
								head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
							
						}break;
						case CMD_SET_DATA_OUT_PIN:
						{
							mp_data_out_pin_t *opin=(mp_data_out_pin_t*)head;
							
							
							if(opin->pin<33)
							{
										if(g_rev_pin_mode_mask0&(1<<(opin->pin-1)))
										{
													head->ret=100;
													head->len=0;
													head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
													goto err_ack;
										}

										if((g_def_pin_mode_mask0&(1<<(opin->pin-1)))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}

										if((g_pin_mode_mask0&(1<<(opin->pin-1)))==0)
										{
											rt_pin_mode(opin->pin,PIN_MODE_OUTPUT);
											g_pin_mode_mask0=g_pin_mode_mask0|(1<<(opin->pin-1));
										}
							}
							else
							{
										if(g_rev_pin_mode_mask1&(1<<(opin->pin-33)))
										{
													head->ret=100;
													head->len=0;
													head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
													goto err_ack;
										}
										if((g_def_pin_mode_mask1&(1<<(opin->pin-33)))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
										if((g_pin_mode_mask1&(1<<(opin->pin-33)))==0)
										{
											rt_pin_mode(opin->pin,PIN_MODE_OUTPUT);
											g_pin_mode_mask1=g_pin_mode_mask1|(1<<(opin->pin-33));
										}
							}
							
							rt_pin_write(opin->pin,opin->val?1:0);
							head->len=0;
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}break;
						case CMD_GET_DATA_OUT:
						{
							mp_data_in_t *in=(mp_data_in_t*)head;
							mp_data_out_t *out=(mp_data_out_t*)head;
							out->val1=0;
							out->val1=0;
								if(g_rev_pin_mode_mask0&out->pin_mask0)
								{
											head->ret=100;
											head->len=0;
											head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
											goto err_ack;
								}
								if(g_rev_pin_mode_mask1&out->pin_mask1)
								{
											head->ret=100;
											head->len=0;
											head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
											goto err_ack;
								}
							
								for(i=0;i<32;i++)
								{
									if((out->pin_mask0&(1<<i)))
									{
										if((g_def_pin_mode_mask0&(1<<i))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								for(i=0;i<16;i++)
								{
									if((out->pin_mask1&(1<<i)))
									{
										if((g_def_pin_mode_mask1&(1<<i))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
									}
								}
								
								for(i=0;i<32;i++)
								{
									if((in->pin_mask0&(1<<i)))
									{
											//if((g_pin_mode_mask0&(1<<i)))
											//{
											//	rt_pin_mode(i,PIN_MODE_INPUT_PULLUP);
											//	g_pin_mode_mask0=g_pin_mode_mask0&(~(1<<i));
											//}
											if(rt_pin_get_write(1+i))
											{
												out->val0=out->val0|(1<<i);
											}
											else
											{
												out->val0=out->val0&(~(1<<i));
											}	
									}
								}
								
								for(i=0;i<16;i++)
								{
									if((in->pin_mask1&(1<<i)))
									{
											//if((g_pin_mode_mask1&(1<<i)))
											//{
											//	rt_pin_mode(32+i,PIN_MODE_INPUT_PULLUP);
											//	g_pin_mode_mask1=g_pin_mode_mask1&(~(1<<i));
											//}
											
											if(rt_pin_get_write(33+i))
											{
												out->val1=out->val1|(1<<i);
											}
											else
											{
												out->val1=out->val1&(~(1<<i));
											}	
									}
								}				
								head->len=sizeof(mp_data_out_t)-sizeof(mp_head_t);
								head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
							
						}break;
						case CMD_GET_DATA_OUT_PIN:
						{
							mp_data_in_pin_t *ipin=(mp_data_in_pin_t*)head;			
							mp_data_out_pin_t *opin=(mp_data_out_pin_t*)head;
							if(opin->pin<33)
							{
								if(g_rev_pin_mode_mask0&(1<<(opin->pin-1)))
								{
											head->ret=100;
											head->len=0;
											head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
											goto err_ack;
								}
										if((g_def_pin_mode_mask0&(1<<(opin->pin-1)))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
							}
							else
							{
								if(g_rev_pin_mode_mask1&(1<<(opin->pin-33)))
								{
											head->ret=100;
											head->len=0;
											head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
											goto err_ack;
								}
										if((g_def_pin_mode_mask1&(1<<(opin->pin-33)))==0)//默认输入
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
							}
							opin->pin=ipin->pin;
							opin->val=rt_pin_get_write(opin->pin)?1:0;
							head->len=sizeof(mp_data_out_pin_t)-sizeof(mp_head_t);
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}break;
						case CMD_GET_DATA_IN_PIN:
						{
							mp_data_in_pin_t *ipin=(mp_data_in_pin_t*)head;			
							mp_data_out_pin_t *opin=(mp_data_out_pin_t*)head;
							if(ipin->pin<33)
							{
										if(g_rev_pin_mode_mask0&(1<<(ipin->pin-1)))
										{
													head->ret=100;
													head->len=0;
													head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
													goto err_ack;
										}
										if((g_def_pin_mode_mask0&(1<<(ipin->pin-1))))//默认输出
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
										
										if((g_pin_mode_mask0&(1<<(ipin->pin-1))))
										{
											rt_pin_mode(ipin->pin,PIN_MODE_INPUT_PULLUP);
											g_pin_mode_mask0=g_pin_mode_mask0|(1<<(ipin->pin-1));
										}
							}
							else
							{
										if(g_rev_pin_mode_mask1&(1<<(ipin->pin-33)))
										{
													head->ret=100;
													head->len=0;
													head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
													goto err_ack;
										}	
										if((g_def_pin_mode_mask1&(1<<(ipin->pin-33)))==0)//默认输出
										{
												head->ret=100;
												head->len=0;
												head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
												goto err_ack;
										}
										if((g_pin_mode_mask1&(1<<(ipin->pin-33)))==0)
										{
											rt_pin_mode(ipin->pin,PIN_MODE_INPUT_PULLUP);
											g_pin_mode_mask1=g_pin_mode_mask1|(1<<(ipin->pin-33));
										}
							}
							opin->pin=ipin->pin;
							opin->val=rt_pin_read(opin->pin)?1:0;
							head->len=sizeof(mp_data_out_pin_t)-sizeof(mp_head_t);
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}break;
						case CMD_SPI_WRITE:
						{
							mp_spi_write_t *swrite=(mp_spi_write_t*)head;

							rt_device_write(spidev,0,swrite->data,swrite->head.len);
							head->len=0;
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
							
						}break;
						case CMD_SPI_READ:		
						{
							unsigned int cp=0;
							mp_spi_read_t *sread=(mp_spi_read_t*)head;
							head->len=rt_device_read(spidev,0,sread->data,sread->head.option);
							//memcpy(sread->data,&sread->data[8],head->len);
							for(cp=0;cp<head->len;cp++)
							{
								sread->data[cp]=sread->data[cp+8];
							}
							sread->data[head->len]=0;
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}
							break;
						case CMD_GET_MC:		
						{
							unsigned long buf_tx[128];
							unsigned long i=0,t=0;
							mp_mc_t *sread=(mp_mc_t*)head;
							
							memset(buf_tx,0,128*4);
							spi_packt((unsigned long*)buf_tx,0,0,1);
							
							rt_device_read(spidev,0,buf_tx,24*4);
							
							
							sread->use=0;
							sread->mod=0;
							sread->set[0]=0;
							sread->set[1]=0;
							sread->set[2]=0;
							
							
							
							for(i=0;i<24;i++)
							{
								if((buf_tx[i*4+2]==0xFFFFFFFF) || (buf_tx[i*4+3]==0xFFFFFFFF))
									break;
								
								sread->use=sread->use|1<<i;
								if(((spi_swap(buf_tx[i*4+4])>>8)&0xFF)==0x1a)
									sread->mod=sread->mod|1<<i;
								
								if((buf_tx[i*4+2]==0) && (buf_tx[i*4+3]==0))
									continue;
								
									t=spi_swap(buf_tx[i*4+3]);
									if((t&0xFFFF)==0xac01)
										sread->set[i/8]=sread->set[i/8]|(1<<((1%8)*4));
									
									t=t>>16;
									if((t&0xFFFF)==0xac02)
										sread->set[i/8]=sread->set[i/8]|((1<<((1%8)*4))+1);	
									
								
								
									t=spi_swap(buf_tx[i*4+2]);
									if((t&0xFFFF)==0xac03)
										sread->set[i/8]=sread->set[i/8]|((1<<((1%8)*4))+2);	
									
									t=t>>16;
									if((t&0xFFFF)==0xac04)
										sread->set[i/8]=sread->set[i/8]|((1<<((1%8)*4))+3);					

							}
							head->len=sizeof(unsigned long)*5;
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}
							break;
						case CMD_READ_ALL:		
						{
							unsigned long buf_tx[128];
							unsigned long i=0,t=0;

							mp_data_in_t *in=(mp_data_in_t*)head;
							mp_read_all_t *out=(mp_read_all_t*)head;
							
								if(g_rev_pin_mode_mask0&in->pin_mask0)
								{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
								}								
								if(g_rev_pin_mode_mask1&in->pin_mask1)
								{
										head->ret=100;
										head->len=0;
										head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
										goto err_ack;
								}	
								for(i=0;i<32;i++)
								{
									if((in->pin_mask0&(1<<i)))
									{
											if(((g_pin_mode_mask0&g_def_pin_mode_mask0)&(1<<i)))
											{
												//rt_pin_mode(1+i,PIN_MODE_INPUT_PULLUP);
												//g_pin_mode_mask0=g_pin_mode_mask0&(~(1<<i));
													if(rt_pin_get_write(1+i))
													{
														out->val0=out->val0|(1<<i);
													}
													else
													{
														out->val0=out->val0&(~(1<<i));
													}	
											}
											else
											{
													if(rt_pin_read(1+i))
													{
														out->val0=out->val0|(1<<i);
													}
													else
													{
														out->val0=out->val0&(~(1<<i));
													}	
												
											}

									}
								}
								
								for(i=0;i<16;i++)
								{
									if((in->pin_mask1&(1<<i)))
									{
										//if(((g_pin_mode_mask0&g_def_pin_mode_mask0)&(1<<i)))
											if(((g_pin_mode_mask1&g_def_pin_mode_mask1)&(1<<i)))
											{
												//rt_pin_mode(33+i,PIN_MODE_INPUT_PULLUP);
												//g_pin_mode_mask1=g_pin_mode_mask1&(~(1<<i));
													if(rt_pin_get_write(33+i))
													{
														out->val1=out->val1|(1<<i);
													}
													else
													{
														out->val1=out->val1&(~(1<<i));
													}

											}
											else{
													if(rt_pin_read(33+i))
													{
														out->val1=out->val1|(1<<i);
													}
													else
													{
														out->val1=out->val1&(~(1<<i));
													}
										}											
									}
								}		

							
							memset(buf_tx,0,128*4);
							spi_packt((unsigned long*)buf_tx,0,0,1);
							
							rt_device_read(spidev,0,buf_tx,24*4);
							
							
							out->use=0;
							out->mod=0;
							out->set[0]=0;
							out->set[1]=0;
							out->set[2]=0;
							
							
							
							for(i=0;i<24;i++)
							{
								if((buf_tx[i*4+2]==0xFFFFFFFF) || (buf_tx[i*4+3]==0xFFFFFFFF))
									break;
								
								out->use=out->use|1<<i;
								if(((spi_swap(buf_tx[i*4+4])>>8)&0xFF)==0x1a)
									out->mod=out->mod|1<<i;
								
								if((buf_tx[i*4+2]==0) && (buf_tx[i*4+3]==0))
									continue;
								
									t=spi_swap(buf_tx[i*4+3]);
									if((t&0xFFFF)==0xac01)
										out->set[i/8]=out->set[i/8]|(1<<((1%8)*4));
									
									t=t>>16;
									if((t&0xFFFF)==0xac02)
										out->set[i/8]=out->set[i/8]|((1<<((1%8)*4))+1);	
									
								
								
									t=spi_swap(buf_tx[i*4+2]);
									if((t&0xFFFF)==0xac03)
										out->set[i/8]=out->set[i/8]|((1<<((1%8)*4))+2);	
									
									t=t>>16;
									if((t&0xFFFF)==0xac04)
										out->set[i/8]=out->set[i/8]|((1<<((1%8)*4))+3);					

							}	
								head->len=sizeof(mp_read_all_t)-sizeof(mp_head_t);
								head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}
							break;
						case CMD_MOTOR_CTL:
						{
							
						}
						break;
						case CMD_MOTOR_STATUS:
						{
							
						}break;
						case CMD_SET_GLOBAL_MASK:
						{
							mp_data_in_t *in=(mp_data_in_t*)head;
							g_def_pin_mode_mask0=in->pin_mask0;
							g_def_pin_mode_mask1=in->pin_mask1;
							head->len=0;
							head->crc=crc((unsigned char*)head+sizeof(mp_head_t),head->len);
						}
						default:
							head->ret=2;
							head->crc=0;
							head->len=0;
							break;
						
					}
err_ack:					
					head->sum=sum((unsigned char*)head,sizeof(mp_head_t)-2);
					len=send(sock_client, (unsigned char*)head, sizeof(mp_head_t)+head->len,0);
					if(len<=0)
					{
						goto sockets_err;
					}
					rt_thread_delay(2);
			}		
			sockets_err:
			closesocket(sock_client);
			rt_kprintf("close tcp sockets %d!\r\n",len);
			sock_client=-1;
			rt_thread_delay(2);
		}
		rt_device_close(spidev);
}

