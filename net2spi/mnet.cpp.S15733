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

#include "mnet.h"
#include "mnet_p.h"
#include <sys/time.h>


int MNet::errlast=0;

MNetPrivate::MNetPrivate(MNet* q) : q(q)
{
}
MNetPrivate::~MNetPrivate()
{
}

MNet::MNet()
 : d(new MNetPrivate(this))
{
	this->pack_header=NULL;
	this->data_head=NULL;
	this->recvDataPtr=NULL;
	this->pack_info=NULL;//=recvDataPtr

	this->spi=NULL;
	this->control=NULL;

	this->ackResult.crc=0;
	this->ackResult.res=0;

    pack_header_last.cmdType = CMD_REP_IO;
	pack_header_last.cmdOption = 0;
	pack_header_last.option=0;
	//
    iLastDataType = 0;
    iLastDataAddr = 0;
    iLastDataLength = 0;
}

int MNet::initialize(MSpi	 *spi,MControl *control,char *srv)
{
	if(spi==NULL ||control==NULL)
		return -1;

	this->spi=spi;
	this->control=control;
	memcpy(server,srv,strlen(srv));
	if ((host = gethostbyname(server)) == NULL)
	{
		perror("err gethostbyname\r\n");
		return -1;
	}

	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(sock <0) {
		printf("Error at socket():%d\n", errno);
		return -2 ;
	}

// 	int nZero = 0;
// 	setsockopt( sock, SOL_SOCKET, SO_SNDBUF, ( char * )&nZero, sizeof( nZero ) );


	addrSrv.sin_addr = *((struct in_addr *)host->h_addr);//inet_addr(server/*"192.168.1.200"*/);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(DefaultPort);
	//printf("server:%x\n",addrSrv.sin_addr.s_addr);

	protocol=new MNetProtocol();
	if(protocol==NULL)
	{
		close(sock);
		return -3;
	}

	return 0;
}


MNet::~MNet()
{
	shutdown(sock,2);
    delete d;
	delete protocol;
	freeComPtr();
	close(sock);
}

int MNet::freeComPtr(void)
{

	if(pack_header!=NULL)
	{

		pack_header_last = (*pack_header);
        free(pack_header);
        this->pack_header=NULL;
	}


	if(data_head!=NULL)
	{
        iLastDataType = data_head->type;
        iLastDataAddr = data_head->addr;
        iLastDataLength = data_head->datLen;
        //
        free(data_head);
        this->data_head=NULL;
	}


	if(recvDataPtr!=NULL)//==pack_info
	{
        free(recvDataPtr);
        this->recvDataPtr=NULL;

	}





	this->pack_info=NULL;

}

int MNet::recvPackHead(int sock)
{
	fd_set fd_socket;		// 所有可用套节字集合
	fd_set fd_read;
	int headSize;
	int ret,i,j;
	unsigned char *head_buff;

	headSize=sizeof(pack_header_t);
	head_buff=(unsigned char *)malloc(headSize+1);//why add 1?

	if(head_buff==NULL)
	{
		ackResult=protocol->setResult(ACK_ERR_MALLOC,0);
		protocol->status=PACK_ACK;
		return -2;
	}

	memset(head_buff,0,headSize+1);

head_again:
	FD_ZERO(&fd_socket);
	FD_SET(this->sock, &fd_socket);

	fd_read = fd_socket;


	ret = ::select(sock+1, &fd_read, NULL, NULL, NULL); // 判断是否有新数据
	if (ret < 0){
		errlast=errno;
		return ERR_NET_STATUS;
	}

	int nrecv = ::recv(sock, head_buff,headSize, 0);
	if (nrecv > 0 )	// （2）可读
	{
        gettimeofday(&m_recv_time,NULL);
		int now_recv = nrecv ;
head_recv_again:
		while(now_recv<headSize)
		{
			nrecv = ::recv(sock, &head_buff[now_recv],headSize-now_recv, 0);
			if (nrecv < 0){
				errlast=errno;
				freeComPtr();
				dbg0_d("err:head");
				if(head_buff!=NULL)
				{
                    free(head_buff);
                    head_buff=NULL;
				}

				ackResult=protocol->setResult(ACK_ERR_PACK_HEADER,__LINE__);
				protocol->status=PACK_ACK;
				return ERR_NET_STATUS;
			}
			now_recv += nrecv ;
		}

head_check_again:
		if(head_buff[0]!=PROTOCOL_VER_MIN)//check head info
		{
		  i=0;
		  for(i=0;i<headSize;i++)
		  {
		    if(head_buff[i]==PROTOCOL_VER_MIN)
		      break;
		  }

		  if(i>=headSize)
		  {
		    now_recv=0;
		    goto head_again;
		  }
		  else
		  {
		    //printf("remove\n");
		    now_recv=headSize-i;
		    for(j=0;j<now_recv;j++,i++)
		    {
		      head_buff[j]=head_buff[i];
		    }
		    goto head_recv_again;
		  }
		}

		pack_header=protocol->getHeaderFormNet(head_buff);
		if(pack_header==NULL)
		{
		    for(i=0;i<headSize-1;i++)
		     {
			head_buff[i]=head_buff[i+1];
		     }
		     head_buff[i]=0;
			goto head_check_again;
		}
		/*for(j=0;j<headSize;j++)
		{
		      //head_buff[j]=head_buff[i];
		      printf("%2x ",head_buff[j]);
		}
		printf("\n");*/
		if(pack_header->cmdType==CMD_REQ_IO)
		{
			protocol->status=PACK_RECIVE_DATE;
			if(recvDataPtr!=NULL)//用于准备接收数据
			{
                free(recvDataPtr);
                recvDataPtr=NULL;
			}

		}
		else if(pack_header->cmdType==CMD_QUERY_REQ_IO)
		{
		  if(pack_header->dataLen==0)
			protocol->status=PACK_PROCESS_QUERY;
		  else
		  {
			protocol->status=PACK_RECIVE_DATE;
			if(recvDataPtr!=NULL)//用于准备接收数据
			{
                free(recvDataPtr);
                recvDataPtr=NULL;
			}

		  }

		}
		else{
			if(pack_header->cmdType==CMD_REP_IO)
				ackResult=protocol->setResult(ACK_TEST,0);
			else
			{
				dbg0_d("err:head");
				ackResult=protocol->setResult(ACK_ERR_PACK_HEADER,__LINE__);
			}
			protocol->status=PACK_ACK;
		}

		if(pack_header->ver>PROTOCOL_VER_MAX || pack_header->ver<PROTOCOL_VER_MIN)
			ackResult=protocol->setResult(ACK_WAR_PROTOCOL_VER,0);

		if(head_buff!=NULL)
		{
            free(head_buff);
            head_buff=NULL;
		}


		return 0;
	}
	else{
		errlast=errno;
		freeComPtr();
		dbg0_d("err:recv head %d, lasterr:%d",nrecv,errlast);
		ackResult=protocol->setResult(ACK_ERR_PACK_HEADER,__LINE__);
		protocol->status=PACK_ACK;
		return ERR_NET_STATUS;
	}
}

int MNet::recvData(int sock)
{
	if(pack_header==NULL)
	{
		protocol->status=PACK_RECIVE_HEADER;
		return -1;
	}

	this->recvDataPtr=(unsigned char*)malloc(pack_header->dataLen+1);
	if(this->recvDataPtr==NULL)
	{
		ackResult=protocol->setResult(ACK_ERR_MALLOC,__LINE__);
		protocol->status=PACK_ACK;
		freeComPtr();
		return -2;
	}
	memset(this->recvDataPtr,0,pack_header->dataLen+1);
data_gain:
	int nrecv = ::recv(sock, recvDataPtr,pack_header->dataLen, 0);
	if (nrecv > 0 )	// （2）可读
	{
		int now_recv = nrecv ;
data_recive_gain:
		while(now_recv<pack_header->dataLen)
		{
			nrecv = ::recv(sock, &recvDataPtr[now_recv],pack_header->dataLen-now_recv, 0);
			if (nrecv < 0){
				errlast=errno;
				ackResult=protocol->setResult(ACK_ERR_NET_RECV,__LINE__);
				protocol->status=PACK_ACK;
				freeComPtr();//已释放recvDataPtr
				return ERR_NET_STATUS;
			}
			now_recv += nrecv ;
		}
		if(recvDataPtr[0]!=PROTOCOL_RW_READ && recvDataPtr[0]!=PROTOCOL_RW_WRITE)//find head
		{
		  int i=0,j=0;
		  for(i=0;i<pack_header->dataLen;i++)
		  {
		    if(recvDataPtr[i]==PROTOCOL_RW_READ ||recvDataPtr[0]==PROTOCOL_RW_WRITE)
		      break;
		  }

		  if(i>=pack_header->dataLen)
		  {
		    now_recv=0;
		    goto data_gain;
		  }
		  else
		  {
		   // printf("rmv\n");
		    now_recv=pack_header->dataLen-i;
		    for(j=0;j<now_recv;j++,i++)
		    {
		      recvDataPtr[j]=recvDataPtr[i];
		    }
		    goto data_recive_gain;
		  }
		}


		/*unsigned char *p=recvDataPtr;
		int i=0;
		for(i=0;i<sizeof(data_head_t);i++)
		{
		 printf("%2x ",p[i]);
		}
		printf("\r\n");*/

		this->data_head=protocol->getDataHeadFormNet( this->recvDataPtr);
		if(data_head==NULL)
		{

			dbg0_d("err:data header\r\n");
		    unsigned char *p=recvDataPtr;
		    int i=0;
		      for(i=0;i<sizeof(data_head_t);i++)
		      {
		      printf("%2x ",p[i]);
		      }
		      printf("\r\n");
			freeComPtr();
			ackResult=protocol->setResult(ACK_ERR_DATA_HEADER,__LINE__);
			protocol->status=PACK_ACK;

			return -3;
		}

		if(pack_header->cmdType==CMD_QUERY_REQ_IO)
		{


		  protocol->status=PACK_PROCESS_QUERY;
		  if(recvDataPtr!=NULL)//用于准备接收数据
		  {
              free(recvDataPtr);
              recvDataPtr=NULL;
		  }

		}
		else{

		    protocol->status=PACK_PROCESS_DATE;
		    this->pack_info=(pack_rw_info_t *)recvDataPtr;
		}

		return 0;
	}
	else
	{
		dbg0_d("err:recv %d\r\n",nrecv);
		errlast=errno;
		ackResult=protocol->setResult(ACK_ERR_NET_RECV,__LINE__);
		protocol->status=PACK_ACK;
		freeComPtr();//已释放recvDataPtr
		return ERR_NET_STATUS;
	}
}

