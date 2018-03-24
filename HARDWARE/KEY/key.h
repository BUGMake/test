#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK miniSTM32������
//������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 


//#define KEY0 PCin(5)   	
//#define KEY1 PAin(15)	 
//#define WK_UP  PAin(0)	 
 

#define KEY0  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)//��ȡ����0
#define KEY1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)//��ȡ����1
#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//��ȡ����2 
 

#define KEY0_PRES	1		//KEY0  
#define KEY1_PRES	2		//KEY1 
#define WKUP_PRES	3		//WK_UP  


typedef enum {
KEY_FALSE=0,
KEY_TRUE=1,
}PWM_ENUM;

typedef struct{
unsigned char keyStep;//��������
unsigned char cardStep;//ϴ�Ʋ���
PWM_ENUM keyFlag;//
PWM_ENUM cardFlag;
unsigned char keyValue[8];//������ֵ
unsigned char cardWashStep;
PWM_ENUM cardWashFlag;//
unsigned char cardWashTime;	
//PWM_ENUM ledFlag;//
//unsigned char ledTime;
//unsigned char cardValue[16];//ϴ�Ʋ���ֵ
}PWM_STRUCT;


void KEY_Init(void);//IO��ʼ��
u8 KEY_Scan(u8 mode);  	//����ɨ�躯��




extern PWM_STRUCT stPwm;

void keyValueInit(void);

u8 KEY_Read(void);//��ȡ������������


#endif
