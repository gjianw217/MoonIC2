/**
  ******************************************************************************
  * @file    stm32_eth.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    11/20/2009
  * @brief   This file provides all the ETH firmware functions.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32_eth.h"
#include "stm32f4xx_rcc.h"
/**
  * @}
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
/*
 * STM32 Eth Driver for RT-Thread
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-05     Bernard      eth interface driver for STM32F107 CL
 */
#include <rtthread.h>
#include <netif/ethernetif.h>
#include <netif/etharp.h>
#include <lwip/icmp.h>
#include "lwipopts.h"

#define ETH_DEBUG
//#define ETH_RX_DUMP
//#define ETH_TX_DUMP

#ifdef ETH_DEBUG
#define STM32_ETH_TRACE	        rt_kprintf
#else
#define STM32_ETH_TRACE(...)
#endif /* ETH_DEBUG */

#if defined(ETH_RX_DUMP) ||  defined(ETH_TX_DUMP)
static void packet_dump(const char * msg, const struct pbuf* p)
{
    rt_uint32_t i;
    rt_uint8_t *ptr = p->payload;

    STM32_ETH_TRACE("%s %d byte\n", msg, p->tot_len);

    for(i=0; i<p->tot_len; i++)
    {
        if( (i%8) == 0 )
        {
            STM32_ETH_TRACE("  ");
        }
        if( (i%16) == 0 )
        {
            STM32_ETH_TRACE("\r\n");
        }
        STM32_ETH_TRACE("%02x ",*ptr);
        ptr++;
    }
    STM32_ETH_TRACE("\n\n");
}
#endif /* dump */

//#define ETH_RXBUFNB        	4
//#define ETH_TXBUFNB        	2
static ETH_InitTypeDef ETH_InitStructure;
static ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
static rt_uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];

#define MAX_ADDR_LEN 6
struct rt_stm32_eth
{
    /* inherit from ethernet device */
    struct eth_device parent;

    /* interface address info. */
    rt_uint8_t  dev_addr[MAX_ADDR_LEN];			/* hw address	*/
};
static struct rt_stm32_eth stm32_eth_device;
static struct rt_semaphore tx_buf_free;

