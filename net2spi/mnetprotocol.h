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

#ifndef MNETPROTOCOL_H
#define MNETPROTOCOL_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//
#pragma data_alignment=1;

#define PROTOCOL_VER_MAX 0x01
#define PROTOCOL_VER_MIN 0x01


#define PROTOCOL_RW_WRITE 0xA5
#define PROTOCOL_RW_READ  0x98

#define FPGA_TYPE_MOVE		0x02
#define FPGA_TYPE_PROBE		0x03


typedef enum cmd_type// 命令
{
	CMD_REQ_IO = (1 << 0) ,    // 命令请求
	CMD_REP_IO = (~CMD_REQ_IO), // 命令应答
	CMD_NOTIFY_IO = (1 << 1)  ,  // IO通知
	CMD_QUERY_REQ_IO = (1<<2) , // IO查询请求
	CMD_QUERY_REP_IO = (~CMD_QUERY_REQ_IO) , // 查询应答
}cmd_type_t ;

typedef enum pack_status// 命令
{
 PACK_RECIVE_HEADER=0,
 PACK_RECIVE_DATE,
 PACK_PROCESS_DATE,
 PACK_PROCESS_QUERY,
 PACK_ACK,
}pack_status_t;


typedef enum result_ack// 命令
{
	ACK_SUCCESS=0,
	ACK_ERR_PACK_HEADER,
	ACK_ERR_MALLOC,
	ACK_ERR_DATA_HEADER,
	ACK_TEST,
	ACK_WAR_PROTOCOL_VER,//5
	ACK_ERR_SPI_WRITE,
	ACK_ERR_SPI_READ,
	ACK_ERR_DATA_LENGTH,
	ACK_ERR_RUN_APP,
	ACK_ERR_CRC_CHECK,	//10
	ACK_ERR_NET_SEND,
	ACK_ERR_NET_RECV,
	ACK_ERR_SPI_WRITE_CRC,
	ACK_ERR_SUM_CHECK,
	ACK_ERR_FILE_OPEN,	//15
	ACK_ERR_FILE_OFFSET,
	ACK_ERR_FLASH_WRITE,
	ACK_ERR_NO_MOVE_STATUS,
	ACK_ERR_UNKNOW_SN,
	ACK_ERR_DEVICE,//20
	ACK_FLASH_WRITE_LAST_READ_ERROR,
    ACK_FLASH_WRITE_START_READ_ERROR,
    ACK_FLASH_WRITE_STATUS_BUSY_ERROR,
    ACK_FLASH_WRITE_SECTOR_READ_ERROR,
    ACK_FLASH_WRITE_SECTOR_CRC_ERROR,//25
    ACK_FLASH_REWRITE_SECTOR_ERROR,


}result_ack_t;

//#define FLASH_WRITE_ERROR (FLASH_WRITE_LAST_READ_ERROR||FLASH_WRITE_START_READ_ERROR|| \
    FLASH_WRITE_STATUS_BUSY_ERROR||FLASH_WRITE_SECTOR_READ_ERROR||FLASH_WRITE_SECTOR_CRC_ERROR)

typedef struct pack_header
{
	unsigned short ver;			// 版本
	unsigned short dataLen;		// 数据长度
	unsigned short cmdType;		// 命令 cmd_type_t
	unsigned long  cmdOption;
	unsigned short option;		// 可选项
	unsigned long chkSum;		// 校验和

}__attribute__((packed)) pack_header_t ;

typedef struct pack_result
{
	unsigned long res ; // 结果
	unsigned long crc ; // 数据crc
}__attribute__((packed)) pack_result_t;

//type
#define DATA_TYPE_CONTROL_MOVE  	0xFF
#define DATA_TYPE_UPDATE_FPGA		0xFE
#define DATA_TYPE_KEY_REPORT		0xFD
#define DATA_TYPE_SN_REPORT			0xFC
#define DATA_TYPE_INTENSITY_REPORT	0xFB
#define DATA_TYPE_VOLTAGE_REPORT	0xFA


#define OPTION_NOTIFY_IO_KEY_PAUSE  		0x3B8C
#define OPTION_QUERY_REP_IO_SN		  		0x3D75
#define OPTION_QUERY_REP_IO_INTENSITY  		0x3A46
#define OPTION_QUERY_REP_IO_VOLTAGE  		0x34C7

#define OPTION_UPDATE_FPGA_BGN 0xA93B
#define OPTION_UPDATE_FPGA_END 0xA93C

#define OPTION_UPDATE_PROCESS_FPGA_BGN		0xA93D
#define OPTION_UPDATE_PROCESS_FPGA_ING		0xA93E
#define OPTION_UPDATE_PROCESS_FPGA_END		0xA93F
#define OPTION_UPDATE_PROGRESS_FPGA_QUERY	0xA940
#define OPTION_UPDATE_MULTI_DATA			0x538B
#define OPTION_EXECUTE_MULTI_CMD			0x57A9

typedef struct pack_data_head
{
	unsigned char rw;	// 读写标志
	unsigned char type; // 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
	unsigned long addr; // 地址 1字节地址
	unsigned short datLen; // 1字节长度
	unsigned long crc;  // 数据
	unsigned long res ; // 结果
	unsigned long chkSum; // 校验和
}__attribute__((packed)) data_head_t ;

typedef struct pack_rw_info
{
	data_head_t dataHead;
	unsigned char data[1];
}__attribute__((packed)) pack_rw_info_t;





class MNetProtocol
{
public:
MNetProtocol();
~MNetProtocol();
static unsigned long checkSum(unsigned char *data,int len);
unsigned long checkSum(unsigned long sum,unsigned char *data,int len);
unsigned long crc32 (unsigned char *block, unsigned int length);

pack_header_t *getHeaderFormNet(unsigned char *data);
pack_result_t setResult(result_ack_t res,unsigned long crc );
data_head_t *getDataHeadFormNet(unsigned char *data);

pack_status_t status;

};

#endif // MNETPROTOCOL_H