int MNet::rwFpgaReg(void)
{
	int ret;
	unsigned int *pReg,*reg;
	unsigned long crc;
	unsigned char *pData;

	if(data_head==NULL || recvDataPtr==NULL ||pack_header==NULL || pack_info==NULL)
	{
		freeComPtr();
		ackResult=protocol->setResult(ACK_ERR_RUN_APP,__LINE__);
		protocol->status=PACK_ACK;
		return -1;
	}

	if(data_head->datLen<1)
	{
		freeComPtr();
		ackResult=protocol->setResult(ACK_ERR_DATA_LENGTH,__LINE__);
		protocol->status=PACK_ACK;
		return -2;
	}

	if(data_head->rw==PROTOCOL_RW_WRITE)
	{
		crc=protocol->crc32((unsigned char*)pack_info->data,data_head->datLen); //CRC校验处理
		if(crc!=data_head->crc) //写 crc32 出错
		{
			dbg0_d("err:crc local(%x),net(%x)",crc,data_head->crc);
			freeComPtr();
			ackResult=protocol->setResult(ACK_ERR_CRC_CHECK,__LINE__);
			protocol->status=PACK_ACK;
			return -3;
		}
	}

	//reg=(unsigned int *)malloc((data_head->datLen/4)*4+8);
    reg=(unsigned int *)malloc(FPGA_FLASH_SECTOR_SIZE*2+data_head->datLen
    +(FPGA_FLASH_SPI_REG_NUM+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_EXT_NUM)*BYTESPERUINT32);

//    reg=(unsigned int *)malloc((data_head->datLen/FPGA_FLASH_SECTOR_SIZE+1)*FPGA_FLASH_SECTOR_SIZE
//    +(FPGA_FLASH_SPI_REG_NUM+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_EXT_NUM)*BYTESPERUINT32);

   // reg=(unsigned int *)malloc((data_head->datLen/BYTESPERUINT32)*BYTESPERUINT32
    //+(FPGA_FLASH_SPI_REG_NUM+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_EXT_NUM)*BYTESPERUINT32);

	if(reg==NULL)
	{
		freeComPtr();
		ackResult=protocol->setResult(ACK_ERR_MALLOC,__LINE__);
		protocol->status=PACK_ACK;
		return -4;
	}

	pReg=reg;
	pReg=spi->packet_init_head(data_head->rw,data_head->type,data_head->addr/4,pReg);//why is 4?
    //adding the modflash function code
    if(FPGA_DEVICE_FLASH==data_head->type)//0x07
    {
        processModFlash(reg,pReg);
        //printf("FPGA_DEVICE_FLASH==data_head->type\n");
        freeComPtr();
        //printf("freeComPtr()\n");
        if(reg!=NULL)
        {
            printf("reg!=NULL\n");
            free(reg);
            reg=NULL;
        }
        //printf("free(reg)\n");

        return 0;
    }
	//dbg0_d("rw %c\n",data_head->rw==PROTOCOL_RW_WRITE?'w':'r');
	if(data_head->rw==PROTOCOL_RW_WRITE)
	{

// #ifdef __DEBUG_3_
// 		dbg3_p("write: %x,%x\r\n",data_head->type,data_head->addr);
// 		pData=(unsigned char* )pack_info->data;
// 		for(int i=0;i<data_head->datLen;i++)//转大端
// 		{
// 			dbg3_p("%x ",*pData);
// 			pData++;
// 			if(i%32==31)
// 				dbg3_p("\r\n");
// 		}
// 		dbg3_p("\r\n");
// #endif
		pData=(unsigned char* )pack_info->data;
		memcpy((unsigned char*)pReg,pData,data_head->datLen);
		protocol->status=PACK_ACK;
		ret=spi->fpga_spi_write(reg,(data_head->datLen/4)+2);
		if(ret<0)
		{
			free(reg);
			reg=NULL;
			freeComPtr();
			ackResult=protocol->setResult(ACK_ERR_SPI_WRITE,ret);
			return -5;
		}

		if(((pack_header->cmdOption>>24)&0xFF)!=0x5A)
		{
		    pReg=reg;
		    pReg=spi->packet_init_head(PROTOCOL_RW_READ,data_head->type,data_head->addr/4,pReg);
		    ret=spi->fpga_spi_read(reg,(data_head->datLen/4)+2);
		    if(ret<0)
		    {
			    free(reg);
			    reg=NULL;
			    freeComPtr();
			    ackResult=protocol->setResult(ACK_ERR_SPI_READ,ret);
			    return -6;
		    }

    // #ifdef __DEBUG_3_
    // 		dbg3_p("read:%x,%x\r\n",data_head->type,data_head->addr);
    // 		pData=(unsigned char* )(reg+2);
    // 		for(int i=0;i<data_head->datLen;i++)//转小端
    // 		{
    //
    // 			dbg3_p("%x ",*pData);
    // 			pData++;
    // 			if(i%32==31)
    // 				dbg3_p("\r\n");
    // 		}
    // 		dbg3_p("\r\n");
    // #endif

		    ackResult.crc=this->protocol->crc32((unsigned char*)&reg[2],data_head->datLen);
		    if(ackResult.crc!=data_head->crc){
			    ackResult.res=ACK_ERR_SPI_WRITE_CRC;
		    }
		}
	}
	else
	{


		spi->fpga_spi_read(reg,(data_head->datLen/4)+2);
		//dbg0_d("r %d,%d,%d\n",data_head->type,data_head->addr,data_head->datLen);

		int sendLen=sizeof(pack_header_t)+sizeof(data_head_t)+data_head->datLen;
		unsigned char *pSend=(unsigned char *)malloc(sendLen);
		if(pSend==NULL)
		{
			dbg0_d("err:malloc\n");
			free(reg);
			reg=NULL;
			freeComPtr();
			ackResult=protocol->setResult(ACK_ERR_MALLOC,__LINE__);
			protocol->status=PACK_ACK;
			return -7;
		}

// #ifdef __DEBUG_3_
// 		dbg3_p("read:%x,%x\r\n",data_head->type,data_head->addr);
// 		pReg=reg+2;
// 		pData=(unsigned char*)pReg;
// 		for(int i=0;i<data_head->datLen;i++)//转小端
// 		{
// 			dbg3_p("%x ",*pData);
// 			pData++;
// 			if(i%32==31)
// 				dbg3_p("\r\n");
// 		}
// 		dbg3_p("\r\n");
// #endif

		pReg=reg+2;
		data_head->crc=this->protocol->crc32((unsigned char*)pReg,data_head->datLen);
		data_head->chkSum=this->protocol->checkSum((unsigned char * )data_head,sizeof(data_head_t)-4);
		pData=(unsigned char* )(pSend+sizeof(pack_header_t)+sizeof(data_head_t));

		pack_header->dataLen=sizeof(data_head_t)+data_head->datLen;
		pack_header->chkSum=protocol->checkSum((unsigned char *)pack_header,sizeof(pack_header_t)-4);

		memcpy( (char*)pSend, (char*)pack_header,sizeof(pack_header_t));
		memcpy( (char*)(pSend+sizeof(pack_header_t)), (char*)data_head,sizeof(data_head_t));
		memcpy(pData,(unsigned char*)pReg,data_head->datLen);
		ret = send(sock, (char*)pSend,sendLen, 0) ;
		/*for(int i=0;i<sendLen;i++)
		{
		 if(i%16==0)
		   printf("\n");
		 printf("%2x ",pSend[i]);

		}
		 printf("\r\n");*/
		 if(pSend!=NULL)
		 {
            free(pSend);
            pSend=NULL;
		 }

		if(ret<0)
		{
			dbg0_d("sock send err:");
			freeComPtr();
			if(reg!=NULL)
				free(reg);
			reg=NULL;
			ackResult=protocol->setResult(ACK_ERR_NET_SEND,ret);
			errlast=errno;
			protocol->status=PACK_ACK;
			return ERR_NET_STATUS;
		}

		protocol->status=PACK_RECIVE_HEADER;

	}

	freeComPtr();
	if(reg!=NULL)
	{
        free(reg);
        reg=NULL;
	}

}
#define DATA_BUFF_LEN	0x10020