/* interrupt service routine for ETH */
void ETH_IRQHandler(void)
{
    rt_uint32_t status;

    /* enter interrupt */
    rt_interrupt_enter();

    /* get DMA IT status */
    status = ETH->DMASR;

    if ( (status & ETH_DMA_IT_R) != (u32)RESET ) /* packet receiption */
    {
        /* a frame has been received */
        eth_device_ready(&(stm32_eth_device.parent));

        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    }

    if ( (status & ETH_DMA_IT_T) != (u32)RESET ) /* packet transmission */
    {
        rt_sem_release(&tx_buf_free);
        ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
    }

    /* Clear received IT */
    if ((status & ETH_DMA_IT_NIS) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_NIS;
    if ((status & ETH_DMA_IT_AIS) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_AIS;
    if ((status & ETH_DMA_IT_RO) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_RO;

    if ((status & ETH_DMA_IT_RBU) != (u32)RESET)
    {
        ETH_ResumeDMAReception();
        ETH->DMASR = (u32)ETH_DMA_IT_RBU;
    }

    if ((status & ETH_DMA_IT_TBU) != (u32)RESET)
    {
        ETH_ResumeDMATransmission();
        ETH->DMASR = (u32)ETH_DMA_IT_TBU;
    }

    /* leave interrupt */
    rt_interrupt_leave();
}

/* RT-Thread Device Interface */

/* initialize the interface */
static rt_err_t rt_stm32_eth_init(rt_device_t dev)
{
    vu32 Value = 0;

    /* Reset ETHERNET on AHB Bus */
    ETH_DeInit();

    /* Software reset */
    ETH_SoftwareReset();

    /* Wait for software reset */
    while(ETH_GetSoftwareResetStatus()==SET);

    /* ETHERNET Configuration ------------------------------------------------------*/
    /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
    ETH_StructInit(&ETH_InitStructure);

    /* Fill ETH_InitStructure parametrs */
    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;
    ETH_InitStructure.ETH_Speed = ETH_Speed_100M;
    ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;

    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Disable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#if CHECKSUM_BY_HARDWARE
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif /* CHECKSUM_BY_HARDWARE */

    /*------------------------   DMA   -----------------------------------*/

    /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
    the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
    if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
    ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
    ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
    ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    /* Configure ETHERNET */
    Value = ETH_Init(&ETH_InitStructure,0);

    /* Enable DMA Receive interrupt (need to enable in this case Normal interrupt) */
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, ENABLE);

    /* Initialize Tx Descriptors list: Chain Mode */
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    /* Initialize Rx Descriptors list: Chain Mode  */
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

    /* MAC address configuration */
    ETH_MACAddressConfig(ETH_MAC_Address0, (u8*)&stm32_eth_device.dev_addr[0]);

    /* Enable MAC and DMA transmission and reception */
    ETH_Start();

    return RT_EOK;
}

static rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t rt_stm32_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t rt_stm32_eth_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    switch(cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if(args) rt_memcpy(args, stm32_eth_device.dev_addr, 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }

    return RT_EOK;
}

/* ethernet device interface */
/* transmit packet. */
rt_err_t rt_stm32_eth_tx( rt_device_t dev, struct pbuf* p)
{
    struct pbuf* q;
    rt_uint32_t offset;

    /* get free tx buffer */
    {
        rt_err_t result;
        result = rt_sem_take(&tx_buf_free, 2);
        if (result != RT_EOK) return -RT_ERROR;
    }

    offset = 0;
    for (q = p; q != NULL; q = q->next)
    {
        rt_uint8_t* ptr;
        rt_uint32_t len;

        len = q->len;
        ptr = q->payload;

        /* Copy the frame to be sent into memory pointed by the current ETHERNET DMA Tx descriptor */
        while (len)
        {
            (*(__IO uint8_t *)((DMATxDescToSet->Buffer1Addr) + offset)) = *ptr;

            offset ++;
            ptr ++;
            len --;
        }
    }

#ifdef ETH_TX_DUMP
    packet_dump("TX dump", p);
#endif

    /* Setting the Frame Length: bits[12:0] */
    DMATxDescToSet->ControlBufferSize = (p->tot_len & ETH_DMATxDesc_TBS1);
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
    /* Enable TX Completion Interrupt */
    DMATxDescToSet->Status |= ETH_DMATxDesc_IC;

#if CHECKSUM_BY_HARDWARE
    DMATxDescToSet->Status |= ETH_DMATxDesc_ChecksumTCPUDPICMPFull;
    /* clean ICMP checksum STM32F need */
    {
        struct eth_hdr *ethhdr = (struct eth_hdr *)(DMATxDescToSet->Buffer1Addr);
        /* is IP ? */
        if( ethhdr->type == htons(ETHTYPE_IP) )
        {
            struct ip_hdr *iphdr = (struct ip_hdr *)(DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR);
            /* is ICMP ? */
            if( IPH_PROTO(iphdr) == IP_PROTO_ICMP )
            {
                struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR + sizeof(struct ip_hdr) );
                iecho->chksum = 0;
            }
        }
    }
#endif /* CHECKSUM_BY_HARDWARE */

    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;
    /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET)
    {
        /* Clear TBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TBUS;
        /* Transmit Poll Demand to resume DMA transmission*/
        ETH->DMATPDR = 0;
    }

    /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);

    /* Return SUCCESS */
    return RT_EOK;
}

