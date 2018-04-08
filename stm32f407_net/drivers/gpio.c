/*
 * File      : gpio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-01-05     Bernard      the first version
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <gpio.h>

#ifdef RT_USING_PIN

/* STM32 GPIO driver */
struct pin_index
{
    int index;
    uint32_t rcc;
    GPIO_TypeDef *gpio;
    uint32_t pin;
};

struct pin_irq
{
	struct pin_index *index;
	gpio_irq_hander hander;
	unsigned long para;
};
static unsigned char EXTI9_5_IRQn_cnt=0;
static unsigned char EXTI15_10_IRQn_cnt=0;
static char *pin_irq_assert[16]={
	"\x01\x11\x2b\xFF",//17\43\49\x31
	"\x12\x2c\xFF",//44
	"\x24\x2D\xFF",//"\36\45\50",x32
	"\x02\x25\x2E\xFF",//3 "\x2\37\46\51",\x33
	"\x26\x2F\xFF",//"\38\47",
	"\x27\x30\xFF",//"\39\48",
	"\x05\x07\x08\x28\xFF",//40
	"\x06\x09\x13\x29\xFF",//7 "\x6\x9\19\41",
	"\x03\x04\x0a\x14\x1C\x2A\xFF",	
	"\x0B\x15\x1D\xFF",
	"\x16\x1E\xFF",//10
	"\x0c\x17\x1F\xFF",
	"\x0d\x18\x20\xFF",
	"\x0e\x19\x21\xFF",
	"\x0f\x1A\x22\xFF",
	"\x10\x1B\x23\xFF",
};//0£¬ý49£¬50£¬51
//8,18,14,15,25,26,33,
static struct pin_irq pin_irq_hander[16];

static const struct pin_index pins[] =
{
    { 0, RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_1},//led
		{ 1, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_0},
    { 2, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_3},
		{ 3, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_8},
		{ 4, RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_8},
    { 5, RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_6},
    { 6, RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_7},
    { 7, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_6},
		
    { 8, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_6},
    { 9, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_7},
    { 10, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_8},
    { 11, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_9},
		
    { 12, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_11},
    { 13, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_12},
    { 14, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_13},
    { 15, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_14},
    { 16, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_15},
		
    { 17, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_0},
    { 18, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_1},
		
    { 19, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_7},
		{ 20, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_8},
		{ 21, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_9},
		{ 22, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_10},
		{ 23, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_11},
		{ 24, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_12},
		{ 25, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_13},
		{ 26, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_14},
		{ 27, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_15},
		
		{ 28, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_8},
		{ 29, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_9},
		{ 30, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_10},
		{ 31, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_11},
		{ 32, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_12},
		{ 33, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_13},
		{ 34, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_14},
		{ 35, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_15},
		
		{ 36, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_2},
		{ 37, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_3},
		{ 38, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_4},
		{ 39, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_5},
		{ 40, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_6},
		{ 41, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_7},
		{ 42, RCC_AHB1Periph_GPIOG, GPIOG, GPIO_Pin_8},
		
		{ 43, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_0},
		{ 44, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_1},
		{ 45, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_2},
		{ 46, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_3},
		{ 47, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_4},
		{ 48, RCC_AHB1Periph_GPIOD, GPIOD, GPIO_Pin_5},
		
		{ 49, RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_0},//key
		{ 50, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_2},//nConfig
		{ 51, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_3},//nCe

};

#define ITEM_NUM(items) sizeof(items)/sizeof(items[0])
const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};

void stm32_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    if (value == PIN_LOW)
    {
        GPIO_ResetBits(index->gpio, index->pin);
    }
    else
    {
        GPIO_SetBits(index->gpio, index->pin);
    }
}

int stm32_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }

    if (GPIO_ReadInputDataBit(index->gpio, index->pin) == Bit_RESET)
    {
        value = PIN_LOW;
    }
    else
    {
        value = PIN_HIGH;
    }

    return value;
}
int stm32_pin_get_write(rt_device_t dev, rt_base_t pin)
{
	 int value;
    const struct pin_index *index;
		value = PIN_LOW;
    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }

		if (GPIO_ReadOutputDataBit(index->gpio, index->pin) == Bit_RESET)
    {
        value = PIN_LOW;
    }
    else
    {
        value = PIN_HIGH;
    }
		
		return value;
}
void stm32_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;
    GPIO_InitTypeDef  GPIO_InitStructure;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    /* GPIO Periph clock enable */
    RCC_AHB1PeriphClockCmd(index->rcc, ENABLE);

    /* Configure GPIO_InitStructure */
    GPIO_InitStructure.GPIO_Pin     = index->pin;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_25MHz;

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    } 
    else
    {
        /* input setting:default. */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    }
    GPIO_Init(index->gpio, &GPIO_InitStructure);
}

int assert_pin_irq(rt_base_t pin)
{
	int i=0,find=0;
	char *p=pin_irq_assert[0];
	
	find=0;
	for(i=0;i<16;i++)
	{
		p=pin_irq_assert[i];
		while(*p!=0xFF)
		{
			if(*p==pin)
			{
				find=1;
				goto pin_find;
			}
			else
				p++;
		}
	}
pin_find:
	if(find==1)
	{
			return i;
	}
	
	return -1;
}

