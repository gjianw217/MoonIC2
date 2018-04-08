#ifndef __STM32_PWM_H__
#define __STM32_PWM_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>



struct rt_device_pwm_out
{
    rt_uint16_t ch;
    rt_uint16_t flck;
		rt_base_t scale;
		rt_base_t setp;
};

struct rt_device_pwm_mode
{
    rt_uint16_t ch;
    rt_uint16_t mode;
};


struct rt_pwm_ops
{
    void (*pwm_out)(struct rt_device *device, rt_base_t pwm_ch, rt_base_t flck, rt_base_t scale,rt_base_t 	setp);
    int (*pwm_ctl)(struct rt_device *device, rt_base_t pwm_ch,rt_base_t cmd);
		int (*pwm_motor)(struct rt_device *device, rt_base_t pwm_ch,rt_base_t flck,rt_base_t 	setp);
};

#define PWM_CMD_STOP				0x00
#define PWM_CMD_RESTART			0x01
#define PWM_CMD_MOTOR_END		0x02

struct rt_device_pwm
{
    struct rt_device parent;
    const struct rt_pwm_ops *ops;
};

int rt_device_pwm_register(const char *name, const struct rt_pwm_ops *ops, void *user_data);

void rt_pwm_out(rt_base_t pwm, rt_base_t value, rt_base_t scale,rt_base_t 	setp);
int  rt_pwm_ctl(rt_base_t pwm,rt_base_t cmd);
void rt_pwm_motor(rt_base_t motor, rt_base_t dir,rt_base_t en,rt_base_t fclk,rt_base_t 	setp);

int stm32_hw_pwm_init(void);

#endif
