#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#define  __ARM_AM335X_SPI__ 	1
#define NET_SERVER_ADDR "192.168.0.200\0"



#define __DEBUG_4_
#ifdef __DEBUG_4_
#define dbg4_p(format,...) do{printf(format,##__VA_ARGS__);}while(0)
#define dbg4(format,...)   do{printf(__FILE__" %s %ld "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#else
#define dbg4(format,...)
#define dbg4_p(format,...)
#endif




#define __DEBUG_3_
#ifdef __DEBUG_3_
#define dbg3_p(format,...) do{printf(format,##__VA_ARGS__);}while(0)
#define dbg3(format,...) do{printf(__FILE__" %s %ld "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define __DEBUG_0_
#define __DEBUG_1_
#define __DEBUG_2_
#else
#define dbg3(format,...)
#define dbg3_p(format,...)
#endif

#ifdef __DEBUG_2_
#define dbg2(format,...) do{printf(" %s %ld "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define __DEBUG_0_
#define __DEBUG_1_
#else
#define dbg2(format,...)
#endif


#ifdef __DEBUG_1_
#define dbg1(format,...) do{printf("%s "format,__FUNCTION__,##__VA_ARGS__);}while(0)
#define __DEBUG_0_
#else
#define dbg1(format,...)
#endif

#ifdef __DEBUG_0_
#define dbg0(format,...) do{printf(format,##__VA_ARGS__);}while(0)
#define dbg0_d(format,...) do{printf(__FILE__" %s %ld "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#else
#define dbg0(format,...)
#define dbg0_d(format,...)
#endif


//#define ARM_PC_DEBUG 1
#define BYTESPERUINT32 4
#define FPGA_FLASH_SPI_REG_NUM 8
#define FPGA_FLASH_SPI_CMD_NUM 2
#define FPGA_FLASH_SPI_EXT_NUM 8

#define FPGA_FLASH_SPI_OPE_DUM 0xFF
#define FPGA_FLASH_READ_REQUEST  0x03
#define FPGA_FLASH_WRITE_DATA 0x20


#define FPGA_FLASH_READ_STATUS_REG 0x07
//#define FPGA_FLASH_READ_VALID_REG 0x06
#define FPGA_FLASH_READ_DATA_REG 0x08

//#define FPGA_FLASH_CHECK_READ_STATUS 3
//#define FPGA_FLASH_CHECK_WRITE_STATUS 1
//#define FPGA_FLASH_CHECK_REQUEST_STATUS 2

#define FPGA_FLASH_SECTOR_UINT_ADDR 0x1000
//#define FPGA_DEVICE_FLASH   0x07
typedef enum flash_status// return type
{
	FPGA_FLASH_CHECK_WRITE_STATUS=1,
	FPGA_FLASH_CHECK_REQUEST_STATUS,
	FPGA_FLASH_CHECK_READ_STATUS,
	FPGA_FLASH_CHECK_REGISTER_STATUS,
	FPGA_FLASH_CHECK_FPGA_STATUS,//5
	FPGA_FLASH_CHECK_DATA_VALID,


}flash_status_t;


typedef enum flash_operation_ret// return type
{
	FLASH_OPERATION_SUCCESS=0,
	FLASH_TIMEOUT_ERROR,
	FLASH_REQ_ERROR,
	FLASH_READ_ERROR,
	FLASH_WRITE_ERROR,
	FLASH_SPI_READ_ERROR,//5
	FLASH_SPI_WRITE_ERROR,


}result_ret_t;



#endif
