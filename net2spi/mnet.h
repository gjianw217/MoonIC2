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

#ifndef MNET_H
#define MNET_H
#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <linux/errno.h>
#include <err.h>
#include <error.h>
#include <errno.h>
#include <netdb.h>

#include <linux/socket.h>
//#include <linux/tcp.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <sys/types.h>
  #include <netinet/tcp.h> //#include <linux/tcp.h>
#include <time.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "mspi.h"
#include "mcontrol.h"

#include "mthread.h"

#include "mnetprotocol.h"
#include "mconfig.h"


#define CLIENT_SN_FILE		"/mooncell/etc/sn"
#define CLIENT_HP203_DIR	"/sys/bus/iio/devices/iio:device1/"
#define CLIENT_VOLTAGE_DIR	"/sys/bus/iio/devices/iio:device0/"

#define FILE_INTENSITY		"in_intensity0_raw"
#define FILE_PROXIMITY		"in_proximity1_raw"
#define FILE_TEMP			"in_temp2_raw"

#define FILE_VOLTAGE_24		"in_voltage1_raw"
#define FILE_VOLTAGE_BAT	"in_voltage0_raw"

#define NET_DAT_BUFFER_SIZE 1024

#define NET_SERVER_PORT 4567
#define SPI_TEST	1

#define LAST_ERRO_NASE	-3000
#define ERRO_CRC 		LAST_ERRO_NASE
#define ERRO_SPI_WRITE	(ERRO_CRC+1)
#define ERRO_SPI_READ	(ERRO_SPI_WRITE+1)


#define ERR_NET_STATUS	-5000

class MNet :  public MThread
{
public:
      MNet();
      ~MNet();
      virtual void run();
      int initialize(MSpi	 *spi,MControl *control,char *srv);
      void pause_key(void);
      unsigned long randomex(unsigned long modnum);
      int recvPackHead(int sock);
      int recvData(int sock);
      int freeComPtr(void);
      int processData(void);
      int rwFpgaReg(void);
      int processAck(bool frCom=true);
      void notify_callback(unsigned long proc,unsigned long res,unsigned long data);
      int reportSn(void);
      int processQuery(void);
      int reportHp203Value(void);
      int  getIioDeviceValue(char *dir,char *name);
      int reportVoltage(void);
      void getVoltage(unsigned long *v1,unsigned long  *v2);
      int stop();

      pack_header_t *pack_header;
      pack_result_t ackResult;
      MNetProtocol *protocol;

      int processModFlash(unsigned int *reg,unsigned int *pReg);

      int ModFlashWrite(uint32_t *reg);
      int ModFlashRead(unsigned int *reg,unsigned int *pReg);

private:
    int readStatus(uint32_t *reg,uint8_t *ptr,uint8_t rw);
    int readRequest(uint32_t *reg,uint8_t *ptr,uint32_t addr,uint16_t len);
    int readData(uint32_t *reg,uint8_t *ptr,uint32_t addr,uint16_t len);
    int ModFlashReadPre(uint32_t *reg, uint32_t addr,unsigned int len);
    uint32_t readDataValidation(void);

private:
    class MNetPrivate* const d;
	int sock;
	static const int DefaultPort = NET_SERVER_PORT;
	struct hostent *host;
	struct sockaddr_in addrSrv;
	char server[100];
	MSpi	 *spi;
	MControl *control;
	static int errlast;


	data_head_t *data_head;
	//
    pack_header_t pack_header_last;
    int iLastDataType;
    int iLastDataAddr;
    int iLastDataLength;

	pack_rw_info_t *pack_info;

	unsigned char *recvDataPtr;
	bool exit;

	struct timeval m_recv_time;
	struct timeval m_send_time;
};
extern MNet     *g_net;
#endif // MNET_H
