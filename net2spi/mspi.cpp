/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mspi.h"





 MSpi::MSpi()
{


}



 MSpi::~MSpi()
{
	pthread_mutex_destroy(&lock_spi);
#if __ARM_AM335X_SPI__
	close(fd_spi);
#endif
}

int MSpi::initialize()
{
#if __ARM_AM335X_SPI__
	fd_spi=open(FPGA_SPI_DEVICE_NAME, O_RDWR);
	if(fd_spi<0)
	{
		std::cout<<FPGA_SPI_DEVICE_NAME<<",not exits or busy!"<<std::endl;
		return fd_spi;
	}
	//dbg0("spi success!\r\n");
#endif
	if(pthread_mutex_init(&lock_spi,NULL)!=0)
	{
		dbg0("err:init lock_spi");
		return -2;
	}
	return 0;
}


 int MSpi::fpga_spi_write(unsigned int *buff,int len)
{
	int ret;
	char *buf=(char*)buff;

	if(buf==NULL ||fd_spi<0)
	{
		dbg0("err:write spi\r\n");
		return -1;
	}

	pthread_mutex_lock(&lock_spi);
#if __ARM_AM335X_SPI__
	ret=write(fd_spi,buf,len*4);
#endif
	pthread_mutex_unlock(&lock_spi);

	return ret;
}

 int MSpi::fpga_spi_read(unsigned int *buff,int len)
{
	int ret;
	char *buf=(char*)buff;

	if(buf==NULL || fd_spi<0)
		return -1;

	pthread_mutex_lock(&lock_spi);
#if __ARM_AM335X_SPI__
	ret=read(fd_spi,buf,len*4);
#endif
	pthread_mutex_unlock(&lock_spi);

	return ret;
}

unsigned int *MSpi::packet_init_head(unsigned char  rw,unsigned char device,unsigned short reg,unsigned int *buff)
{
	unsigned char * ptr=(unsigned char *)buff;

	if(buff==NULL)
		return NULL;

	if(rw==FPGA_RW_WRITE)
	{
		* ptr++ = (FPGA_RW_WRITE)&FPGA_RW_MASK;
	}
	else{
		* ptr++ = (FPGA_RW_READ)&FPGA_RW_MASK;
	}

	* ptr++ = device&FPGA_DEVICE_MASK;

	* ptr++ = (reg>>8)&0xFF;
	* ptr++ =	reg&0xFF;

	*(buff+1)=FPGA_NULL;

	return buff+2;
}

int MSpi::packet_set_data(unsigned int *buff,int idx,unsigned int data)
{
	if(buff==NULL)
		return -1;

	if(idx<2)
		return -2;

	*(buff+idx) = data;

	return idx;
}

int MSpi::packet_cpy_data(unsigned int *buff,int idx,unsigned int *data,int len)
{
	if(buff==NULL ||data==NULL)
		return -1;

	if(idx<2 ||len<1)
		return -2;

	memcpy((char*)(buff+idx),(char*)data,len*sizeof(unsigned int));
// 	int i=0;
//
// 	for(i=0;i<len;i++)
// 	{
// 		*(buff+idx) = *data;
// 		idx++;
// 		data++;
// 	}
//
	return idx;
}

unsigned int MSpi::reverse_data(unsigned int data)
{
	unsigned int tmp=(data&0xFF)<<24;
	tmp |=(data&0xFF00)<<8;
	tmp |=(data&0xFF0000)>>8;
	tmp |=(data&0xFF000000)>>24;
	return tmp;
}
