#include "stm32_pwm.h"
#include <board.h>
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#ifdef RT_USING_FINSH
#include <finsh.h>
#endif

static struct rt_device_pwm _hw_pwm;

struct pwm_cfg
{
		rt_base_t fclk;
		rt_base_t scale;
		rt_base_t setp;
		rt_base_t setp_cnt;
	  rt_base_t init;
};

struct pwm_setp_cfg
{
		rt_base_t fclk[21];
		rt_base_t setp[21];
		rt_base_t setp_indx;
		
};

struct pwm_index
{
    int index;
    uint32_t rcc;
    GPIO_TypeDef *gpio;
    uint32_t pin;
		uint32_t af_src;	//GPIO_PinSource3
		uint32_t af;	//GPIO_AF_TIM14

	
		TIM_TypeDef *tim;
		uint32_t rcc_tim;
		uint16_t apbn;
		uint16_t ch;
		uint32_t irq;
		
};

struct pwm_cfg pwm_ch_cfg[9];
struct pwm_setp_cfg pwm_setp_ch_cfg[9];
void stm32_pwm_out(rt_device_t dev, rt_base_t pwm_ch, rt_base_t flck, rt_base_t scale,rt_base_t 	setp);
void stm32_pwm_reout(rt_device_t dev, rt_base_t pwm_ch, rt_base_t flck, rt_base_t scale,rt_base_t 	setp);
static rt_size_t _pwm_out(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_device_pwm_out *status;
    struct rt_device_pwm *pwm = (struct rt_device_pwm *)dev;

    /* check parameters */
    RT_ASSERT(pwm != RT_NULL);

    status = (struct rt_device_pwm_out *) buffer;
    if (status == RT_NULL || size != sizeof(*status)) return 0;

    pwm->ops->pwm_out(dev, (rt_base_t)status->ch, (rt_base_t)status->flck,(rt_base_t)status->scale,(rt_base_t)status->setp);

    return size;
}

static rt_err_t  _pwm_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    struct rt_device_pwm_mode *mode;
    struct rt_device_pwm *pwm = (struct rt_device_pwm *)dev;

    /* check parameters */
    RT_ASSERT(pwm != RT_NULL);

    mode = (struct rt_device_pwm_mode *) args;
    if (mode == RT_NULL) return -RT_ERROR;

    pwm->ops->pwm_ctl(dev, (rt_base_t)mode->ch, (rt_base_t)mode->mode);

    return 0;
}

int rt_device_pwm_register(const char *name, const struct rt_pwm_ops *ops, void *user_data)
{
    _hw_pwm.parent.type         = RT_Device_Class_Miscellaneous;
    _hw_pwm.parent.rx_indicate  = RT_NULL;
    _hw_pwm.parent.tx_complete  = RT_NULL;

    _hw_pwm.parent.init         = RT_NULL;
    _hw_pwm.parent.open         = RT_NULL;
    _hw_pwm.parent.close        = RT_NULL;
    _hw_pwm.parent.read         = RT_NULL;
    _hw_pwm.parent.write        = _pwm_out;
    _hw_pwm.parent.control      = _pwm_control;

    _hw_pwm.ops                 = ops;
    _hw_pwm.parent.user_data    = user_data;

    /* register a character device */
    rt_device_register(&_hw_pwm.parent, "pwm", RT_DEVICE_FLAG_RDWR);

    return 0;
}

/* RT-Thread Hardware PIN APIs */

void rt_pwm_motor(rt_base_t motor, rt_base_t dir,rt_base_t en,rt_base_t fclk,rt_base_t 	setp)
{
	switch(motor)
	{
		case 0:
		rt_pin_mode(1,PIN_MODE_OUTPUT);
		rt_pin_write(1,!en);
		rt_pin_mode(2,PIN_MODE_OUTPUT);
		rt_pin_write(2,dir);
		_hw_pwm.ops->pwm_motor(&_hw_pwm.parent, 2, fclk,setp);
		break;
		case 1:
		rt_pin_mode(4,PIN_MODE_OUTPUT);
		rt_pin_write(4,!en);
		rt_pin_mode(5,PIN_MODE_OUTPUT);
		rt_pin_write(5,dir);
		_hw_pwm.ops->pwm_motor(&_hw_pwm.parent, 5, fclk,setp);
		break;
		case 2:
		rt_pin_mode(7,PIN_MODE_OUTPUT);
		rt_pin_write(7,!en);
		rt_pin_mode(8,PIN_MODE_OUTPUT);
		rt_pin_write(8,dir);
		_hw_pwm.ops->pwm_motor(&_hw_pwm.parent, 8, fclk,setp);
		break;
	}
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pwm_motor, pwmMotor, control Motor stop/restart);

