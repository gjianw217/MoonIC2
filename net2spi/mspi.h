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

#ifndef MSPI_H
#define MSPI_H

#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"

#include "mconfig.h"

#define FPGA_SPI_DEVICE_NAME  "/dev/fpga_spi2.0"

#define FPGA_RW_WRITE 0xA5
#define FPGA_RW_READ  0x98

#define FPGA_DEVICE_BATTERY 0x00
#define FPGA_DEVICE_PANEL	0x01
#define FPGA_DEVICE_MOVE	0x02
#define FPGA_DEVICE_PROBE	0x03
#define FPGA_DEVICE_EXPOS	0x04
#define FPGA_DEVICE_PMBUS	0x05
#define FPGA_DEVICE_FLASH   0x07


#define FPGA_FLASH_SECTOR_SIZE 4096

#define FPGA_NULL			0xFFFFFFFF

#define FPGA_RW_MASK		0x000000FF
#define FPGA_DEVICE_MASK	0x000000FF
#define FPGA_REG_MASK		0x0000FFFF


using namespace std;

class MSpi
{
public:
	MSpi();
	~MSpi();
	int initialize();
	int fpga_spi_write(unsigned int *buff,int len);
	int fpga_spi_read(unsigned int *buff,int len);
	unsigned int *packet_init_head(unsigned char  rw,unsigned char device,unsigned short reg,unsigned int *buff);
	int packet_set_data(unsigned int *buff,int idx,unsigned int data);
	int packet_cpy_data(unsigned int *buff,int idx,unsigned int *data,int len);
	unsigned int reverse_data(unsigned int data);

private:
	int fd_spi;
	pthread_mutex_t  lock_spi;

};

#endif // MSPI_H
