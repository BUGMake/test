#include "pwm.h"
#include "led.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//PWM  驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  


//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM1_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //使能GPIO外设时钟使能
	                                                                     	

  //设置该引脚为复用输出功能,输出TIM1 CH1的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx

  TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE 主输出使能	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1预装载使能	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM1, ENABLE);  //使能TIM1   
	
	//罗校敬
	TIM_SetCompare1(TIM1,(arr+1)/2);//调整PWM的占空比 = 50%
}

//罗校敬
void TIM1_SET_PWM(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_SetCompare1(TIM1,(arr+1)/2);//调整PWM的占空比 = 50%
}

//罗校敬
static u16 time_on = 20;
static u16 time_off = 100;
static u16 time_on2 = 40;
static u16 time_off2 = 100;

void TIM1_SET_test(void)//set_on_off_test -f 20 100 40 100
{
//	TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭
//	delay_us(time_off);//160us = 1
//	TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能		
//	delay_us(time_on);//260us = 0
	
	TIM1_SET_PWM(199,0);//PWM输出：PA8，不分频，PWM频率=72000/(999+1)=72Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_off);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_on);//260us = 0	
	TIM1_SET_PWM(199,0);//PWM输出：PA8，不分频，PWM频率=72000/(999+1)=72Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_off);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_on);//260us = 0	

	TIM1_SET_PWM(199,0);//PWM输出：PA8，不分频，PWM频率=72000/(999+1)=72Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_off2);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_on2);//260us = 0		
	TIM1_SET_PWM(199,0);//PWM输出：PA8，不分频，PWM频率=72000/(999+1)=72Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_off2);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	delay_us(time_on2);//260us = 0		
}
void TIM1_SET_on_off(u16 set_time_on,u16 set_time_off,u16 set_time_on2,u16 set_time_off2)
{
	time_on = set_time_on;
	time_off = set_time_off;
	time_on2 = set_time_on2;
	time_off2 = set_time_off2;	
}