void MNet::notify_callback(unsigned long proc,unsigned long res,unsigned long data)
{

}


int MNet::processData(void)
{
	int ret;
	if(data_head==NULL || recvDataPtr==NULL ||pack_header==NULL  || pack_info==NULL)
	{
		freeComPtr();
		ackResult=protocol->setResult(ACK_ERR_RUN_APP,__LINE__);
		protocol->status=PACK_ACK;
		return -1;
	}

	switch(this->data_head->type)
	{
		case DATA_TYPE_CONTROL_MOVE:
			if(this->data_head->rw==PROTOCOL_RW_WRITE)
			{
				unsigned int  *val=(unsigned int  *)pack_info->data;
				this->control->set_reg(this->data_head->addr,*val);
			}
			else
			{
				this->control->get_reg(this->data_head->addr,(unsigned int  *)pack_info->data);
			}
			break;
		default:

			return rwFpgaReg();
			break;
	}

	return 0;
}

int MNet::processAck(bool frCom){
	int len;
	unsigned long *pdat;

	len=sizeof(pack_header_t)+sizeof(data_head_t)+4;// 4-->4 bytes-->32 bit data



	if(pack_header==NULL)
	{
		pack_header=(pack_header_t*)malloc(sizeof(pack_header_t));
		pack_header->cmdType=pack_header_last.cmdType;
		pack_header->cmdOption=pack_header_last.cmdOption;
		pack_header->option=pack_header_last.option;
		dbg4("pack_header->cmdType=CMD_REP_IO\n\r");
	}

	if(pack_header!=NULL)
	{
		pack_header->ver=PROTOCOL_VER_MIN;
		pack_header->dataLen=sizeof(data_head_t)+4;//data head len+ data len
// 		if(pack_header->cmdType!=CMD_NOTIFY_IO || pack_header->cmdType!=CMD_QUERY_REP_IO)
// 		{
//            pack_header->cmdType=CMD_REP_IO;
//            dbg4("pack_header->cmdType=CMD_REP_IO\n\r");
// 		}
		pack_header->chkSum=protocol->checkSum((unsigned char*)pack_header,sizeof(pack_header_t)-4);//4=sizeof(pack_header->chkSum)
	}


	dbg4("res:%x\r\n",ackResult.res);
	if(ackResult.res!=0)
		dbg4("res:0x%x,crc:0x%x\r\n",ackResult.res,ackResult.crc);

	if((this->pack_info==NULL) ||(sizeof(pack_info)<(sizeof(data_head_t)+4)))
	{
// 		if(recvDataPtr!=NULL)//==pack_info
// 			free(recvDataPtr);
// 		recvDataPtr=NULL;
// 		if(this->pack_info!=NULL)
// 			free(pack_info);

		pack_info=(pack_rw_info_t*)malloc(sizeof(data_head_t)+4);//4?????
		//recvDataPtr=(unsigned char*)pack_info;
	}


    pack_info->dataHead.type = iLastDataType;
    pack_info->dataHead.addr = iLastDataAddr;
    //pack_info->dataHead.datLen = iLastDataLength;
    //
    dbg4("type:%d,addr:0x%x,datLen:%d\r\n",iLastDataType,iLastDataAddr,iLastDataLength);
	pack_info->dataHead.res=ackResult.res;
	pdat=(unsigned long *)pack_info->data;
	*pdat=ackResult.crc;
	pack_info->dataHead.datLen=4;
	pack_info->dataHead.crc=protocol->crc32((unsigned char *)pack_info->data,pack_info->dataHead.datLen);
	pack_info->dataHead.chkSum=protocol->checkSum((unsigned char *)&pack_info->dataHead,sizeof(data_head_t)-4);

	unsigned char *pSend=(unsigned char *)malloc(len);

//    gettimeofday(&m_send_time,NULL);
//    dbg4("recv time is: %lds %ldus\n", m_recv_time.tv_sec,m_recv_time.tv_usec);
//    dbg4("send time is: %lds %ldus\n",m_send_time.tv_sec,m_send_time.tv_usec);
//    dbg4("internal time is: %lds %ldus\n\n",m_send_time.tv_sec-m_recv_time.tv_sec,m_send_time.tv_usec-m_recv_time.tv_usec);

	if(pSend!=NULL)
	{
        //dbg4("pSend!=NULL\r\n");
		memcpy(pSend, (char*)this->pack_header,sizeof(pack_header_t));
		memcpy(pSend+sizeof(pack_header_t),pack_info,sizeof(data_head_t)+4);
		len = send(sock, (unsigned char*)pSend,sizeof(pack_header_t)+sizeof(data_head_t)+4 , 0) ;//sizeof(data_head_t)+4;pack_header->dataLen
		if(pSend!=NULL)
		{
            dbg4("pSend!=NULL\r\n,len %d",len);
            free(pSend);
            pSend=NULL;
		}

	}
	else
	{
        //dbg4("pSend==NULL\r\n");
		len = send(sock, (unsigned char*)pack_header,sizeof(pack_header_t) , 0) ;
		if(len>=0)
		{
            dbg4("pSend==NULL len %d\r\n",len);
            len = send(sock, (unsigned char*)pack_info,+sizeof(data_head_t)+4 , 0) ;
		}

	}
// 	if(frCom==false)
// 	{
//
// 	}

	if(len<0)
	{
		dbg0_d("sock send err \r\n");
		errlast=errno;
		ackResult=protocol->setResult(ACK_SUCCESS,0);
		protocol->status=PACK_RECIVE_HEADER;
		if(frCom==true)
		{
			freeComPtr();
		}
		return ERR_NET_STATUS;
	}
	if(frCom==true)
	{
		freeComPtr();
	}
	ackResult=protocol->setResult(ACK_SUCCESS,0);
	protocol->status=PACK_RECIVE_HEADER;

}

int sock_pause_key=-1;
bool key_pause=false;

int pause_key(int socketn)
{
	typedef struct pack_key_info
	{
		data_head_t dataHead;
		unsigned long data0;
		unsigned long data1;
	}__attribute__((packed)) pack_key_info_t;

	typedef struct pack_key_pkt
	{
		pack_header_t head;
		pack_key_info_t key;
	}__attribute__((packed)) pack_key_pkt_t;

	pack_key_pkt_t pkt={
	  {
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_key_info_t),		// 数据长度
	CMD_NOTIFY_IO,		// 命令 cmd_type_t
	0,
	OPTION_NOTIFY_IO_KEY_PAUSE,		// 可选项
	0		// 校验和
	},
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_KEY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			8, // 1字节长度
			0,  // 数据
			ACK_ERR_NO_MOVE_STATUS, // 结果
			0, // 校验和
		},
		OPTION_NOTIFY_IO_KEY_PAUSE,
		OPTION_NOTIFY_IO_KEY_PAUSE,
	},
	};

	/*pack_header_t head={
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_data_info_t),		// 数据长度
	CMD_NOTIFY_IO,		// 命令 cmd_type_t
	0,
	OPTION_NOTIFY_IO_KEY_PAUSE,		// 可选项
	0		// 校验和
	};*/
	MNetProtocol *protocol=new MNetProtocol();
	pkt.head.chkSum=protocol->checkSum((unsigned char*)&pkt.head,sizeof(pack_header_t)-4);

	MSpi	 *spi=new MSpi();
	int ret=spi->initialize();


	/*pack_data_info_t key_data=
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_KEY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			8, // 1字节长度
			0,  // 数据
			ACK_ERR_NO_MOVE_STATUS, // 结果
			0, // 校验和
		},
		OPTION_NOTIFY_IO_KEY_PAUSE,
		OPTION_NOTIFY_IO_KEY_PAUSE,
	};*/
	if(ret>=0)
	{
		unsigned int buff[8];
		unsigned int *pReg=buff;
		pReg=spi->packet_init_head(PROTOCOL_RW_READ,FPGA_TYPE_MOVE,0x02,buff);	//0x02 内部状态寄存器
		ret=spi->fpga_spi_read(buff,4);//2+2
		if(ret>=0)
		{
			pkt.key.dataHead.res=ACK_SUCCESS;
			pkt.key.data0=buff[2];
			pkt.key.data1=buff[3];
			//printf("spi %x,%x\r\n",key_data.data0,key_data.data1);
		}

	}

	pkt.key.dataHead.crc=protocol->crc32((unsigned char*)&pkt.key.data0,8);
	pkt.key.dataHead.chkSum=protocol->checkSum((unsigned char*)&pkt.key.dataHead,sizeof(data_head_t)-4);

	//send(sock_pause_key, (unsigned char*)&head,sizeof(pack_header_t) , 0) ;
	ret=send(socketn, (unsigned char*)&pkt,sizeof(pack_key_pkt_t) , 0) ;
	delete spi;
	delete protocol;
	if(ret<0)
	{
	    return ERR_NET_STATUS;
	}
	return 0;
}

