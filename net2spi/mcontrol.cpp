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

#include "mcontrol.h"



MControl::MControl()
{	
	memset(this->arm_dev_reg,0,ARM_DEV_REG_MAX*sizeof(unsigned int));
	control_status=CONTROL_IDLE;
	runStop=false;
}

MControl::~MControl()
{
	pthread_mutex_destroy(&this->lock_run);
	pthread_cond_destroy(&this->lock_cond);
	pthread_rwlock_destroy(&this->lock_data);
	//未pthread_mutex_init 不会引起至命错误。
	
}

int MControl::initialize(MSpi *spi)
{
	if(spi==NULL)
		return -1;
	
	this->spi=spi;

	
	if(pthread_rwlock_init(&this->lock_data,NULL)!=0)
	{
		dbg0("err:init lock_data");
		return -2;
	}
	
	if(pthread_mutex_init(&this->lock_run,NULL)!=0)
	{
		dbg0("err:init lock_run");
		goto control_err1;
	}
	
	if(pthread_cond_init(&this->lock_cond,NULL)!=0)
	{
		dbg0("err:init lock_cond");
		goto control_err2;
	}
	
	return 0;
	
control_err2:		
	pthread_mutex_destroy(&this->lock_run);
	
control_err1:
	pthread_rwlock_destroy(&this->lock_data);
	
	return -2;
}

int MControl::stop()
{
	runStop=true;
	change_status(CONTROL_EXIT);
	return 0;
}

void MControl::run()
{
	int ret;
	while((!sigexit)&&(!runStop))
	{
		//dbg3("control status %x %x\r\n",pthread_self(), control_status);
		
		switch(control_status)
		{
			case CONTROL_IDLE:
			{
				ret=waiting_event();
			}break;
			case CONTROL_MOVE:
			{
			}break;
			case CONTROL_EXIT:
			{
				runStop=true;
			}break;
		}
		
		usleep(100);
	}
}
int MControl::get_reg(int addr,unsigned int *val){
	
	if(addr<0 ||addr >=ARM_DEV_REG_MAX)
		return -1;
	
	*val=this->arm_dev_reg[addr];
	
	return 0;
}
int MControl::set_reg(int addr,unsigned int val)
{
	control_status_t status=CONTROL_IDLE;
	
	if(addr<0 ||addr >=ARM_DEV_REG_MAX)
		return -1;
	
	this->arm_dev_reg[addr]=val;
	switch(addr)
	{
		case 0x00:
		case 0x01:
			status=CONTROL_MOVE;
			break;
			
	}
	change_status(status);

	
	return 0;	
}

int MControl::waiting_event()
{
	pthread_mutex_lock(&lock_run);
	return pthread_cond_wait(&lock_cond,&lock_run);
}

int MControl::wake_up()
{
	pthread_mutex_unlock(&lock_run);//解锁  
    pthread_cond_broadcast(&lock_cond);//唤醒等待该条件的所有线程  
}

int MControl::change_status(control_status_t status)
{
	if(status>=CONTROL_MAX)
		return -1;
	
	this->control_status=status;
	//if(status!=STATUS_IDLE)
		wake_up();
	
	return 0;
}