void EXTI0_IRQHandler(void)
{
	 EXTI_ClearITPendingBit(EXTI_Line0);
	 if(pin_irq_hander[0].hander!=0)
			pin_irq_hander[0].hander(pin_irq_hander[0].para);
	
}

void EXTI1_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line1);
	if(pin_irq_hander[1].hander!=0)
			pin_irq_hander[1].hander(pin_irq_hander[1].para);
}
	
void EXTI2_IRQHandler(void){
	EXTI_ClearITPendingBit(EXTI_Line2);
		if(pin_irq_hander[2].hander!=0)
			pin_irq_hander[2].hander(pin_irq_hander[2].para);
}
                                                             
void EXTI3_IRQHandler(void){
	EXTI_ClearITPendingBit(EXTI_Line3);
		if(pin_irq_hander[3].hander!=0)
			pin_irq_hander[3].hander(pin_irq_hander[3].para);
}
                                                            
void EXTI4_IRQHandler(void){
 EXTI_ClearITPendingBit(EXTI_Line4);
			if(pin_irq_hander[4].hander!=0)
			pin_irq_hander[4].hander(pin_irq_hander[4].para);
}
 
void EXTI9_5_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line5)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
		if(pin_irq_hander[5].hander!=0)
			pin_irq_hander[5].hander(pin_irq_hander[5].para);
	}
	if(EXTI_GetITStatus(EXTI_Line6)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line6);
		if(pin_irq_hander[6].hander!=0)
			pin_irq_hander[6].hander(pin_irq_hander[6].para);
	}
	if(EXTI_GetITStatus(EXTI_Line7)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line7);
		if(pin_irq_hander[7].hander!=0)
			pin_irq_hander[7].hander(pin_irq_hander[7].para);
	}
	if(EXTI_GetITStatus(EXTI_Line8)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line8);
		if(pin_irq_hander[8].hander!=0)
			pin_irq_hander[8].hander(pin_irq_hander[8].para);
	}
	if(EXTI_GetITStatus(EXTI_Line9)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line9);
		if(pin_irq_hander[9].hander!=0)
			pin_irq_hander[9].hander(pin_irq_hander[9].para);
	}

}

void EXTI15_10_IRQHandler(void){
	if(EXTI_GetITStatus(EXTI_Line10)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line10);
		if(pin_irq_hander[10].hander!=0)
			pin_irq_hander[10].hander(pin_irq_hander[10].para);
	}
	if(EXTI_GetITStatus(EXTI_Line11)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line11);
		if(pin_irq_hander[11].hander!=0)
			pin_irq_hander[11].hander(pin_irq_hander[11].para);
	}
	if(EXTI_GetITStatus(EXTI_Line12)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line12);
		if(pin_irq_hander[12].hander!=0)
			pin_irq_hander[12].hander(pin_irq_hander[12].para);
	}
	if(EXTI_GetITStatus(EXTI_Line13)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line13);
		if(pin_irq_hander[13].hander!=0)
			pin_irq_hander[13].hander(pin_irq_hander[13].para);
	}
	if(EXTI_GetITStatus(EXTI_Line14)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line14);
		if(pin_irq_hander[14].hander!=0)
			pin_irq_hander[14].hander(pin_irq_hander[14].para);
	}
	if(EXTI_GetITStatus(EXTI_Line15)==SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line15);
		if(pin_irq_hander[15].hander!=0)
			pin_irq_hander[15].hander(pin_irq_hander[15].para);
	}
}