void rt_pwm_out(rt_base_t pwm, rt_base_t value, rt_base_t scale,rt_base_t 	setp)
{
    _hw_pwm.ops->pwm_out(&_hw_pwm.parent, pwm, value,scale,setp);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pwm_out, pwmOut, write value to hardware pwm);

int  rt_pwm_ctl(rt_base_t pwm,rt_base_t cmd)
{
    return _hw_pwm.ops->pwm_ctl(&_hw_pwm.parent, pwm,cmd);
}
FINSH_FUNCTION_EXPORT_ALIAS(rt_pwm_ctl, pwmCtl, control pwm stop/restart);

/********************************************* device init************************************/

void pwm_irq_handler(const struct pwm_index *index,struct pwm_cfg *cfg)
{
	uint16_t it_flg=0;
	switch(index->ch)
	{
		case 0x01:
			it_flg=TIM_IT_CC1;
		break;
		case 0x02:
			it_flg=TIM_IT_CC2;
		break;
		case 0x03:
			it_flg=TIM_IT_CC3;
		break;
		case 0x04:
			it_flg=TIM_IT_CC4;
		break;
	}
		if(TIM_GetITStatus(index->tim, it_flg) == SET)
    {
        TIM_ClearITPendingBit(index->tim, it_flg );
  			cfg->setp_cnt++;
	  		if(cfg->setp <= cfg->setp_cnt )
	  		{
					struct pwm_setp_cfg *cfg=&pwm_setp_ch_cfg[index->index];
					if(cfg->fclk[0]==0 && cfg->setp==0){
						memset(cfg,0,sizeof(struct pwm_setp_cfg));
						TIM_Cmd(index->tim,DISABLE);
						TIM_ITConfig(index->tim, it_flg, DISABLE);
						if(index->tim==TIM1 ||index->tim==TIM8) 
								TIM_CtrlPWMOutputs(index->tim,DISABLE);
					}
					else
					{
						if(cfg->setp_indx>20)
						{
								memset(cfg,0,sizeof(struct pwm_setp_cfg));
								TIM_Cmd(index->tim,DISABLE);
								TIM_ITConfig(index->tim, it_flg, DISABLE);
								if(index->tim==TIM1 ||index->tim==TIM8) 
										TIM_CtrlPWMOutputs(index->tim,DISABLE);
						}
						else
						{
							cfg->setp_indx=cfg->setp_indx+1;
							stm32_pwm_reout(&_hw_pwm.parent, index->index, cfg->fclk[cfg->setp_indx], 50,cfg->setp[cfg->setp_indx]);
						}
					}
 			 }
		}
}
/*
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
*/