/* reception packet. */
struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{
    struct pbuf* p;
    rt_uint32_t framelength = 0;

    /* init p pointer */
    p = RT_NULL;

    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET))
        return p;

    if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET))
    {
        /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
        framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT) - 4;

        /* allocate buffer */
        p = pbuf_alloc(PBUF_LINK, framelength, PBUF_RAM);
        if (p != RT_NULL)
        {
            const char * from;
            struct pbuf* q;

            from = (const char *)(DMARxDescToGet->Buffer1Addr);

            for (q = p; q != RT_NULL; q= q->next)
            {
                /* Copy the received frame into buffer from memory pointed by the current ETHERNET DMA Rx descriptor */
                memcpy(q->payload, from, q->len);
                from += q->len;
            }

#ifdef ETH_RX_DUMP
            packet_dump("RX dump", p);
#endif /* ETH_RX_DUMP */
        }
    }

    /* Set Own bit of the Rx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMARxDescToGet->Status = ETH_DMARxDesc_OWN;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        ETH->DMARPDR = 0;
    }

    /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
    /* Chained Mode */
    if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RCH) != (uint32_t)RESET)
    {
        /* Selects the next DMA Rx descriptor list for next buffer to read */
        DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
    }
    else /* Ring Mode */
    {
        if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RER) != (uint32_t)RESET)
        {
            /* Selects the first DMA Rx descriptor for next buffer to read: last Rx descriptor was used */
            DMARxDescToGet = (ETH_DMADESCTypeDef*) (ETH->DMARDLAR);
        }
        else
        {
            /* Selects the next DMA Rx descriptor list for next buffer to read */
            DMARxDescToGet = (ETH_DMADESCTypeDef*) ((uint32_t)DMARxDescToGet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));
        }
    }

    return p;
}

static void RCC_Configuration(void)
{
    /* Enable ETHERNET clock  */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                          RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    /* Enable GPIOs clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE| RCC_APB2Periph_AFIO, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * GPIO Configuration for ETH
    AF Output Push Pull:
    - ETH_MDC   : PC1
    - ETH_MDIO  : PA2
    - ETH_TX_EN : PB11
    - ETH_TXD0  : PB12
    - ETH_TXD1  : PB13
    - ETH_TXD2  : PC2
    - ETH_TXD3  : PB8
    - ETH_PPS_OUT / ETH_RMII_PPS_OUT: PB5

    Input (Reset Value):
    - ETH_MII_TX_CLK: PC3
    - ETH_MII_RX_CLK / ETH_RMII_REF_CLK: PA1
    - ETH_MII_CRS: PA0
    - ETH_MII_COL: PA3
    - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PA7
    - ETH_MII_RXD0: PC4
    - ETH_MII_RXD1: PC5
    - ETH_MII_RXD2: PB0
    - ETH_MII_RXD3: PB1
    - ETH_MII_RX_ER: PB10

    ***************************************
             For Remapped Ethernet pins
    *******************************************
    Input (Reset Value):
    - ETH_MII_RX_DV / ETH_RMII_CRS_DV: PD8
    - ETH_MII_RXD0 / ETH_RMII_RXD0: PD9
    - ETH_MII_RXD1 / ETH_RMII_RXD1: PD10
    - ETH_MII_RXD2: PD11
    - ETH_MII_RXD3: PD12
 */
static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

#if STM32_ETH_IO_REMAP
    /* ETHERNET pins remapp in STM3210C-EVAL board: RX_DV and RxD[3:0] */
    GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);
#endif /* STM32_ETH_IO_REMAP */

    /* MII/RMII Media interface selection */
#if (RMII_MODE == 0) /* Mode MII. */
    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_MII);
#elif (RMII_MODE == 1)  /* Mode RMII. */
    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
#endif /* RMII_MODE */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* MDIO */
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

        /* MDC */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        /* MDIO */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    } /* MDIO */

    /* TXD */
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

        /* TX_EN */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* TXD0 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* TXD1 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

#if (RMII_MODE == 0)
        /* TXD2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        /* TXD3 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* TX_CLK */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif /* RMII_MODE */
    } /* TXD */

    /* RXD */
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

#if (STM32_ETH_IO_REMAP == 0)
        /* RX_DV/CRS_DV */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* RXD0 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        /* RXD1 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

#if (RMII_MODE == 0)
        /* RXD2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* RXD3 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif /* RMII_MODE */
#else
        /* RX_DV/CRS_DV */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        /* RXD0 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        /* RXD1 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

#if (RMII_MODE == 0)
        /* RXD2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        /* RXD3 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif /* RMII_MODE */