static void NVIC_Configuration(IRQn_Type irq)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = irq;//TIM2_IRQn;//
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
int stm32_pin_irq_unregister(rt_device_t dev, rt_base_t pin)
{
	int hander_idx=0,EXTIn_IRQn;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	hander_idx=assert_pin_irq(pin);
	
	if(hander_idx==-1)
			return -1;
		if(pin_irq_hander[hander_idx].index==0)
		{
			return -2;
		}
		
		EXTI_ClearITPendingBit(1<<hander_idx);
		EXTI_InitStructure.EXTI_Line= 1<<hander_idx;//EXTI_Line2 | EXTI_Line3 | EXTI_Line5; 

		EXTI_InitStructure.EXTI_Mode= EXTI_Mode_Interrupt; 

		EXTI_InitStructure.EXTI_Trigger= EXTI_Trigger_Rising;//EXTI_Trigger_Rising_Falling; 
		EXTI_InitStructure.EXTI_LineCmd=DISABLE;                                          

		EXTI_Init(&EXTI_InitStructure);
		EXTI_ClearITPendingBit(1<<hander_idx);
		
switch(hander_idx)
		{
			case 0:
			EXTIn_IRQn=EXTI0_IRQn;
			break;
			case 1:
			EXTIn_IRQn=EXTI1_IRQn;
			break;
			case 2:
			EXTIn_IRQn=EXTI2_IRQn;
			break;
			case 3:
			EXTIn_IRQn=EXTI3_IRQn;
			break;	
			case 4:
			EXTIn_IRQn=EXTI4_IRQn;
			break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			EXTIn_IRQn=EXTI9_5_IRQn;
			EXTI9_5_IRQn_cnt--;
			if(EXTI9_5_IRQn_cnt>0)
				return 0;
			break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			EXTIn_IRQn=EXTI15_10_IRQn;
			EXTI15_10_IRQn_cnt--;
			if(EXTI15_10_IRQn_cnt>0)
				return 0;
			break;
			
		}

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTIn_IRQn;//TIM2_IRQn;//
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
		pin_irq_hander[hander_idx].hander=0;
		pin_irq_hander[hander_idx].index=0;
		pin_irq_hander[hander_idx].para=0;
}
int stm32_pin_irq_register(rt_device_t dev, rt_base_t pin, rt_base_t mode,gpio_irq_hander hander,unsigned long para)
{
	//pin_irq_assert
	//pin_irq_hander
		int hander_idx=0,EXTI_PortSource,EXTIn_IRQn=0;
	   struct pin_index *index;
	
    GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

		hander_idx=assert_pin_irq(pin);
		if(hander_idx==-1)
			return -1;
		if(pin_irq_hander[hander_idx].index!=0)
		{
			return -2;
		}

    index = (struct pin_index *)get_pin(pin);
		pin_irq_hander[hander_idx].index=index;
		pin_irq_hander[hander_idx].hander=hander;
		pin_irq_hander[hander_idx].para=para;
		
		EXTI_ClearITPendingBit(1<<hander_idx);//EXTI_Line1
		
		EXTI_PortSource=0;
		switch((unsigned long)index->gpio)
		{
			
			case (unsigned long)GPIOA:
					EXTI_PortSource=EXTI_PortSourceGPIOA;
				break;
			case (unsigned long)GPIOB:
				EXTI_PortSource=EXTI_PortSourceGPIOB;
			break;
			case (unsigned long)GPIOC:
				EXTI_PortSource=EXTI_PortSourceGPIOC;
			break;
			case (unsigned long)GPIOD:
				EXTI_PortSource=EXTI_PortSourceGPIOD;
			break;
			case (unsigned long)GPIOE:
				EXTI_PortSource=EXTI_PortSourceGPIOE;
			break;
			case (unsigned long)GPIOF:
				EXTI_PortSource=EXTI_PortSourceGPIOF;
			break;
			case (unsigned long)GPIOG:
				EXTI_PortSource=EXTI_PortSourceGPIOG;
			break;
			case (unsigned long)GPIOH:
				EXTI_PortSource=EXTI_PortSourceGPIOH;
			break;
			case (unsigned long)GPIOI:
				EXTI_PortSource=EXTI_PortSourceGPIOI;
			break;
		}
		SYSCFG_EXTILineConfig(EXTI_PortSource,hander_idx);//GPIO_PinSource2
	
		EXTI_InitStructure.EXTI_Line= 1<<hander_idx;//EXTI_Line2 | EXTI_Line3 | EXTI_Line5; 

		EXTI_InitStructure.EXTI_Mode= EXTI_Mode_Interrupt; 

		EXTI_InitStructure.EXTI_Trigger= mode;//EXTI_Trigger_Rising_Falling; 
		EXTI_InitStructure.EXTI_LineCmd=ENABLE;                                          

		EXTI_Init(&EXTI_InitStructure);

		switch(hander_idx)
		{
			case 0:
			EXTIn_IRQn=EXTI0_IRQn;
			break;
			case 1:
			EXTIn_IRQn=EXTI1_IRQn;
			break;
			case 2:
			EXTIn_IRQn=EXTI2_IRQn;
			break;
			case 3:
			EXTIn_IRQn=EXTI3_IRQn;
			break;	
			case 4:
			EXTIn_IRQn=EXTI4_IRQn;
			break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			EXTIn_IRQn=EXTI9_5_IRQn;
			EXTI9_5_IRQn_cnt++;
			if(EXTI9_5_IRQn_cnt>1)
				return 0;
			break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			EXTIn_IRQn=EXTI15_10_IRQn;
			EXTI15_10_IRQn_cnt++;
			if(EXTI15_10_IRQn_cnt>1)
				return 0;
			break;
			
		}
		NVIC_Configuration(EXTIn_IRQn);
}

const static struct rt_pin_ops _stm32_pin_ops =
{
    stm32_pin_mode,
    stm32_pin_write,
    stm32_pin_read,
		stm32_pin_get_write,
		stm32_pin_irq_register,
		stm32_pin_irq_unregister,
};

int stm32_hw_pin_init(void)
{
    rt_device_pin_register("pin", &_stm32_pin_ops, RT_NULL);
		rt_pin_mode(0,PIN_MODE_OUTPUT);
		rt_pin_write(0,0);
    return 0;
}
INIT_BOARD_EXPORT(stm32_hw_pin_init);

#endif