static const struct  pwm_index pwms[] =
{
	{ 0, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_0,GPIO_PinSource0,GPIO_AF_TIM5, TIM5, RCC_APB1Periph_TIM5, 1,1,TIM5_IRQn},
	{ 1, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_3,GPIO_PinSource3,GPIO_AF_TIM2, TIM2, RCC_APB1Periph_TIM2, 1,4,TIM2_IRQn},
	{ 2, RCC_AHB1Periph_GPIOA, GPIOA, GPIO_Pin_8,GPIO_PinSource8,GPIO_AF_TIM1, TIM1, RCC_APB2Periph_TIM1, 2,1,TIM1_CC_IRQn},
	{ 3, RCC_AHB1Periph_GPIOB, GPIOB, GPIO_Pin_8,GPIO_PinSource8,GPIO_AF_TIM4, TIM4, RCC_APB1Periph_TIM4, 1,3,TIM4_IRQn},
	{ 4, RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_6,GPIO_PinSource6,GPIO_AF_TIM3, TIM3, RCC_APB1Periph_TIM3, 1,1,TIM3_IRQn},
	{ 5, RCC_AHB1Periph_GPIOC, GPIOC, GPIO_Pin_7,GPIO_PinSource7,GPIO_AF_TIM8, TIM8, RCC_APB2Periph_TIM8, 2,2,TIM8_CC_IRQn},
	{ 6, RCC_AHB1Periph_GPIOE, GPIOE, GPIO_Pin_6,GPIO_PinSource6,GPIO_AF_TIM9, TIM9, RCC_APB2Periph_TIM9, 2,2,TIM1_BRK_TIM9_IRQn},
	{ 7, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_6,GPIO_PinSource6,GPIO_AF_TIM10,TIM10,RCC_APB2Periph_TIM10,2,1,TIM1_UP_TIM10_IRQn},
	{ 8, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_7,GPIO_PinSource7,GPIO_AF_TIM11,TIM11,RCC_APB2Periph_TIM11,2,1,TIM1_TRG_COM_TIM11_IRQn},
	//{10, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_8,GPIO_PinSource8,GPIO_AF_TIM13,TIM13,RCC_APB1Periph_TIM13,1,1,TIM8_UP_TIM13_IRQn},
	//{11, RCC_AHB1Periph_GPIOF, GPIOF, GPIO_Pin_9,GPIO_PinSource9,GPIO_AF_TIM14,TIM14,RCC_APB1Periph_TIM14,1,1,TIM8_TRG_COM_TIM14_IRQn},
};


//index 0;
void TIM5_IRQHandler(void)
{
	pwm_irq_handler(&pwms[0],&pwm_ch_cfg[0]);
}

//index 1;
void TIM2_IRQHandler(void)
{
	pwm_irq_handler(&pwms[1],&pwm_ch_cfg[1]);
}

//index 2;
void TIM1_CC_IRQHandler(void)
{
	pwm_irq_handler(&pwms[2],&pwm_ch_cfg[2]);
}

//index 3;
void TIM4_IRQHandler(void)
{
	pwm_irq_handler(&pwms[3],&pwm_ch_cfg[3]);
}

//index 4;
void TIM3_IRQHandler(void)
{
		pwm_irq_handler(&pwms[4],&pwm_ch_cfg[4]);
}

//index 5;
void TIM8_CC_IRQHandler(void)
{
	pwm_irq_handler(&pwms[5],&pwm_ch_cfg[5]);
}

//index 6;
void TIM1_BRK_TIM9_IRQHandler(void)
{
	pwm_irq_handler(&pwms[6],&pwm_ch_cfg[6]);
}
//index 7;
void TIM1_UP_TIM10_IRQHandler(void)
{
	pwm_irq_handler(&pwms[7],&pwm_ch_cfg[7]);
}

//index 8;
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
	pwm_irq_handler(&pwms[8],&pwm_ch_cfg[8]);
}
//index 7;
/*void TIM8_UP_TIM13_IRQHandler(void)
{
	//pwm_irq_handler(&pwms[7],&pwm_ch_cfg[7]);
}

//index 8;
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	//pwm_irq_handler(&pwms[8],&pwm_ch_cfg[8]);
}
*/

#define ITEM_NUM(items) sizeof(items)/sizeof(items[0])
const struct pwm_index *get_pwm_ch(uint8_t pin)
{
    const struct pwm_index *index;

    if (pin < ITEM_NUM(pwms))
    {
        index = &pwms[pin];
    }
    else
    {
        index = RT_NULL;
    }

    return index;
};




