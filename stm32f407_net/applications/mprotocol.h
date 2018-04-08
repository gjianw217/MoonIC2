#ifndef __MPROTOCOL_H__
#define __MPROTOCOL_H__

#define CMD_SERCH_ALL					0x01		//UDP
#define CMD_SERCH_BY_IP				0x02  //UDP


#define CMD_SET_DATA_OUT			0x03
#define CMD_GET_DATA_IN				0x04
#define CMD_SET_DATA_OUT_PIN	0x05
#define CMD_GET_DATA_IN_PIN		0x06
#define CMD_SPI_WRITE					0x07
#define CMD_SPI_READ					0x08

#define CMD_UPDATE_SERVER			0x09 //UDP
#define CMD_UPDATE_PACKET			0x0A //TCP

#define CMD_GET_DATA_OUT			0x0B
#define CMD_GET_DATA_OUT_PIN	0x0C
#define CMD_GET_MC						0x0D
#define CMD_READ_ALL					0x0E //CMD_GET_DATA_IN|CMD_GET_MC2
#define CMD_MOTOR_CTL					0x0F 
#define CMD_MOTOR_STATUS			0x10 

#define CMD_SET_GLOBAL_MASK		0x11 


#define CMD_STOP							0xAA //use pc thread



#define MP_TRUE		1
#define MP_FALSE	0

#pragma pack(1)
typedef struct  mp_head_type{
	unsigned long head;
	unsigned char cmd;
	unsigned char ret;
	unsigned long option;
	unsigned short len;
	unsigned long crc;
	unsigned short sum;
}mp_head_t;

#pragma pack(1)
typedef struct   mp_search_ack_data_type{
	unsigned long ip;
	unsigned char mac[6];
	unsigned char frimver[6];
	unsigned char type;
	unsigned char mod;
	unsigned short dhcp;
	unsigned short port;
}mp_search_ack_data_t;

	#pragma pack(1)
	typedef struct mp_data_out_type{
		mp_head_t head;
		unsigned long pin_mask0;
		unsigned long pin_mask1;
		unsigned long  val0;
		unsigned long  val1;
	}mp_data_out_t;

#pragma pack(1)
	typedef struct mp_data_in_type{
		mp_head_t head;
		unsigned long pin_mask0;
		unsigned long pin_mask1;
	}mp_data_in_t;
#pragma pack(1)
	/*typedef struct mp_data_get_out_type{
		mp_head_t head;
		unsigned long val0;
		unsigned long val1;
	}mp_data_get_out_t;*/
	
	#pragma pack(1)
	typedef struct mp_data_out_pin_type{
		mp_head_t head;
		unsigned short pin;
		unsigned short val;
	}mp_data_out_pin_t;
	
	#pragma pack(1)
	typedef struct mp_data_in_pin_type{
		mp_head_t head;
		unsigned long pin;
	}mp_data_in_pin_t;
	
	#pragma pack(1)
	typedef struct mp_spi_write_type{
		mp_head_t head;
		unsigned char data[1];
	}mp_spi_write_t;
	
	#pragma pack(1)
	typedef struct mp_spi_read_type{
		mp_head_t head;
		unsigned char data[1];
	}mp_spi_read_t;
	
	
	#pragma pack(1)
	typedef struct mp_mc_type{
		mp_head_t head;
		unsigned long use;
		unsigned long mod;
		unsigned long set[3];
	}mp_mc_t;
	
	#pragma pack(1)
	typedef struct mp_read_all_type{
		mp_head_t head;
		unsigned long pin_mask0;
		unsigned long pin_mask1;
		unsigned long  val0;
		unsigned long  val1;
		
		unsigned long use;
		unsigned long mod;
		unsigned long set[3];
	}mp_read_all_t;
	
	unsigned short sum(unsigned char * data,int len);
	unsigned long crc(unsigned char * data,int len);

	void head(mp_head_t *h);
	int searchAll(unsigned char * data);
	int searchByIp(unsigned char * data,unsigned long ip);
	int setDateOut(unsigned char * data,unsigned long pin_mask0,unsigned long pin_mask1,unsigned long value0,unsigned long value1);
	int setDateOutPin(unsigned char * data,unsigned char pin,unsigned char value);
	int getDateIn(unsigned char * data,unsigned long pin_mask0,unsigned long pin_mask1);
	int getDateInPin(unsigned char * data,unsigned char pin);
	int spiWrite(unsigned char * data,unsigned char *buff,unsigned short len);
	int spiRead(unsigned char * data,unsigned char *buff,unsigned short len);
	int protocolAck(unsigned char * data,unsigned short cmd,unsigned short ret);
	
	mp_head_t *headFind(unsigned char * data,unsigned long len);
	unsigned char  headValidity(mp_head_t *head);//有效性
	unsigned char  dataValidity(mp_head_t *head);
	unsigned char  headIntegrity(mp_head_t *head,int len);//完整性
	unsigned char  dataIntegrity(mp_head_t *head,int len);

	mp_search_ack_data_t *get_search_one(mp_head_t *head);
	
	unsigned long spi_swap(unsigned long d);
	int spi_packt(unsigned long *buff,char rw,unsigned short addr,char type);
	

#endif
