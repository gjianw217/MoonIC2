#include <stdint.h>
#include "spi_fpga.h"



struct spi_fpga_device  spi_fpga_device;

 void fpga_lock(struct spi_fpga_device * fpga_device)
{
    rt_mutex_take(&fpga_device->lock, RT_WAITING_FOREVER);
}

 void fpga_unlock(struct spi_fpga_device * fpga_device)
{
    rt_mutex_release(&fpga_device->lock);
}

 rt_size_t stm32_fpga_read(rt_device_t dev,
                                   rt_off_t pos,
                                   void* buffer,
                                   rt_size_t size)
{
		
	
		char *ptr=buffer;
	
		RT_ASSERT(dev != RT_NULL);
	
    fpga_lock((struct spi_fpga_device *)dev);

		rt_spi_send_then_recv(spi_fpga_device.rt_spi_device,
                          ptr, 8,
                          &ptr[8], size);

    fpga_unlock((struct spi_fpga_device *)dev);

    return size;
}

 rt_size_t stm32_fpga_write(rt_device_t dev,
                                    rt_off_t pos,
                                    const void* buffer,
                                    rt_size_t size)
{
		RT_ASSERT(dev != RT_NULL);
    fpga_lock((struct spi_fpga_device *)dev);


		rt_spi_send(spi_fpga_device.rt_spi_device, buffer, size);
		/*rt_spi_send_then_send(spi_fpga_device.rt_spi_device,
					send_buffer, 	//命令buffer
					4,						//长度
					buffer,				//数据buffer
					256						//长度
			);*/

    fpga_unlock((struct spi_fpga_device *)dev);

    return size;
}

rt_err_t stm32_dev_fpga_init(const char * fpga_device_name, const char * spi_device_name)
{
    struct rt_spi_device * rt_spi_device;

    /* initialize mutex */
    if (rt_mutex_init(&spi_fpga_device.lock, spi_device_name, RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        rt_kprintf("init sd lock mutex failed\n");
        return -RT_ENOSYS;
    }

    rt_spi_device = (struct rt_spi_device *)rt_device_find(spi_device_name);
    if(rt_spi_device == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\r\n", spi_device_name);
        return -RT_ENOSYS;
    }
    spi_fpga_device.rt_spi_device = rt_spi_device;

    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB; /* SPI Compatible: Mode 0 and Mode 3 */
        cfg.max_hz = 24 * 1000 * 1000; /* 24M */
        rt_spi_configure(spi_fpga_device.rt_spi_device, &cfg);
    }

    /* init fpga */

    /* register device */
    spi_fpga_device.fpga_device.type    = RT_Device_Class_SPIDevice;//RT_Device_Class_Block;
    spi_fpga_device.fpga_device.init    = RT_NULL;
    spi_fpga_device.fpga_device.open    = RT_NULL;
    spi_fpga_device.fpga_device.close   = RT_NULL;
    spi_fpga_device.fpga_device.read 	  = stm32_fpga_read;
    spi_fpga_device.fpga_device.write   = stm32_fpga_write;
    spi_fpga_device.fpga_device.control = RT_NULL;
    /* no private */
    spi_fpga_device.fpga_device.user_data = RT_NULL;

    rt_device_register(&spi_fpga_device.fpga_device, fpga_device_name,
                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
//rt_spi_bus_attach_device
    return RT_EOK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
#endif
#include "mprotocol.h"

int  spi_read_write_test(int addr,unsigned long dat)
{
	rt_device_t spidev=NULL;
	unsigned long buf_tx[16],buf_rx[20];
  int i=0;
	
	spidev=rt_device_find("fpga");
	rt_device_open(spidev,RT_DEVICE_OFLAG_RDWR);
	
	
		
//for(i=0;i<100000;i++)
	{
		spi_packt((unsigned long*)buf_tx,1,addr,0);
		buf_tx[2]=spi_swap(dat);
		
		//rt_spi_send(rt_spi_device, (char*)buf_tx, 3*4);
		rt_device_write(spidev,0,buf_tx,12);
		rt_thread_delay(1);
		//spi_fpga_device.fpga_device.read(&spi_fpga_device.fpga_device,0,buf_rx,16);
		spi_packt((unsigned long*)buf_rx,0,addr,0);
		/*rt_spi_send_then_recv(rt_spi_device,
                          (char*)buf_rx, 8,
                          (char*)&buf_rx[2], 4);
	*/
		rt_device_read(spidev,0,buf_rx,4);
		rt_thread_delay(1);
	}
		rt_kprintf("read:%8x\r\n",	spi_swap(buf_rx[2]));
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(spi_read_write_test, spiTest, read write spi test);

int  fpga_rw_test(unsigned char rw,unsigned char type,unsigned short addr,unsigned long v)
{
	rt_device_t spidev=NULL;
	unsigned long buf_tx[64];
  int i=0,j=0;
	
	spidev=rt_device_find("fpga");
	rt_device_open(spidev,RT_DEVICE_OFLAG_RDWR);
	
	
		
	memset(buf_tx,spi_swap(v),4*64);

	

	//buf_tx[3]=spi_swap(0xA5);

		
//	for(i=0;i<100000;i++)
	{
		spi_packt((unsigned long*)buf_tx,rw>0?1:0,addr,type);
		if(rw)
		{
			rt_device_write(spidev,0,buf_tx,12);
			
		}
		else
		{
				rt_device_read(spidev,0,(char*)buf_tx, 4*32);
				
				for(j=0;j<8;j++)
			{
				//if(buf_tx[j*4+2] ||  buf_tx[j*4+3])
				rt_kprintf("%d:%8x,%8x,%8x,%8x\r\n",j,spi_swap(buf_tx[j*4+2]),spi_swap(buf_tx[j*4+3]),spi_swap(buf_tx[j*4+4]),spi_swap(buf_tx[j*4+5]));
			}
	}
		rt_thread_delay(10);
}
		rt_thread_delay(1);
		//spi_fpga_device.fpga_device.read(&spi_fpga_device.fpga_device,0,buf_rx,16);
		//spi_packt((unsigned long*)buf_rx,0,addr,0);
		/*rt_spi_send_then_recv(rt_spi_device,
                          (char*)buf_rx, 8,
                          (char*)&buf_rx[2], 4);
	*/
		//rt_device_read(spidev,0,buf_rx,4);
		rt_thread_delay(1);
	//}
		//rt_kprintf("read:%8x\r\n",	spi_swap(buf_rx[2]));
    return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(fpga_rw_test, spiRW, write spi test);