void MNet::getVoltage(unsigned long *v1,unsigned long *v2)
{
	int n=getIioDeviceValue((char *)CLIENT_VOLTAGE_DIR,(char *)FILE_VOLTAGE_BAT);
	/*								---|		Vbat
	 * 		n*1.8*(R1+R2)			  [ ]		R2=100k
	 *   Vx=------------			   |------	Vadc
	 * 		4096*R1					  [ ]		R1=10k
	 *                       		---|		GND
	 */
	*v1=(float)n*0.004834*100;

	/*	---|		V24
	 * 	  [ ]		R2=100k
	 * 	   |------	Vadc
	 * 	  [ ]		R1=6.8k
	 * 	---|		GND
	 */

	n=getIioDeviceValue((char *)CLIENT_VOLTAGE_DIR,(char *)FILE_VOLTAGE_24);
	*v2=(float)n*0.006902*100;
}

int reportMultiData(int sock,MNet *net,unsigned short cmdType)
{
	typedef struct pack_multi_data_info
	{
		data_head_t dataHead;
		unsigned long moveStatus[4];//0x00-0x03         //0x380
		unsigned long probeStatus[8];//0x04-0x0b
		unsigned long voltge[2];
	}__attribute__((packed)) pack_multi_data_info_t;


	typedef struct pack_multi_data_pkt
	{
	  pack_header_t head;
	  pack_multi_data_info_t muti;
	}__attribute__((packed)) pack_multi_data_pkt_t;


	pack_multi_data_pkt_t pkt={
	  {
	  PROTOCOL_VER_MIN,		// 版本
	  sizeof(pack_multi_data_info_t),		// 数据长度
	  cmdType,//CMD_NOTIFY_IO,		// 命令 cmd_type_t
	  0,
	  OPTION_UPDATE_MULTI_DATA,		// 可选项
	  0		// 校验和
	  },
	  {
	    {
		    PROTOCOL_RW_READ,	// 读写标志
		    DATA_TYPE_KEY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
		    0, // 地址 1字节地址
		    8, // 1字节长度
		    0,  // 数据
		    ACK_ERR_NO_MOVE_STATUS, // 结果
		    0, // 校验和
	    },
	    0,
	  },

	};

	/*pack_header_t head={
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_multi_data_info_t),		// 数据长度
	cmdType,//CMD_NOTIFY_IO,		// 命令 cmd_type_t
	0,
	OPTION_UPDATE_MULTI_DATA,		// 可选项
	0		// 校验和
	};*/
	MNetProtocol *protocol=new MNetProtocol();
	pkt.head.chkSum=protocol->checkSum((unsigned char*)&pkt.head,sizeof(pack_header_t)-4);

	MSpi	 *spi=new MSpi();
	int ret=spi->initialize();


/*	pack_multi_data_info_t multi_data=
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_KEY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			8, // 1字节长度
			0,  // 数据
			ACK_ERR_NO_MOVE_STATUS, // 结果
			0, // 校验和
		},
		OPTION_NOTIFY_IO_KEY_PAUSE,
		OPTION_NOTIFY_IO_KEY_PAUSE,
	};
*/
	unsigned int buff[32];
	unsigned int *pReg=buff;
	pReg=spi->packet_init_head(PROTOCOL_RW_READ,FPGA_TYPE_MOVE,0x00,buff);	//0x380-->0x00 Move内部状态寄存器
	ret=spi->fpga_spi_read(buff,6);//2+4
	if(ret>=0)
	{
		pkt.muti.dataHead.res=ACK_SUCCESS;
		memcpy((char*)pkt.muti.moveStatus,(char*)&buff[2],16);
	}

	pReg=spi->packet_init_head(PROTOCOL_RW_READ,FPGA_TYPE_PROBE,0x04,buff);	//0x380 内部状态寄存器
    ret=spi->fpga_spi_read(buff,10);//2S+8
    if(ret>=0)
	{
		pkt.muti.dataHead.res=ACK_SUCCESS;
		memcpy((char*)pkt.muti.probeStatus,(char*)&buff[2],32);
	}
	net->getVoltage(&pkt.muti.voltge[0],&pkt.muti.voltge[1]);
 ///////////////////////////////////

    uint8_t *ptrdata=(uint8_t*)buff;
    memcpy(ptrdata,(char*)pkt.muti.moveStatus,4*4);
    ptrdata=ptrdata+4*4;
    memcpy(ptrdata,(char*)pkt.muti.probeStatus,4*8);
    ptrdata=ptrdata+4*8;
    memcpy(ptrdata,(char*)pkt.muti.voltge,4*2);

    ptrdata=(uint8_t*)buff;
    pkt.muti.dataHead.crc=protocol->crc32(ptrdata,(4+8+2)*4);

   // pkt.muti.dataHead.crc=protocol->crc32((unsigned char*)&pkt.muti.moveStatus,(8+4+2)*4);
/*
    printf("Print MultiData,and the CRC(moveStatus,probeStatus,voltge) is %d\n",pkt.muti.dataHead.crc);
    ptrdata=(uint8_t*)buff;
    for(int i=0;i>56;i++)
    {
        printf("0x%0x",ptrdata[i]);
        if(i%15==0) printf("\n");
    }
*/
////////////////////////////////////////////
    //pkt.muti.dataHead.crc=protocol->crc32((unsigned char*)&pkt.muti.moveStatus,8);
	pkt.muti.dataHead.chkSum=protocol->checkSum((unsigned char*)&pkt.muti.dataHead,sizeof(data_head_t)-4);
	//dbg0("%s\n",__FUNCTION__);
	//send(sock, (unsigned char*)&head,sizeof(pack_header_t) , 0) ;
	ret=send(sock, (unsigned char*)&pkt,sizeof(pack_multi_data_pkt_t) , 0) ;
	delete spi;
	delete protocol;
	if(ret<0)
	{
	  return ERR_NET_STATUS;
	}

	return 0;
}

void sigroutine(int dunno)
{
  if(sock_pause_key<0)
    return;
  printf("sigroutine\n");
	if(SIGUSR2==dunno )
	{
		//key_pause=true;
		pause_key(sock_pause_key);
	}
	else if(SIGALRM==dunno)
	{
		//if(g_net!=NULL)
		//    reportMultiData(sock_pause_key,g_net,CMD_NOTIFY_IO);
	}
}
int MNet::reportSn(void)
{
	typedef struct pack_sn_info
	{
		data_head_t dataHead;
		unsigned char data[16];
	}__attribute__((packed)) pack_sn_info_t;

	typedef struct pack_sn_pkt
	{
		pack_header_t head;
		pack_sn_info_t sn;
	}__attribute__((packed)) pack_sn_pkt_t;


	pack_sn_pkt_t pkt={
	  {
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_sn_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_SN,		// 可选项
	0		// 校验和
	},
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_SN_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			16, // 1字节长度
			0,  // 数据
//			0, // 结果
			ACK_ERR_UNKNOW_SN, // 结果
			0, // 校验和
		},
		0,
	},
	};

	/*pack_header_t head={
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_sn_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_SN,		// 可选项
	0		// 校验和
	};*/

	MNetProtocol *protocol=new MNetProtocol();
	pkt.head.chkSum=protocol->checkSum((unsigned char*)&pkt.head,sizeof(pack_header_t)-4);

	/*pack_sn_info_t sn_data=
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_SN_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			16, // 1字节长度
			0,  // 数据
			ACK_ERR_UNKNOW_SN, // 结果
			0, // 校验和
		},
		0,
	};*/

	int fd=open(CLIENT_SN_FILE,O_RDONLY);
	if(fd>=0)
	{
		unsigned char buff[16];
		int ret=read(fd,buff,16);
		if(ret==16)
		{
			memcpy(pkt.sn.data,buff,16);
			pkt.sn.dataHead.res=ACK_SUCCESS;
		}
		close(fd);

	}

	pkt.sn.dataHead.crc=protocol->crc32((unsigned char*)&pkt.sn.data,16);
	pkt.sn.dataHead.chkSum=protocol->checkSum((unsigned char*)&pkt.sn.dataHead,sizeof(data_head_t)-4);

	//send(this->sock, (unsigned char*)&head,sizeof(pack_header_t) , 0) ;
	int ret=send(this->sock, (unsigned char*)&pkt,sizeof(pack_sn_pkt_t) , 0) ;
	if(ret<0)
	{
	    return ERR_NET_STATUS;
	}
	return 0;
}

int MNet::getIioDeviceValue(char *dir,char *name)
{
	int len=strlen(dir)+strlen(name);
	char *fname=(char *)malloc(len+1);
	char *p=fname;

	memcpy(fname,dir,strlen(dir));
	p=p+strlen(dir);
	memcpy(p,name,strlen(name));
	p=p+strlen(name);
	*p=0;

	int fd=open(fname,O_RDONLY);
    if(fname!=NULL)
    {
        free(fname);
        fname=NULL;
    }

	if(fd>=0)
	{
		char buff[16];
		len=lseek(fd,0,SEEK_END);
		lseek(fd,0,SEEK_SET);
		memset(buff,0,16);
		len=len>15?15:len;
		int ret=read(fd,buff,len);
		close(fd);

		if(ret>=0)
		{
			return atoi(buff);
		}

	}
	//dbg0("err:%d,%d\n",fd,len);
	return 0;
}

