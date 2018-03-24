#ifndef __PWM_H
#define __PWM_H
#include "sys.h"


void TIM1_PWM_Init(u16 arr,u16 psc);
void TIM1_SET_PWM(u16 arr,u16 psc);

void TIM1_SET_test(void);
void TIM1_SET_on_off(u16 set_time_on,u16 set_time_off,u16 set_time_on2,u16 set_time_off2);

#endif
