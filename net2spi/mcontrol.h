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

#ifndef MCONTROL_H
#define MCONTROL_H
#include "mthread.h"
#include "stdio.h"
#include "string.h"
#include <iostream>

#include "mspi.h"
#include "mconfig.h"

#define ARM_DEV_REG_MAX 10

typedef enum{
	CONTROL_IDLE=0x00,
	CONTROL_MOVE,
	CONTROL_EXIT,
	CONTROL_MAX,
}control_status_t;


using namespace std;


class MControl : public MThread
{
public:
	MControl();
	~MControl();
	int initialize(MSpi *spi);
	virtual void run();
	int get_reg(int addr,unsigned int *val);
	int set_reg(int addr,unsigned int val);
	int stop();
// 	
private:
	unsigned int arm_dev_reg[ARM_DEV_REG_MAX];
/**************************************************
 * 0x00----move start,  0x01----move stop
 * 
 * 
 * 
 **************************************************/
	
	pthread_rwlock_t lock_data;
	pthread_mutex_t  lock_run;
	pthread_cond_t   lock_cond;
	control_status_t control_status;
	int waiting_event();
	int wake_up();
	int change_status(control_status_t status);
	MSpi	 *spi;
	bool runStop; 
	
};

#endif // MCONTROL_H