int MNet::reportHp203Value(void)
{
	typedef struct pack_hp203_info
	{
		data_head_t dataHead;
		int data[3];
	}__attribute__((packed)) pack_hp203_info_t;

	typedef struct pack_hp203_pkt
	{
		pack_header_t head;
		pack_hp203_info_t hp203;
	}__attribute__((packed)) pack_hp203_pkt_t;
#undef PACK_INFO_LENGTH
#define PACK_INFO_LENGTH	12


	pack_hp203_pkt_t pkt={
	  {
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_hp203_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_INTENSITY,		// 可选项
	0		// 校验和
	},
		{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_INTENSITY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			PACK_INFO_LENGTH, // 1字节长度
			0,  // 数据
			ACK_SUCCESS, // 结果
			0, // 校验和
		},
		0,
	},
	};

	/*pack_header_t head={
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_sn_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_INTENSITY,		// 可选项
	0		// 校验和
	};*/
	MNetProtocol *protocol=new MNetProtocol();
	pkt.head.chkSum=protocol->checkSum((unsigned char*)&pkt.head,sizeof(pack_header_t)-4);

	/*pack_sn_info_t hp203_data=
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_INTENSITY_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			PACK_INFO_LENGTH, // 1字节长度
			0,  // 数据
			ACK_SUCCESS, // 结果
			0, // 校验和
		},
		0,
	};*/


	pkt.hp203.data[0]=getIioDeviceValue((char *)CLIENT_HP203_DIR,(char *)FILE_INTENSITY);
	pkt.hp203.data[1]=getIioDeviceValue((char *)CLIENT_HP203_DIR,(char *)FILE_PROXIMITY);
	pkt.hp203.data[2]=getIioDeviceValue((char *)CLIENT_HP203_DIR,(char *)FILE_TEMP);


	pkt.hp203.dataHead.crc=protocol->crc32((unsigned char*)&pkt.hp203.data,PACK_INFO_LENGTH);
	pkt.hp203.dataHead.chkSum=protocol->checkSum((unsigned char*)&pkt.hp203.dataHead,sizeof(data_head_t)-4);

	//send(this->sock, (unsigned char*)&head,sizeof(pack_header_t) , 0) ;
	int ret = send(this->sock, (unsigned char*)&pkt,sizeof(pack_hp203_pkt_t) , 0) ;

	if(ret<0)
	{
	    return ERR_NET_STATUS;
	}
	return 0;
}

int MNet::reportVoltage(void)
{
	typedef struct pack_voltage_info
	{
		data_head_t dataHead;
		unsigned long data[2];
	}__attribute__((packed)) pack_voltage_info_t;

	typedef struct pack_voltage_pkt
	{
		pack_header_t head;
		pack_voltage_info_t voltage;
	}__attribute__((packed)) pack_voltage_pkt_t;

#undef PACK_INFO_LENGTH
#define PACK_INFO_LENGTH	8


	pack_voltage_pkt_t pkt={
	  {
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_voltage_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_VOLTAGE,		// 可选项
	0		// 校验和
	},
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_VOLTAGE_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			PACK_INFO_LENGTH, // 1字节长度
			0,  // 数据
			ACK_SUCCESS, // 结果
			0, // 校验和
		},
		0,
	},
	};

	/*pack_header_t head={
	PROTOCOL_VER_MIN,		// 版本
	sizeof(pack_voltage_info_t),		// 数据长度
	CMD_QUERY_REP_IO,		// 命令 cmd_type_t
	0,
	OPTION_QUERY_REP_IO_VOLTAGE,		// 可选项
	0		// 校验和
	};*/

	MNetProtocol *protocol=new MNetProtocol();
	pkt.head.chkSum=protocol->checkSum((unsigned char*)&pkt.head,sizeof(pack_header_t)-4);

	/*pack_voltage_info_t hp203_data=
	{
		{
			PROTOCOL_RW_READ,	// 读写标志
			DATA_TYPE_VOLTAGE_REPORT,// 类型 // 0xff 循环位置  0xfe 在线升级 其他类型参考文档
			0, // 地址 1字节地址
			PACK_INFO_LENGTH, // 1字节长度
			0,  // 数据
			ACK_SUCCESS, // 结果
			0, // 校验和
		},
		0,
	};*/


	int n=getIioDeviceValue((char *)CLIENT_VOLTAGE_DIR,(char *)FILE_VOLTAGE_BAT);
	/*								---|		Vbat
	 * 		n*1.8*(R1+R2)			  [ ]		R2=100k
	 *   Vx=------------			   |------	Vadc
	 * 		4096*R1					  [ ]		R1=10k
	 *                       		---|		GND
	 */
	pkt.voltage.data[0]=(n*0.004834)*100;

	/*	---|		V24
	 * 	  [ ]		R2=100k
	 * 	   |------	Vadc
	 * 	  [ ]		R1=6.8k
	 * 	---|		GND
	 */

	n=getIioDeviceValue((char *)CLIENT_VOLTAGE_DIR,(char *)FILE_VOLTAGE_24);
	pkt.voltage.data[1]=(n*0.006902)*100;

	//dbg0("voltage:%d,%d\r\n",pkt.voltage.data[0],pkt.voltage.data[1]);
	pkt.voltage.dataHead.crc=protocol->crc32((unsigned char*)&pkt.voltage.data,PACK_INFO_LENGTH);
	pkt.voltage.dataHead.chkSum=protocol->checkSum((unsigned char*)&pkt.voltage.dataHead,sizeof(data_head_t)-4);

	//send(this->sock, (unsigned char*)&head,sizeof(pack_header_t) , 0) ;
	int ret=send(this->sock, (unsigned char*)&pkt,sizeof(pack_voltage_pkt_t) , 0) ;
	if(ret<0)
	{
	    return ERR_NET_STATUS;
	}
	return 0;
}
int MNet::stop()
{
	//runStop=true;
	//change_status(CONTROL_EXIT);
	exit=true;
	return 0;
}
void MNet::run()
{
  	const char chOpt=1;
	int keepalive = 1; // 开启keepalive属性
	int keepidle = 1; // 如该连接在3秒内没有任何数据往来,则进行探测
	int keepinterval = 1; // 探测时发包的时间间隔为5 秒
	int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	struct itimerval value,ovalue;
	int erro;
	exit=false;

	while(-1 == connect(sock, (sockaddr*)&addrSrv, sizeof(sockaddr))){
		//printf("connect %s server fail!\n",server);
		goto net_work_err;
		if(sigexit && exit)
			return;
	}
	printf("connect %s server success!\n",server);


	signal(SIGUSR2, sigroutine); //* 下面设置三个信号的处理方法
	/*signal(SIGALRM, sigroutine);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 2*1000*1000;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 2*1000*1000;
	setitimer(ITIMER_REAL, &value, &ovalue);*/

	sock_pause_key=sock;

	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(chOpt));


	if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ))==-1)
	{
		dbg0("err:keepalive");
	}
	if(setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ))==-1)
	{
		dbg0("err:keepidle");
	}
	if(setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ))==-1)
	{
		dbg0("err:keepinterval");
	}
	if(setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ))==-1)
	{
		dbg0("err:keepcount");
	}


	printf("connect server success!\n");

	while(!sigexit && !exit)
	{
		errlast=0;
// 		if(key_pause==true)
// 		{
// 			pause_key();
// 			key_pause=false;
// 		}

		//dbg3_p("net status %x %x\r\n",pthread_self(), protocol->status);
		switch(protocol->status)
		{
			case PACK_RECIVE_HEADER:
				erro=recvPackHead(sock);
				if(erro==ERR_NET_STATUS)
				{
					//freeComPtr();
					goto net_work_err;
				}
				else if(erro!=0)
				{
					if(protocol->status==PACK_RECIVE_DATE || protocol->status==PACK_PROCESS_DATE)//都是错误状态
					{
						ackResult=protocol->setResult(ACK_ERR_RUN_APP,__LINE__);
						protocol->status=PACK_ACK;
					}
				}
				break;
			case PACK_RECIVE_DATE:
				erro=recvData(sock);
				if(erro==ERR_NET_STATUS)
				{
					//freeComPtr();
					goto net_work_err;
				}
				else if(erro!=0)
				{
					if(protocol->status==PACK_RECIVE_DATE || protocol->status==PACK_PROCESS_DATE)//自身，或PACK_PROCESS_DATE，都是错误状态
					{
						ackResult=protocol->setResult(ACK_ERR_RUN_APP,__LINE__);
						protocol->status=PACK_ACK;
					}
				}
				break;
			case PACK_PROCESS_DATE:
				erro=processData();
				if(erro==ERR_NET_STATUS)
				{
					//freeComPtr();
					goto net_work_err;
				}
				else if(erro!=0)
				{
					if(protocol->status==PACK_PROCESS_DATE ||protocol->status==PACK_RECIVE_DATE)//自身，或PACK_RECIVE_DATE，都是错误状态
					{
						ackResult=protocol->setResult(ACK_ERR_RUN_APP,__LINE__);
						protocol->status=PACK_ACK;
					}
				}
				break;
			case PACK_PROCESS_QUERY:
			{
				erro=processQuery();
				if(erro==ERR_NET_STATUS)
				{
					//freeComPtr();
					goto net_work_err;
				}
				protocol->status=PACK_RECIVE_HEADER;
			}break;
			case PACK_ACK:
			{
				erro=processAck();
				if(erro==ERR_NET_STATUS)
				{
					//freeComPtr();
					goto net_work_err;
				}

			}
				break;
		}
	}