#endif /* STM32_ETH_IO_REMAP */

#if (RMII_MODE == 0)
        /* CRS */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* COL */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* RX_CLK */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* RX_ER */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif /* RMII_MODE */
    } /* RXD */

#if (USE_MCO == 1)
#if (RMII_MODE == 0) /* Mode MII. */
    /* Get HSE clock = 25MHz on PA8 pin(MCO) */
    RCC_MCOConfig(RCC_MCO_HSE);
#elif (RMII_MODE == 1)  /* Mode RMII. */
    /* Get HSE clock = 25MHz on PA8 pin(MCO) */
    /* set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
    RCC_PLL3Config(RCC_PLL3Mul_10);
    /* Enable PLL3 */
    RCC_PLL3Cmd(ENABLE);
    /* Wait till PLL3 is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
    {}

    /* Get clock PLL3 clock on PA8 pin */
    RCC_MCOConfig(RCC_MCO_PLL3CLK);
#endif /* RMII_MODE */

    /* MCO pin configuration------------------------------------------------- */
    /* Configure MCO (PA8) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif /* USE_MCO */
}

void rt_hw_stm32_eth_init()
{
    RCC_Configuration();
    GPIO_Configuration();
    NVIC_Configuration();

    // OUI 00-80-E1 STMICROELECTRONICS
    stm32_eth_device.dev_addr[0] = 0x00;
    stm32_eth_device.dev_addr[1] = 0x80;
    stm32_eth_device.dev_addr[2] = 0xE1;
    // generate MAC addr from 96bit unique ID (only for test)
    stm32_eth_device.dev_addr[3] = *(rt_uint8_t*)(0x1FFFF7E8+7);
    stm32_eth_device.dev_addr[4] = *(rt_uint8_t*)(0x1FFFF7E8+8);
    stm32_eth_device.dev_addr[5] = *(rt_uint8_t*)(0x1FFFF7E8+9);

    stm32_eth_device.parent.parent.init       = rt_stm32_eth_init;
    stm32_eth_device.parent.parent.open       = rt_stm32_eth_open;
    stm32_eth_device.parent.parent.close      = rt_stm32_eth_close;
    stm32_eth_device.parent.parent.read       = rt_stm32_eth_read;
    stm32_eth_device.parent.parent.write      = rt_stm32_eth_write;
    stm32_eth_device.parent.parent.control    = rt_stm32_eth_control;
    stm32_eth_device.parent.parent.user_data  = RT_NULL;

    stm32_eth_device.parent.eth_rx     = rt_stm32_eth_rx;
    stm32_eth_device.parent.eth_tx     = rt_stm32_eth_tx;

    /* init tx buffer free semaphore */
    rt_sem_init(&tx_buf_free, "tx_buf", ETH_TXBUFNB, RT_IPC_FLAG_FIFO);

    /* register eth device */
    eth_device_init(&(stm32_eth_device.parent), "e0");
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static void phy_search(void)
{
    int i;
    int value;

    for(i=0; i<32; i++)
    {
        value = ETH_ReadPHYRegister(i, 2);
        rt_kprintf("addr %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_search, search phy use MDIO);

static void phy_dump(int addr)
{
    int i;
    int value;

    rt_kprintf("dump phy addr %d\n", addr);

    for(i=0; i<32; i++)
    {
        value = ETH_ReadPHYRegister(addr, i);
        rt_kprintf("reg %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_dump, dump PHY register);

static void phy_write(int addr, int reg, int value)
{
    ETH_WritePHYRegister(addr, reg ,value);
}
FINSH_FUNCTION_EXPORT(phy_write, write PHY register);

static void emac_dump(int addr)
{
    int i;
    int value;
    int *p = (int *)ETH;

    rt_kprintf("dump EAMC reg %d\n", addr);

    for(i=0; i<sizeof(ETH_TypeDef)/4; i++)
    {
        value = *p++;
        rt_kprintf("reg %04X: %08X\n", i*4, value);
    }
}
FINSH_FUNCTION_EXPORT(emac_dump, dump EMAC register);

#endif // RT_USING_FINSH