static void NVIC_Configuration(const struct pwm_index *index)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = index->irq;//TIM2_IRQn;//
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void stm32_pwm_out(rt_device_t dev, rt_base_t pwm_ch, rt_base_t flck, rt_base_t scale,rt_base_t 	setp)
{
		const struct pwm_index *index;
		GPIO_InitTypeDef GPIO_InitStructure;
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		TIM_OCInitTypeDef  TIM_OCInitStructure;
		rt_base_t cnt=0;
	
		if(flck>=2000000 || flck<0)
		{
			return;
		}
		if(scale>100 || scale<0)
		{
			return;
		}
    index = get_pwm_ch(pwm_ch);
    if (index == RT_NULL)
    {
        return;
    }
		
	cnt=28000000/flck;
		
	pwm_ch_cfg[pwm_ch].fclk=flck;
	pwm_ch_cfg[pwm_ch].scale=scale;
	pwm_ch_cfg[pwm_ch].setp=setp;	
	pwm_ch_cfg[pwm_ch].setp_cnt=0;		
	 
	if(pwm_ch_cfg[pwm_ch].init==0)
	{
		RCC_AHB1PeriphClockCmd(index->rcc, ENABLE); 	//使能PORT_x时钟	
		if(index->apbn==1)
		{
			RCC_APB1PeriphClockCmd(index->rcc_tim,ENABLE);  	//TIM_x时钟使能 
		}
		else
		{
			RCC_APB2PeriphClockCmd(index->rcc_tim,ENABLE);  
		}
		
		GPIO_PinAFConfig(index->gpio,index->af_src,index->af); //GPIO_x复用为定时器
		
		GPIO_InitStructure.GPIO_Pin = index->pin;           //GPIO_x
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		//速度2MHz
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
		GPIO_Init(index->gpio,&GPIO_InitStructure);         //初始化gpio
		
		TIM_DeInit(index->tim);
		pwm_ch_cfg[pwm_ch].init=1;
	}
	TIM_TimeBaseStructure.TIM_Prescaler=5;  //定时器分频 (n=0~~2Mhz) 84M/6=14M
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=cnt;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	//TIMx->ARR = TIM_TimeBaseInitStruct->TIM_Period ;
	TIM_TimeBaseInit(index->tim,&TIM_TimeBaseStructure);//初始化定时器14
	
	//初始化TIM_x Channel_n PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性低
	
	TIM_OCInitStructure.TIM_Pulse=(cnt*scale)/100;
	//TIMx->CCR1 = TIM_OCInitStruct->TIM_Pulse;
	if(index->tim==TIM1 ||index->tim==TIM8) 
	{
		TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Disable;
		TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;
		TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
		TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;
	}
	
	switch(index->ch)
	{
		case 0x01:
			TIM_OC1Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			TIM_OC1PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			TIM_ClearFlag(index->tim, TIM_FLAG_CC1);
			TIM_ITConfig(index->tim, TIM_IT_CC1, ENABLE);
		break;
		case 0x02:
			TIM_OC2Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			TIM_OC2PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			TIM_ClearFlag(index->tim, TIM_FLAG_CC2);
			TIM_ITConfig(index->tim, TIM_IT_CC2, ENABLE);

		break;
		case 0x03:
			TIM_OC3Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			TIM_OC3PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			TIM_ClearFlag(index->tim, TIM_FLAG_CC3);
			TIM_ITConfig(index->tim, TIM_IT_CC3, ENABLE);

		break;
		case 0x04:
			TIM_OC4Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			TIM_OC4PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			TIM_ClearFlag(index->tim, TIM_FLAG_CC4);
			TIM_ITConfig(index->tim, TIM_IT_CC4, ENABLE);
			
		break;
	}
	NVIC_Configuration(index);
  TIM_ARRPreloadConfig(index->tim,ENABLE);//ARPE使能 
	if(index->tim==TIM1 ||index->tim==TIM8) 
			TIM_CtrlPWMOutputs(index->tim,ENABLE);
	//TIM_CtrlPWMOutputs(index->tim,ENABLE); //t1 &t8
	
	TIM_Cmd(index->tim, ENABLE);  //使能TIM_x
		
}