net_work_err:
sock_pause_key=-1;
	freeComPtr();
}


unsigned long MNet::randomex(unsigned long modnum)
{
	static ulong ul_random_num = 1;
	ul_random_num=ul_random_num*1103515245+12345;
	return (ul_random_num/65536)%modnum;
}
int MNet::processQuery(void)
{
	switch(this->pack_header->option)
	{
		case OPTION_QUERY_REP_IO_SN:
		{
			return this->reportSn();
		}
		break;
		case OPTION_QUERY_REP_IO_INTENSITY:
		{
			return this->reportHp203Value();
		}
		break;
		case OPTION_QUERY_REP_IO_VOLTAGE:
		{
			return this->reportVoltage();
		}break;
		case OPTION_UPDATE_MULTI_DATA:
		{
			 return reportMultiData(this->sock,this,CMD_QUERY_REP_IO);
		}
		break;
	}

}



///New Function Add at 20160424
int MNet::processModFlash(unsigned int *reg,unsigned int *pReg)
{

	if(PROTOCOL_RW_WRITE==data_head->rw)
	{
        ModFlashWrite(reg);

	}
	else
	{
       ModFlashRead(reg,pReg);

	}

    return 0;
}

/**
* @brief
* @return   FLASH_OPERATION_SUCCESS(0):success
            FLASH_TIMEOUT_ERROR(-1) :timeout error
            FLASH_SPI_READ_ERROR(-1) :fpga read error
*
*/
int MNet::readStatus(uint32_t *reg,uint8_t *ptr,uint8_t rw)
{
    uint8_t delay=100;
    uint8_t fpga_read_num=5;
    int ret=0;
    uint32_t status=0;
    ///reaq Busy Status
    dbg4("Checkout Busy Status\r\n");
    *ptr++=FPGA_RW_READ;
    *ptr++=FPGA_DEVICE_FLASH;
    *ptr++=00;
    *ptr++=FPGA_FLASH_READ_STATUS_REG;

    *ptr++ =FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ =FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ =FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ =FPGA_FLASH_SPI_OPE_DUM;

    ////////
//    for(int i=0;i<3;i++)
//    {
//        printf("0x%x",reg[i]);
//
//    }


    ////

    while(delay)
    {
        ret=spi->fpga_spi_read(reg,3);
        if(ret<0) //spi read error
        {
            if(fpga_read_num>0)
            {
                fpga_read_num--;
                continue;
            }
            else
            {
                break;//exit the while;
            }
        }


        switch(rw)
        {
        /**
        * RD_Ready(d6--read,0--noread Busy(B3--busy,0--nobusy)
        * [31-16]                     [15-0]
        */
            case FPGA_FLASH_CHECK_WRITE_STATUS://write data
            {   //printf("write data\r\n");
//                printf("Checkout Busy Status,0x%x\r\n",reg[2]);
                status=reg[2]&0xFF000000;
                if(0x0==status)//b3 is busy
                {
                    //printf("Read Busy Status Success\r\n");
                    status=1;//exit while

                }
                break;
            }

            case FPGA_FLASH_CHECK_REQUEST_STATUS://read request
            {   //printf("\r\n");
//                printf("Checkout read request\r\n");
                status=reg[2]&0x0FF00FF00;
                if(0x0==status)
                {
//                    printf("Read notBusy Status and NonReadable Status Success\r\n");
                    status=1;

                }
                break;

            }
            case FPGA_FLASH_CHECK_READ_STATUS://read data
            {   //printf("\r\n");
                printf("Checkout read data\r\n");
                status=reg[2]&0x0FF00;
                if(0xd60000==status)//d6 is readable
                {
                    //printf("Read Readable Statu Success\r\n");
                    status=1;

                }
                break;
            }
            case FPGA_FLASH_CHECK_REGISTER_STATUS://read flash register
            {   //printf("read data\r\n");
                status=reg[2]&0x0FF0000;
                dbg4_p("readStatus Flash Register %x \r\n",status>>8);
                status=1;
                break;
            }
            case FPGA_FLASH_CHECK_FPGA_STATUS:
            {
                status=reg[2]&0x0F;
                dbg4_p("readStatus FPGA Status %x \r\n",status>>24);
                status=1;
                break;
            }
            default:
            {
                //
                status=1;
                dbg4("Option Error\r\n");
                break;
            }
        } //end switch


        if(0==status)
        {
            usleep(5000);
            delay--;
            //printf("status 0x%0x \r\n",reg[2]);
        }
        else
        {
            break;
        }


    }//end while

    if(0==delay)
    {
        ret=FLASH_TIMEOUT_ERROR;
        dbg4("readStatus Read Busy Status Timeout and Exit: 0x%x\r\n",reg[2]);
    }
    else if(0==fpga_read_num)
    {
        dbg4("readStatus fpga_spi_read error 0x%x\r\n",reg[2]);
        ret=FLASH_SPI_READ_ERROR;
    }
    else
    {
        delay=100;
        ret=FLASH_OPERATION_SUCCESS;
    }

    return ret;
}

/**
*
* @brief
* @return -1 Error
          0  Right
*/
int MNet::readRequest(uint32_t *reg,uint8_t *ptr,uint32_t addr,uint16_t len)
{
    uint32_t  temp=0;
    int ret=0;
    ret=readStatus(reg,ptr,FPGA_FLASH_CHECK_REQUEST_STATUS);
    if(FLASH_OPERATION_SUCCESS!=ret)
    {
        dbg4("readRequest function check request error\n");
        return ret;

    }
    ///Send Read Request Command
    //printf("Send Read Request Command\r\n");
    *ptr++ = FPGA_RW_WRITE;
    *ptr++ = FPGA_DEVICE_FLASH;
    *ptr++ = 0x0;
    *ptr++ = 0x0;

    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;

    *ptr++ = 0x0;
    *ptr++ = 0x0;
    *ptr++ = 0x0;
    *ptr++ = FPGA_FLASH_READ_REQUEST;

    *ptr++= 0xFF&(addr>>24);
    *ptr++= 0xFF&(addr>>16);
    *ptr++= 0xFF&(addr>>8);
    *ptr++= 0xFF&(addr);

    temp=len-1;
    *ptr++= 0xFF&(temp>>24);
    *ptr++= 0xFF&(temp>>16);
    *ptr++= 0xFF&(temp>>8);
    *ptr++= 0xFF&(temp);

    ret= spi->fpga_spi_write(reg,5);
    if(ret<0)
    {
        ret= FLASH_SPI_WRITE_ERROR;
        dbg4("readRequest function fpga_spi_write error\n");
    }
    else
    {
        ret= FLASH_OPERATION_SUCCESS;
    }
    return ret;
}

/**
*
* @brief
* @return -1(0xFFFFFFFF) Error
          0xA  Right
*/
uint32_t MNet::readDataValidation(void)
{
    uint8_t delay=100;

    int ret=0;
    uint32_t status=0;
    uint32_t reg[3]={0};
    uint8_t *ptr=(uint8_t *)reg;
    ///reaq Busy Status
    printf("Checkout Busy Status\r\n");
    *ptr++= FPGA_RW_READ;
    *ptr++= FPGA_DEVICE_FLASH;
    *ptr++= 0x00;
    *ptr++= FPGA_FLASH_READ_STATUS_REG;

    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;
    *ptr++ = FPGA_FLASH_SPI_OPE_DUM;

    ret=spi->fpga_spi_read(reg,3);

    if(ret<0) //spi read error
    {
        return -1;
        dbg4("readDataValidation function fpga_spi_write error\n");

    }
    return (reg[2]>>4)&0xF;
    //return (reg[2]&0x0F0);

}

/**
*
* @brief
* @return -1 Error
*/
int MNet::readData(uint32_t *reg,uint8_t *ptr,uint32_t addr,uint16_t len)
{
    int ret=0;

    ret=readStatus(reg,ptr,FPGA_FLASH_CHECK_READ_STATUS);
    if(FLASH_OPERATION_SUCCESS!=ret)
    {
        printf("readData  function readStatus error\n");
        return ret;
    }

    /// Read Data
    *ptr++=FPGA_RW_READ;
    *ptr++=FPGA_DEVICE_FLASH;
    *ptr++=0;
    *ptr++=FPGA_FLASH_READ_DATA_REG;

    *ptr++=FPGA_FLASH_SPI_OPE_DUM;
    *ptr++=FPGA_FLASH_SPI_OPE_DUM;
    *ptr++=FPGA_FLASH_SPI_OPE_DUM;
    *ptr++=FPGA_FLASH_SPI_OPE_DUM;

    ret=spi->fpga_spi_read(reg,len/BYTESPERUINT32+FPGA_FLASH_SPI_CMD_NUM);
    if(ret<0)
    {
        ret=FLASH_SPI_READ_ERROR;
        printf("readData  function fpga_spi_read error\n");
    }
    else
    {
        ret=FLASH_OPERATION_SUCCESS;
    }
    return ret;

}

