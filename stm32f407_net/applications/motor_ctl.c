#include "motor_ctl.h"

#define MOVE_ZERO 							42
#define MOVE_LIMITE_START				43
#define MOVE_LIMITE_END					44
#define LR_LIMITE_START					45
#define LR_LIMITE_END						46
#define UD_LIMITE_START					47
#define UD_LIMITE_END						48


#define MOVE_MAX_CNT_STEP 			100000
#define MOVE_MIN_CNT_STEP 			-10
#define LR_MAX_CNT_STEP 				100000
#define UD_MAX_CNT_STEP 				100000

 long motor0_current_step=0;
 long motor1_current_step=0;
 long motor2_current_step=0;

int motor_io_irq_hander(unsigned long par)
{
	rt_kprintf("rq %d!\r\n",par);
	if(par==MOVE_ZERO || par==MOVE_LIMITE_START|| par==MOVE_LIMITE_END)
	{
		rt_pwm_ctl(2,PWM_CMD_MOTOR_END);//PWM_CMD_STOP  PWM_CMD_MOTOR_END
		if(par==MOVE_ZERO)
			motor0_current_step=0;
		else if(par==MOVE_LIMITE_START)
			motor0_current_step=MOVE_MIN_CNT_STEP;
		else
			motor0_current_step=MOVE_MAX_CNT_STEP;
	}
	if(par==LR_LIMITE_START || par==LR_LIMITE_START)
	{
		rt_pwm_ctl(5,PWM_CMD_STOP);//PWM_CMD_STOP
		if(par==LR_LIMITE_START)
			motor1_current_step=0;
		else
			motor1_current_step=LR_MAX_CNT_STEP;
		
	}
	if(par==UD_LIMITE_START || par==UD_LIMITE_START)
	{
		rt_pwm_ctl(8,PWM_CMD_STOP);//PWM_CMD_STOP
		if(par==UD_LIMITE_START)
			motor2_current_step=0;
		else
			motor2_current_step=UD_MAX_CNT_STEP;
	}
	
	return 0;
}


 int motor_init_irq(void)
 {
		rt_pin_mode(MOVE_ZERO, PIN_MODE_INPUT);
		rt_pin_mode(MOVE_LIMITE_START, PIN_MODE_INPUT);
		rt_pin_mode(MOVE_LIMITE_END, PIN_MODE_INPUT);
		rt_pin_mode(LR_LIMITE_START, PIN_MODE_INPUT);
		rt_pin_mode(LR_LIMITE_END, PIN_MODE_INPUT);
		rt_pin_mode(UD_LIMITE_START, PIN_MODE_INPUT);
		rt_pin_mode(UD_LIMITE_END, PIN_MODE_INPUT);

		rt_pin_irq_register(MOVE_ZERO,EXTI_Trigger_Falling,motor_io_irq_hander,MOVE_ZERO);
		rt_pin_irq_register(MOVE_LIMITE_START,EXTI_Trigger_Falling,motor_io_irq_hander,MOVE_LIMITE_START);
		rt_pin_irq_register(MOVE_LIMITE_END,EXTI_Trigger_Falling,motor_io_irq_hander,MOVE_LIMITE_END);
		rt_pin_irq_register(LR_LIMITE_START,EXTI_Trigger_Falling,motor_io_irq_hander,LR_LIMITE_START);
		rt_pin_irq_register(LR_LIMITE_END,EXTI_Trigger_Falling,motor_io_irq_hander,LR_LIMITE_END);
		rt_pin_irq_register(UD_LIMITE_START,EXTI_Trigger_Falling,motor_io_irq_hander,UD_LIMITE_START);
		rt_pin_irq_register(UD_LIMITE_END,EXTI_Trigger_Falling,motor_io_irq_hander,UD_LIMITE_END);
 }



int motor_ctl(rt_base_t motor, rt_base_t dir,rt_base_t 	setp)
{
	rt_base_t fclk=0;
	
	switch(motor)
	{
		case 0:
			if(motor0_current_step+setp>MOVE_MAX_CNT_STEP-10)
				return -1;
			if(motor0_current_step+setp<MOVE_MIN_CNT_STEP+5)
				return -2;
			motor0_current_step=motor0_current_step+setp;
			
			if(setp>(MOVE_MAX_CNT_STEP*0.4))
				fclk=16000;
			else if(setp>(MOVE_MAX_CNT_STEP*0.1))
				fclk=8000;
			else 
				fclk=4000;
			

			break;
		case 1:
			if(motor1_current_step+setp>LR_MAX_CNT_STEP-10)
					return -3;
			if(motor1_current_step+setp<0)
					return -4;
			motor1_current_step=motor1_current_step+setp;
			
			if(setp>(LR_MAX_CNT_STEP*0.4))
				fclk=16000;
			else if(setp>(LR_MAX_CNT_STEP*0.1))
				fclk=8000;
			else 
				fclk=4000;
			

			break;
		case 2:
			if(motor2_current_step+setp>UD_MAX_CNT_STEP-10)
					return -5;
			if(motor2_current_step+setp<0)
					return -6;
			motor2_current_step=motor2_current_step+setp;
		
			if(setp>(UD_MAX_CNT_STEP*0.4))
				fclk=16000;
			else if(setp>(UD_MAX_CNT_STEP*0.1))
				fclk=8000;
			else 
				fclk=4000;
			

			break;
		default:
			return -100;
	}
	
	
	
	
	 rt_pwm_motor(motor, dir, 1, fclk,setp);
		return 0;
}