void stm32_pwm_reout(rt_device_t dev, rt_base_t pwm_ch, rt_base_t flck, rt_base_t scale,rt_base_t 	setp)
{
		const struct pwm_index *index;
//		GPIO_InitTypeDef GPIO_InitStructure;
//		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//		TIM_OCInitTypeDef  TIM_OCInitStructure;
		rt_base_t cnt=0;
	
		if(flck>=2000000 || flck<0)
		{
			return;
		}
		if(scale>100 || scale<0)
		{
			return;
		}
    index = get_pwm_ch(pwm_ch);
    if (index == RT_NULL)
    {
        return;
    }
		
	cnt=28000000/flck;
		
	pwm_ch_cfg[pwm_ch].fclk=flck;
	pwm_ch_cfg[pwm_ch].scale=scale;
	pwm_ch_cfg[pwm_ch].setp=setp;	
	pwm_ch_cfg[pwm_ch].setp_cnt=0;		
	 

	//TIM_TimeBaseStructure.TIM_Prescaler=5;  //定时器分频 (n=0~~2Mhz) 84M/6=14M
	//TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	//TIM_TimeBaseStructure.TIM_Period=cnt;   //自动重装载值
	//TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	//TIMx->ARR = TIM_TimeBaseInitStruct->TIM_Period ;
		index->tim->ARR=cnt;
	//TIM_TimeBaseInit(index->tim,&TIM_TimeBaseStructure);//初始化定时器14
	
	//初始化TIM_x Channel_n PWM模式	 
	//TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
 	//TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	//TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性低
	
	//TIM_OCInitStructure.TIM_Pulse=(cnt*scale)/100;
	//TIMx->CCR1 = TIM_OCInitStruct->TIM_Pulse;
	
	/*if(index->tim==TIM1 ||index->tim==TIM8) 
	{
		TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Disable;
		TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;
		TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
		TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;
	}*/
	
	switch(index->ch)
	{
		case 0x01:
			//TIM_OC1Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			
			//TIM_OC1PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			//TIM_ClearFlag(index->tim, TIM_FLAG_CC1);
			index->tim->SR = (uint16_t)~TIM_FLAG_CC1;
			index->tim->CCR1 = (cnt*scale)/100;
			TIM_ITConfig(index->tim, TIM_IT_CC1, ENABLE);
		break;
		case 0x02:
			//TIM_OC2Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			//TIM_OC2PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			//TIM_ClearFlag(index->tim, TIM_FLAG_CC2);
			index->tim->SR = (uint16_t)~TIM_FLAG_CC2;
			index->tim->CCR2 = (cnt*scale)/100;
			TIM_ITConfig(index->tim, TIM_IT_CC2, ENABLE);

		break;
		case 0x03:
			//TIM_OC3Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			//TIM_OC3PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			//TIM_ClearFlag(index->tim, TIM_FLAG_CC3);
			index->tim->SR = (uint16_t)~TIM_FLAG_CC3;
			index->tim->CCR3 = (cnt*scale)/100;
			TIM_ITConfig(index->tim, TIM_IT_CC3, ENABLE);

		break;
		case 0x04:
			//TIM_OC4Init(index->tim, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM1 4OC1
			//TIM_OC4PreloadConfig(index->tim, TIM_OCPreload_Enable);  //使能TIM14在CCR1上的预装载寄存器
			//TIM_ClearFlag(index->tim, TIM_FLAG_CC4);
		
			index->tim->SR = (uint16_t)~TIM_FLAG_CC4;
			index->tim->CCR4 = (cnt*scale)/100;
			TIM_ITConfig(index->tim, TIM_IT_CC4, ENABLE);
			
		break;
	}
	//NVIC_Configuration(index);
  TIM_ARRPreloadConfig(index->tim,ENABLE);//ARPE使能 
	if(index->tim==TIM1 ||index->tim==TIM8) 
			TIM_CtrlPWMOutputs(index->tim,ENABLE);
	//TIM_CtrlPWMOutputs(index->tim,ENABLE); //t1 &t8
	
	TIM_Cmd(index->tim, ENABLE);  //使能TIM_x
		
}