/**
*
* @brief
* @return -1 Error
          0  Right
*/
int MNet::ModFlashReadPre(uint32_t *reg, uint32_t addr,unsigned int len)
{
    // Init Data
    uint8_t *ptr;
    int ret =0;
    int i=0;
    //memset(reg,0x00,len+(FPGA_FLASH_SPI_REG_NUM+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_EXT_NUM)*BYTESPERUINT32);
    /*************************************************************************/
    /// Send Read Request Command
    ptr=(uint8_t *)reg;
    ret=readRequest(reg,ptr,addr,len);
    if(FLASH_OPERATION_SUCCESS!=ret)
    {
        return ret;
    }


    /// Read Data
    ptr=(uint8_t *)reg;
    ret=readData(reg,ptr,addr,len);
    if(FLASH_OPERATION_SUCCESS==ret)
    {
        return ret;
    }

    //Print Read Data
    /*
    ptr=(uint8_t *)reg;
    printf("Read data From FPGA Flash:\n");
    printf("ModFlashReadPre %d\n",addr);
    for(i=0;i<len+FPGA_FLASH_SPI_CMD_NUM*BYTESPERUINT32;i++)
    {
        printf("0x%0x ",ptr[i]);
        if(i%16==0)printf("\r\n");
    }
    printf("\r\n");
    */

    return ret;

}

int
MNet::ModFlashRead(unsigned int *reg,unsigned int *pReg)
{
    unsigned char *pData;
    int ret=0;

    ret=ModFlashReadPre(reg,data_head->addr,data_head->datLen);//2是数据头
    if(FLASH_OPERATION_SUCCESS!=ret)
    {
        ackResult.res=ACK_ERR_SPI_READ;
        protocol->status=PACK_ACK;
        return FLASH_READ_ERROR;
    }

    //printf("Ready for the data to PC ...\r\n");
    int sendLen=sizeof(pack_header_t)+sizeof(data_head_t)+data_head->datLen;
    unsigned char *pSend=(unsigned char *)malloc(sendLen);
    if(pSend==NULL)
    {
        dbg4("err:malloc\n");
        if(reg!=NULL)
        {
            free(reg);
            reg=NULL;
        }

        freeComPtr();
        ackResult=protocol->setResult(ACK_ERR_MALLOC,__LINE__);
        protocol->status=PACK_ACK;
        return -7;
    }

    pReg=reg+2;
    //memset(preg,0x55,data_head->datLen);
    data_head->crc=this->protocol->crc32((unsigned char*)pReg,data_head->datLen);
    data_head->chkSum=this->protocol->checkSum((unsigned char * )data_head,sizeof(data_head_t)-4);
    pData=(unsigned char* )(pSend+sizeof(pack_header_t)+sizeof(data_head_t));


    pack_header->dataLen=sizeof(data_head_t)+data_head->datLen;
    pack_header->chkSum=protocol->checkSum((unsigned char *)pack_header,sizeof(pack_header_t)-4);

    memcpy( (char*)pSend, (char*)pack_header,sizeof(pack_header_t));
    memcpy( (char*)(pSend+sizeof(pack_header_t)), (char*)data_head,sizeof(data_head_t));
    memcpy(pData,(unsigned char*)pReg,data_head->datLen);
    ret = send(sock, (char*)pSend,sendLen, 0) ;

//    printf("data_head->crc=0x%x,data_head->chkSum=0x%0x\n",data_head->crc,data_head->chkSum);
//    printf("pack_header_t size:%d",sizeof(pack_header_t));
//    printf("data_head_t size %d",sizeof(data_head_t));
//
//    printf("data_head->datLen:%d",data_head->datLen);
//    for(int i=0;i<data_head->datLen;i++)
//    {
//     if(i%16==0)
//       printf("\n");
//     printf("%2x ",pData[i]);
//
//    }
//     printf("\r\n");
//
//
//
//
//    printf("sendLen%d",sendLen);
//    for(int i=0;i<sendLen;i++)
//    {
//     if(i%16==0)
//       printf("\n");
//     printf("%2x ",pSend[i]);
//     if(i==sizeof(pack_header_t)-1)printf("\n");
//     if(i==sizeof(pack_header_t)+sizeof(data_head_t)-1)printf("\n");
//
//    }
//     printf("\r\n");


     if(pSend!=NULL)
     {
        printf("free pSend memeory \n");
        free(pSend);
        pSend=NULL;
     }

    if(ret<0)
    {
        printf("sock send err:");
        freeComPtr();
//        if(reg!=NULL)
//        {
//            free(reg);
//            reg=NULL;
//        }

        ackResult=protocol->setResult(ACK_ERR_NET_SEND,ret);
        errlast=errno;
        protocol->status=PACK_ACK;
        return ERR_NET_STATUS;
    }

    protocol->status=PACK_RECIVE_HEADER;
    //printf("Read Data Compeletely\n");
    return ret;
}


