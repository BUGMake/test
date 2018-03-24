#include "pwm.h"
#include "led.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//PWM  ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  


//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM1_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);// 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  //ʹ��GPIO����ʱ��ʹ��
	                                                                     	

  //���ø�����Ϊ�����������,���TIM1 CH1��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx

  TIM_CtrlPWMOutputs(TIM1,ENABLE);	//MOE �����ʹ��	

	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //CH1Ԥװ��ʹ��	 
	
	TIM_ARRPreloadConfig(TIM1, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIM1   
	
	//��У��
	TIM_SetCompare1(TIM1,(arr+1)/2);//����PWM��ռ�ձ� = 50%
}

//��У��
void TIM1_SET_PWM(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_SetCompare1(TIM1,(arr+1)/2);//����PWM��ռ�ձ� = 50%
}

//��У��
static u16 time_on = 20;
static u16 time_off = 100;
static u16 time_on2 = 40;
static u16 time_off2 = 100;

void TIM1_SET_test(void)//set_on_off_test -f 20 100 40 100
{
//	TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�
//	delay_us(time_off);//160us = 1
//	TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��		
//	delay_us(time_on);//260us = 0
	
	TIM1_SET_PWM(199,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(999+1)=72Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_off);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_on);//260us = 0	
	TIM1_SET_PWM(199,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(999+1)=72Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_off);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_on);//260us = 0	

	TIM1_SET_PWM(199,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(999+1)=72Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_off2);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_on2);//260us = 0		
	TIM1_SET_PWM(199,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(999+1)=72Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_off2);//160us = 1
	TIM1_SET_PWM(711,0);
	delay_us(10);
	TIM1_SET_PWM(675,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	delay_us(time_on2);//260us = 0		
}
void TIM1_SET_on_off(u16 set_time_on,u16 set_time_off,u16 set_time_on2,u16 set_time_off2)
{
	time_on = set_time_on;
	time_off = set_time_off;
	time_on2 = set_time_on2;
	time_off2 = set_time_off2;	
}

