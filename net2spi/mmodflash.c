rD+��`k�3�v��i�H8N=���Q]�k�x��V�.P�ߚ�?EKjA����jZX�N_�j��t(}q�$U��S���T��Z�ɀ�&���|n�m��������=��,�R�?�YBMP��oo��MӁ���uY�Q�̡]��;���Y3�Ew�N�S�4.�L1\]��]��X���t���v�>��_���
�Ի$��1M\�ؠM���+%�I;�f(k�C�P0U�|+���A��H��܏}��W��状�P��|UH��u8]+^3��M���Aפ;��ͫ���
��G������Q[��Ⱥ'r��H1���p��ϷIq��V�Ŀ�6���8B瘻�;N}�]lT`�p(3��I,ES��ªc�_�;��J���)C܀��8Lsģ��Ǆ�=F�[Z���q�oz�CK9:�`A��FgY?p�j�@E��a�z=�X=�-Ě-�y2�$ !W�h�ٸ}�()�g�p^X������B�B~���W����jDR���O?�o��B�EȤ�:���'���m�-���f����%[�a��tx�+K���~��C8qa�}�S�d	a���G���<բ"�=-C���㡡��>;Y�E�)�!�p��*���go��o[�٭�gj1_^5���e��0�tEs�/���kA�p�;�)���̧*��!���
#�aB�`����}c��ؑ���(ͧ����˱�e��ݗ��h:��"m$�C��Ǆ�{��L�z�A�J���%�2�����J'eғ`�K��徥�@H��QQQdQ��]*,�.	���_:y����!��_G h�`�)�����*�`��S����Z����b&�ݫ����s`+M��-���7��h�={�|��J�!�P��U�#�|b^������.u���O�Z��U��8&���^���K��)�=��x�10�mఄ�{�7MPk[��3�h����.���j�ϝ�ǝgo����s WriteData(spi,OpeFlashAddr,OpeFlashLen);
    printf("Read Data.............................\n");
    ReadData(spi,OpeFlashAddr,OpeFlashLen);

    pread=(uint8_t *)&reg[2];
    pwrite=saveData;
    DataCheck(pwrite,pread,FPGA_FLASH_SECTOR_SIZE);
//    while(1)
//    {
//        WriteData(spi,OpeFlashAddr,OpeFlashLen);
//        sleep(1);
//
//    }

    if(reg!=NULL)
    {
        free(reg);
        reg=NULL;
    }
}

static void readRequest(MSpi *spi,uint8_t *ptr,uint32_t addr,uint16_t len)
{
    if(0!=readStatus(spi,ptr,2))
    {
        printf("check error\n");
        return ;

    }
    ///Send Read Request Command
    printf("Send Read Request Command\r\n");
    *ptr++ = (FPGA_RW_WRITE);
    *ptr++ =0x07;
    *ptr++ =0x0;
    *ptr++ =0x0;

    *ptr++ =0xff;
    *ptr++ =0xff;
    *ptr++ =0xff;
    *ptr++ =0xff;

    *ptr++ =0x0;
    *ptr++ =0x0;
    *ptr++ =0x0;
    *ptr++ =0x03;

    *ptr++=0xFF&(addr>>24);
    *ptr++=0xFF&(addr>>16);
    *ptr++=0xFF&(addr>>8);
    *ptr++=0xFF&(addr);

    len=len-1;
    *ptr++=0xFF&(len>>24);
    *ptr++=0xFF&(len>>16);
    *ptr++=0xFF&(len>>8);
    *ptr++=0xFF&(len);

    spi->fpga_spi_write(reg,5);
}

static int readStatus(MSpi *spi,uint8_t *ptr,uint8_t rw)
{
    uint8_t delay=10;
    int ret=0;
    int status=0;
    ///reaq Busy Status
    printf("Checkout Busy Status\r\n");
    *ptr++=FPGA_RW_READ;
    *ptr++=0x07;
    *ptr++=00;
    *ptr++=0x07;

    *ptr++ =0xff;
    *ptr++ =0xff;
    *ptr++ =0xff;
    *ptr++ =0xff;

    while(delay)
    {
        ret=spi->fpga_spi_read(reg,3);
        switch(rw)
        {
            case 1://write data
            {   printf("write data\r\n");
                if(0x0==((reg[2]>>16)&0x0FFFF))
                {
                    printf("Read Busy Status Success\r\n");
                    status=1;

                }
                break;
            }

            case 2://read request
            {   printf("read request\r\n");
                if(0x0==reg[2])
                {
                    printf("Read notBusy Status and NonReadable Status Success\r\n");
                    status=1;

                }
                break;

            }
            case 3://read data
            {   printf("read data\r\n");
                if(0xd600==(reg[2]&0x0FFFF))
                {
                    printf("Read Readable Statu Success\r\n");
                    status=1;

                }
                break;
            }
            default:
            {
                printf("Option Error\r\n");
                status=1;
                break;
            }
        }


        if(0==status)
        {
            usleep(5000);
            delay--;
            printf("status 0x%0x \r\n",reg[2]);
        }
        else
        {
            break;
        }


    }
    if(0==delay)
    {
        printf("Read Busy Status Timeout and Exit\r\n");
        ret=-1;

    }
    else
    {
        delay=10;
        ret=0;
    }

    return ret;
}

