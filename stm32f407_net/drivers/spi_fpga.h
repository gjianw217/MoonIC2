#ifndef __SPI_FPGA__
#define __SPI_FPGA__
#include <rtthread.h>
#include <drivers/spi.h>

struct spi_fpga_device
{
    struct rt_device                fpga_device;
    struct rt_spi_device *          rt_spi_device;
    struct rt_mutex                 lock;
};

extern rt_err_t stm32_dev_fpga_init(const char * fpga_device_name,
                            const char * spi_device_name);

#endif