int stm32_pwm_ctl(rt_device_t dev, rt_base_t pwm_ch,rt_base_t cmd)
{
    const struct pwm_index *index;



    index = get_pwm_ch(pwm_ch);
    if (index == RT_NULL)
    {
        return -1;
    }
		switch(cmd)
		{
			case PWM_CMD_RESTART:
			{
				if((pwm_ch_cfg[pwm_ch].fclk==00) || (pwm_ch_cfg[pwm_ch].scale==0))
				{
					return -2;
				}
				stm32_pwm_out(dev, pwm_ch, pwm_ch_cfg[pwm_ch].fclk, pwm_ch_cfg[pwm_ch].scale,pwm_ch_cfg[pwm_ch].setp);
			}break;
			case PWM_CMD_MOTOR_END:
			{
				
				pwm_setp_ch_cfg[pwm_ch].setp_indx=11;//end && stop
				stm32_pwm_reout(dev, pwm_ch, pwm_setp_ch_cfg[pwm_ch].fclk[pwm_setp_ch_cfg[pwm_ch].setp_indx], 50,pwm_setp_ch_cfg[pwm_ch].setp[pwm_setp_ch_cfg[pwm_ch].setp_indx]);
				
				//stm32_pwm_reout(&_hw_pwm.parent, index->index, cfg->fclk[cfg->setp_indx], 50,cfg->setp[cfg->setp_indx]);
			}
			break;
			default:
			{
				TIM_DeInit(index->tim);
				if(index->apbn==1)
				{
					RCC_APB1PeriphClockCmd(index->rcc_tim,DISABLE);  	//TIM_x时钟使能 
				}
				else
				{
					RCC_APB2PeriphClockCmd(index->rcc_tim,DISABLE);  
				}
				memset(&pwm_setp_ch_cfg[pwm_ch],0,sizeof(struct pwm_setp_cfg));
			}
			break;
		}
    return 0;
}

int stm32_pwm_motor(struct rt_device *device, rt_base_t pwm_ch,rt_base_t flck,rt_base_t 	setp)
{
	//
	struct pwm_setp_cfg *cfg=&pwm_setp_ch_cfg[pwm_ch];
	rt_base_t clk=flck/11;
	int i=0;
	rt_base_t cclk=clk;
	rt_base_t st=setp*0.01;
	rt_base_t stm=setp*0.8;
	
	if(setp>10000)
	{
		st=100;
		stm=setp-2000;
	}
	/*else if(setp<300)
	{
		memset(cfg,0,sizeof(struct pwm_setp_cfg));
		stm32_pwm_out(device, pwm_ch, flck, 50,setp);
		return 0;
	}*/

	for(i=0;i<10;i++)
	{
		cfg->fclk[i]=cclk;
		cfg->setp[i]=st;
		cclk=cclk+clk;
	}
		cfg->fclk[10]=flck;
		cfg->setp[10]=stm;
	
	cclk=flck-clk;
	for(i=11;i<21;i++)
	{
		cfg->fclk[i]=cclk;
		cfg->setp[i]=st;
		cclk=cclk-clk;
	}
	
	cfg->setp_indx=0;
	stm32_pwm_out(device, pwm_ch, cfg->fclk[cfg->setp_indx], 50,cfg->setp[cfg->setp_indx]);
	return 0;
}

int stm32_pwm_test(struct rt_device *device, rt_base_t pwm_ch,rt_base_t flck,rt_base_t 	setp)
{
	//
	struct pwm_setp_cfg *cfg=&pwm_setp_ch_cfg[pwm_ch];
	int i=0;
	/*rt_base_t clk=flck/6;
	
	rt_base_t cclk=clk;
	rt_base_t st=setp*0.01;
	rt_base_t stm=setp*0.9;
	
	if(setp>2*5000)
	{
		st=500;
		stm=setp-5000;
	}
	*/
	
	/*else if(setp<300)
	{
		memset(cfg,0,sizeof(struct pwm_setp_cfg));
		stm32_pwm_out(device, pwm_ch, flck, 50,setp);
		return 0;
	}*/

	for(i=0;i<11;i++)
	{
		cfg->fclk[i]=flck;
		cfg->setp[i]=setp;
		//cclk=cclk+clk;
	}

	
	cfg->setp_indx=0;
	stm32_pwm_out(device, pwm_ch, cfg->fclk[cfg->setp_indx], 50,cfg->setp[cfg->setp_indx]);
	return 0;
}
const static struct rt_pwm_ops _stm32_pwm_ops =
{

    stm32_pwm_out,
    stm32_pwm_ctl,
		stm32_pwm_motor,
	//stm32_pwm_test,
};

int stm32_hw_pwm_init(void)
{
	memset(pwm_ch_cfg,0,9*sizeof(struct pwm_cfg));
  rt_device_pwm_register("pwm", &_stm32_pwm_ops, RT_NULL);

		//rt_pwm_out(1,1000,50);
    return 0;
}
INIT_BOARD_EXPORT(stm32_hw_pwm_init);