static void readData(MSpi *spi,uint8_t *ptr,uint32_t addr,uint16_t len)
{

    if(0!=readStatus(spi,ptr,3)) return ;

    /// Read Data
    int ret=0;
    printf("Read Data.....\r\n");
    *ptr++=FPGA_RW_READ;
    *ptr++=0x07;
    *ptr++=0;
    *ptr++=0x08;

    *ptr++=0xFF;
    *ptr++=0xFF;
    *ptr++=0xFF;
    *ptr++=0xFF;

    spi->fpga_spi_read(reg,len/4+2);
    printf("read data num %d\n",ret);


}
void ReadData(MSpi *spi,uint32_t addr,uint16_t len)
{
/*************************************************************************/
    // Init Data
    uint8_t *ptr;
    uint8_t delay=10;
    int ret =0;
    int i=0;
    memset(reg,0x00,FPGA_FLASH_SECTOR_SIZE+(8+2+1)*4);
    /*************************************************************************/
    /// Send Read Request Command
    ptr=(uint8_t *)reg;
    readRequest(spi,ptr,addr,len);

    /// Read Data
    ptr=(uint8_t *)reg;
    readData(spi,ptr,addr,len);
    //sleep(5);

    ptr=(uint8_t *)reg;
//    for(i=0;i<len+2*4;i++)
//    {
//        printf("0x%0x ",ptr[i]);
//        if(i==16)printf("\r\n");
//    }

}

void WriteData(MSpi *spi,uint32_t addr,uint16_t len)
{

    /*************************************************************************/
    unsigned char * ptr=(unsigned char *)reg;
    if(0!=readStatus(spi,ptr,1)) return ;
    //Write Data
    ///Prepare to write the modified data
    memset(reg,0x53,FPGA_FLASH_SECTOR_SIZE+(8+2+1)*4);

    /// Prepare to Command
    printf("Prepare to Command to Write/Program the data to FPGA...\r\n");
    ptr=(unsigned char *)reg;

    *ptr++=FPGA_RW_WRITE;*ptr++=FPGA_DEVICE_FLASH;*ptr++=0;*ptr++=0;

    *ptr++=0xFF;*ptr++=0xFF;*ptr++=0xFF;*ptr++=0xFF;

    *ptr++=0;*ptr++=0;*ptr++=0;*ptr++=0x20;

    *ptr++=0xFF&(addr>>24);*ptr++=0xFF&(addr>>16);*ptr++=0xFF&(addr>>8);*ptr++=0xFF&(addr);

    len=len-1;
    *ptr++=0xFF&(len>>24);*ptr++=0xFF&(len>>16);*ptr++=0xFF&(len>>8);*ptr++=0xFF&(len);

    reg[5]=FPGA_NULL;
    reg[6]=FPGA_NULL;
    reg[7]=FPGA_NULL;
    reg[8]=FPGA_NULL;
    reg[9]=FPGA_NULL;


    //add the rand number to test
    RandNumber(&reg[10],FPGA_FLASH_SECTOR_SIZE);

    ///Write the data to the FPGA
    printf("Write/Program the data to FPGA 58585858...\r\n");
    spi->fpga_spi_write(reg,FPGA_FLASH_SECTOR_SIZE/4+8+2);
}

void RandNumber(uint32_t *reg,uint32_t len)
{
    int i,number;
    uint8_t *pdata=saveData;
    uint8_t *ptr=(unsigned char *)reg;
    srand((unsigned)time(NULL));
    for(i=0;i<len;i++)
    {
        pdata[i]=rand()%256;
        ptr[i]=pdata[i];

    }
}

void DataCheck(uint8_t *pwrite,uint8_t *pread,uint32_t len)
{
    uint32_t i;
    printf("\nCheck the rand data starting \n");
    for(i=0;i<len;i++)
    {
        if(*pwrite++!=*pread++)
            printf("pwrite[%d]=%0x,pread[%d]=%0x\n",i,*pwrite,i,*pread);
    }
     printf("Check the rand data end \n");
}