/**
*
*
*/
int MNet::ModFlashWrite(uint32_t *reg)
{
    int ret=0;
    int writeErro=0;
    unsigned int readLen;
    unsigned int sectorOffset;

    unsigned int *preg=NULL;
    uint8_t *ptr;
    uint32_t len=FPGA_FLASH_SECTOR_SIZE;

    int rewrite_num=0,rewrite_num_crc=0;

#ifdef ARM_PC_DEBUG
	printf("PCCCCCCCCCCCCCCCCCCC Write FPGA Flash \n");
	printf("Write FPGA Flash Addr 0x%0x \n",data_head->addr);
	printf("Write FPGA Flash len  0x%0x \n",data_head->datLen);
	printf("Write FPGA Flash Data:\n");

    for(int i=0;i<data_head->datLen;i++)
	{
        printf("0x%0x ",pack_info->data[i]);
        if(i%15==0)
            printf("\r\n");
	}
#else

    if(reg==NULL)
	{
        printf("assign memery failure\n");
		return -1;
	}
    //preg=reg;
    //printf("MNet::ModFlashWrite(uint32_t *reg)\r\n");

    int writeLen=data_head->datLen;
    //int writeAddr=data_head->addr*4;
    int writeAddr=data_head->addr;
    int sectorNum=writeAddr/FPGA_FLASH_SECTOR_SIZE;
    int sectorPos=writeAddr%FPGA_FLASH_SECTOR_SIZE;
    int loop=(writeLen+FPGA_FLASH_SECTOR_SIZE-1)/FPGA_FLASH_SECTOR_SIZE;
    if(0!=sectorPos)
    {
        loop=loop+1;
    }
    int mySectorPos=writeAddr;
    int mySectorLen=writeLen;
    int mySectorIndex=sectorNum;
    unsigned char *pData=(unsigned char* )pack_info->data;
    int writeAddrSec=0;
    int thisWriteLen=0;
    int write_csc=0;

    //Write Data
    for(int iL=0;iL<loop;iL++)
    {
        //printf("for(int iL=0;iL<loop;iL++). %d times.. %d  %d \r\n",iL, loop, mySectorLen);
        preg=reg;
        //mySectorIndex=mySectorPos/FPGA_FLASH_SECTOR_SIZE;
        if(mySectorLen<1) {
            break;
        }

        mySectorPos=mySectorPos%FPGA_FLASH_SECTOR_SIZE;

        //printf("Write/Program the whole sector to FPGA. %d times.. %d %d %d\r\n",iL, mySectorPos, writeAddr, writeLen);

        ///Judge whether it is the sector initial address
        if(0==mySectorPos)
        {
            ///Judge whether it is the whole sector data
            if(mySectorLen>=FPGA_FLASH_SECTOR_SIZE)
            {
                dbg4_p("Write/Program the whole sector to FPGA...\r\n");
                preg=reg+2+8;
                thisWriteLen=FPGA_FLASH_SECTOR_SIZE;
                memcpy((unsigned char*)preg,pData,thisWriteLen);

                /*
                printf("Write Data 0000 ...\r\n");
                uint8_t * ptr=(uint8_t *)(reg+2+8);
                for(int i=0;i<FPGA_FLASH_SECTOR_SIZE;i++)
                {
                    printf("0x%0x ",ptr[i]);
                    if(i%16==15)printf("\r\n");
                }
                printf("\r\n");


                printf("Write Data pData ...\r\n");
                for(int i=0;i<thisWriteLen;i++)
                {
                    printf("0x%0x ",pData[i]);
                    if(i%16==15)printf("\r\n");
                }
                printf("\r\n");
                */

                pData=pData+FPGA_FLASH_SECTOR_SIZE;
            }
            /// Write/Program the last sector to FPGA
            else
            {
                dbg4_p("Write/Program the last sector to FPGA...\r\n");
                preg=reg+8;
                ret=ModFlashReadPre(preg,mySectorIndex*FPGA_FLASH_SECTOR_UINT_ADDR,FPGA_FLASH_SECTOR_SIZE);
                if(FLASH_OPERATION_SUCCESS!=ret)
                {

                    writeErro=ACK_FLASH_WRITE_LAST_READ_ERROR;
                    break;
                }

                preg=reg+2+8;
                thisWriteLen=mySectorLen;
                memcpy((unsigned char*)preg,pData,thisWriteLen);

                /*
                printf("Write Data thisWriteLen...\r\n");
                uint8_t * ptr=(uint8_t *)(reg+2+8);
                for(int i=0;i<FPGA_FLASH_SECTOR_SIZE;i++)
                {
                    printf("0x%0x ",ptr[i]);
                    if(i%16==15)printf("\r\n");
                }
                printf("\r\n");


                printf("Write Data pData thisWriteLen ...\r\n");
                for(int i=0;i<thisWriteLen;i++)
                {
                    printf("0x%0x ",pData[i]);
                    if(i%16==15)printf("\r\n");
                }
                printf("\r\n");
                */

                //pData=pData+thisWriteLen;
            }

        }
        ///Write/Program the start sector to FPGA
        else
        {
            dbg4_p("Write/Program the start sector to FPGA. ..\r\n");
            thisWriteLen=FPGA_FLASH_SECTOR_SIZE-mySectorPos;
            if(thisWriteLen>mySectorLen)
            {
                thisWriteLen=mySectorLen;

            }

            preg=reg+8;
            ret=ModFlashReadPre(preg,mySectorIndex*FPGA_FLASH_SECTOR_UINT_ADDR,FPGA_FLASH_SECTOR_SIZE);
            if(FLASH_OPERATION_SUCCESS!=ret)
            {

                writeErro=ACK_FLASH_WRITE_START_READ_ERROR;
                break;
            }
            //printf("ModFlashReadPre(preg,mySectorIndex*FPGA_FLASH_SECTOR_UINT_ADDR,FPGA_FLASH_SECTOR_SIZE\r\n");
            //printf("pData=pData+thisWriteLenmySectorPos%d,thisWriteLen%d\r\n",mySectorPos,thisWriteLen);
            preg=reg+2+8+mySectorPos/4;
            memcpy((unsigned char*)preg,pData,thisWriteLen);
            pData=pData+thisWriteLen;
        }

        /// Check the SPI Flash Status whether to read
        ptr=(uint8_t *)reg;
        ret=readStatus(reg,ptr,FPGA_FLASH_CHECK_WRITE_STATUS);
        if(FLASH_OPERATION_SUCCESS!=ret)
        {
             writeErro=ACK_FLASH_WRITE_STATUS_BUSY_ERROR;
             break;

        }
        ///Write the data to the FPGA for debug
         //memset(preg,0x44,len+(FPGA_FLASH_SPI_REG_NUM+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_EXT_NUM)*BYTESPERUINT32);
        /// Prepare to Command to Write/Program the data to FPGA
        writeAddrSec=mySectorIndex*FPGA_FLASH_SECTOR_UINT_ADDR;
        ptr=(uint8_t *)reg;

        *ptr++=FPGA_RW_WRITE;*ptr++=FPGA_DEVICE_FLASH;*ptr++=0;*ptr++=0;
        *ptr++=FPGA_FLASH_SPI_OPE_DUM;*ptr++=FPGA_FLASH_SPI_OPE_DUM;*ptr++=FPGA_FLASH_SPI_OPE_DUM;*ptr++=FPGA_FLASH_SPI_OPE_DUM;

        *ptr++=0;*ptr++=0;*ptr++=0;*ptr++=FPGA_FLASH_WRITE_DATA;
        *ptr++=0xFF&(writeAddrSec>>24);*ptr++=0xFF&(writeAddrSec>>16);*ptr++=0xFF&(writeAddrSec>>8);*ptr++=0xFF&(writeAddrSec);
        len=FPGA_FLASH_SECTOR_SIZE-1;
        *ptr++=0xFF&(len>>24);*ptr++=0xFF&(len>>16);*ptr++=0xFF&(len>>8);*ptr++=0xFF&(len);
        reg[5]=FPGA_NULL;
        reg[6]=FPGA_NULL;
        reg[7]=FPGA_NULL;
         /// Sum CRC
        uint32_t ret_crc=0,data_sum_crc=0;
        for(int j=0;j<3;j++)
        {
            data_sum_crc+=reg[j];
        }
        reg[8]=data_sum_crc&0xFFFFFFFF;
        reg[9]=FPGA_NULL;
        //reset the counters
        rewrite_num=5;
        rewrite_num_crc=5;

resend:
        ret=spi->fpga_spi_write(reg,FPGA_FLASH_SECTOR_SIZE/BYTESPERUINT32+FPGA_FLASH_SPI_CMD_NUM+FPGA_FLASH_SPI_REG_NUM);
//        preg=reg+10;
//        printf("write data \n");
//        for(int i=0;i<FPGA_FLASH_SECTOR_SIZE;i++)
//        {
//            printf("0x%0x ",preg[i]);
//            if(i%16==15)printf("\r\n");
//        }
//        printf("\r\n");

//        if(ret!=0)//if data is correct
//        {
//            if(0==rewrite_num--)
//            {
//                writeErro=ACK_FLASH_REWRITE_SECTOR_ERROR;
//                break;// exit the "for" cycle
//            }
//            else
//            {
//                goto resend;
//
//            }
//
//        }

        /// Check Command CRC
        ret_crc=readDataValidation();;

        if(ret_crc!=0xA)//if data is incorrect
        {
             printf("Read command Validation ret_crc error,rewrite_num %d\n",rewrite_num);
            if(0==(--rewrite_num))
            {
                writeErro=ACK_FLASH_REWRITE_SECTOR_ERROR;
                break;// exit the "for" cycle
            }
            else
            {
                goto resend;

            }

        }
        rewrite_num=5;//write/rewrite sucess and reset the counters

        ///Check This Write Data CRC
        preg=reg+2+8;
        write_csc=this->protocol->crc32((unsigned char*)preg,FPGA_FLASH_SECTOR_SIZE);


        preg=reg;
        ret=ModFlashReadPre(preg,writeAddrSec,FPGA_FLASH_SECTOR_SIZE);
        if(FLASH_OPERATION_SUCCESS!=ret)
        {
            writeErro=ACK_FLASH_WRITE_SECTOR_READ_ERROR+ret+16;
            break;
        }
        preg=reg+2;
        ackResult.crc=this->protocol->crc32((unsigned char*)preg,FPGA_FLASH_SECTOR_SIZE);
        if(ackResult.crc!=write_csc)
        {

            printf("Read data CRC error,rewrite_num_crc %d\n",rewrite_num_crc);
            if(0==(--rewrite_num_crc))
            {
               writeErro=ACK_FLASH_WRITE_SECTOR_CRC_ERROR;
               dbg4_p("this write task failure by crc\r\n");
   //            printf("Read data \n");
   //            for(int i=0;i<FPGA_FLASH_SECTOR_SIZE;i++)
   //            {
   //                printf("0x%0x ",preg[i]);
   //                if(i%16==15)printf("\r\n");
   //            }
   //            printf("\r\n");
               break;// exit the "for" cycle
           }
           else
           {
               goto resend;

           }


        }
        else
        {
            dbg4_p("this write task success by crc %dst loop%d\r\n",iL+1, loop);
        }
        
        rewrite_num_crc=5;//write and read success and reset the counters

        /// Update the cycle variable
        mySectorIndex=mySectorIndex+1;
        mySectorPos=0;
        mySectorLen=mySectorLen-thisWriteLen;

    }///for end

    ///transmit the result to the PC
    protocol->status=PACK_ACK;
    if(ACK_FLASH_WRITE_LAST_READ_ERROR==writeErro||
        ACK_FLASH_WRITE_START_READ_ERROR==writeErro||
        ACK_FLASH_WRITE_STATUS_BUSY_ERROR==writeErro||
        ACK_FLASH_WRITE_SECTOR_READ_ERROR==writeErro||
        ACK_FLASH_REWRITE_SECTOR_ERROR==writeErro||
        ACK_FLASH_WRITE_SECTOR_CRC_ERROR==writeErro
    )
    {
        dbg4_p("this write data error\n");
        ackResult.res=writeErro;//
        return FLASH_WRITE_ERROR;
    }
#endif


    if(((pack_header->cmdOption>>24)&0xFF)!=0x5A)
    {
        dbg4_p("(pack_header->cmdOption>>24)&0xFF)!=0x5A and read data\n");
        preg=reg;
        ret=ModFlashReadPre(preg,writeAddr,data_head->datLen);
        if(ret<0)
        {

            ackResult.res=ACK_ERR_SPI_WRITE_CRC;
            return -3;
        }

        ackResult.crc=this->protocol->crc32((unsigned char*)&preg[2],data_head->datLen);
        if(ackResult.crc!=data_head->crc)
        {
            dbg4_p("write data all  error\n");
            ackResult.res=ACK_ERR_SPI_WRITE_CRC;
        }
    }
    return 0;

}
