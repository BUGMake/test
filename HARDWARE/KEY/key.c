#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//�������� ��������		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/06
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									   
//////////////////////////////////////////////////////////////////////////////////	 
 	    


/*6��������ϴ�ƽ����źŽ���*/

#define KEY_VALUE_GPIO     ((GPIOB->IDR&GPIO_Pin_12))       
#define CARD_VALUE_GPIO    ((GPIOB->IDR&GPIO_Pin_13))

PWM_STRUCT stPwm;

//�����жϳ�ʼ������
//STCP     PB0
//clk      PC4
//key_in   PB12
//DS_OUT   PB13
void keyValueInit(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	//����
	GPIO_InitTypeDef GPIO_InitStructure;
 

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;////PB0 STCP
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB0
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;////PC4 clk
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC4

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;////PB12 key_in
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB12
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;////PB13 DS_OUT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB13
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

	//GPIOB.0 �ж����Լ��жϳ�ʼ������   �½��ش���
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);

	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

	//GPIOC.4	  �ж����Լ��жϳ�ʼ������  �½��ش���	//KEY0
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource4);
	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); 

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//ʹ�ܰ���KEY2���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);
	
}
//PB0 ��λ����
void EXTI0_IRQHandler(void) //
{
	if(EXTI->PR & EXTI_Line0)
	{
	if((GPIOB->IDR&GPIO_Pin_0)==0)//�½��ش��� 
	{
		  
		stPwm.keyStep=0;
		stPwm.keyFlag=KEY_FALSE;

		if(stPwm.cardStep==0&&stPwm.cardFlag==KEY_FALSE)
		{
			stPwm.cardFlag=KEY_TRUE;
		}	
		else
		{
				stPwm.cardFlag=KEY_FALSE;
		   stPwm.cardStep=0;
		   stPwm.cardWashStep=0;
		}
	}
	//else
	
	else if((GPIOB->IDR&GPIO_Pin_0)) //�������ж�
	{
			
		stPwm.cardFlag=KEY_FALSE;
		stPwm.cardStep=0;
		stPwm.cardWashStep=0;
		//��������
		if(stPwm.keyStep==0)//����
		{
			stPwm.keyFlag=KEY_TRUE;					
		}
		else
		{
		  stPwm.keyStep=0;
  	  stPwm.keyFlag=KEY_FALSE;
		}
	}
}
		//����ж�
	EXTI->PR = EXTI_Line0;
	//
}
//clk    PC4
void EXTI4_IRQHandler(void)//
{	  
   if(EXTI->PR & EXTI_Line4)
	//if()//�����ش���
	{   
		if((GPIOC->IDR&GPIO_Pin_4))//�����ش���
		{
	    //����			
		if(stPwm.cardStep<16&&stPwm.cardFlag==KEY_TRUE&&stPwm.keyFlag==KEY_FALSE)
		{			
			stPwm.cardStep++;
	        //��2��������ϴ�� �ߵ�ƽ���� //��12��������ϴ�ƿ����ź� �ߵ�ƽ��Ч //��9�������ǵ� �͵�ƽ����  
	    if(stPwm.cardStep==2&&stPwm.cardWashStep==0&&CARD_VALUE_GPIO==0/*�͵�ƽ*/)//����
			{
				stPwm.cardWashStep=1;
			}
			else if(stPwm.cardStep==12&&stPwm.cardWashStep==1&&CARD_VALUE_GPIO==0/*�͵�ƽ*/)
			{ 
				stPwm.cardWashTime++;
				if(stPwm.cardWashTime>3)
				{
					stPwm.cardWashTime=0;
			    stPwm.cardWashFlag=KEY_TRUE;
				}
			} 			
		}
		else{
			stPwm.cardStep=0;
			stPwm.cardWashStep=0;
			stPwm.cardFlag=KEY_FALSE;			
		}
		//����
		if(stPwm.keyStep<7&&stPwm.cardFlag==KEY_FALSE&&stPwm.keyFlag==KEY_TRUE)
		{						
	        if(stPwm.keyStep>=1&&KEY_VALUE_GPIO==0)//�ߵ�ƽ����
					{
						stPwm.keyValue[6-stPwm.keyStep]=1;	
					}	
          stPwm.keyStep++;						     		         			    		
		}
		else{
			stPwm.keyStep=0;
			stPwm.keyFlag=KEY_FALSE;
		}
  	}
	}
	EXTI->PR = EXTI_Line4;	
}




 
















			
			
			
//������ʼ������ 
//PA15���ó�����
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTA,PORTCʱ��

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;//PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA15
	
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC5
// 
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PA0
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA0	
} 
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//����ֵ��
//0��û���κΰ�������
//KEY0_PRES��KEY0����
//KEY1_PRES��KEY1����
//WKUP_PRES��WK_UP���� 
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||WK_UP==1))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
		else if(KEY1==0)return KEY1_PRES;
		else if(WK_UP==1)return WKUP_PRES; 
	}else if(KEY0==1&&KEY1==1&&WK_UP==0)key_up=1; 	     
	return 0;// �ް�������
}

//xi 1
//bei 2
//dong 3
//nan 4
u8 KEY_Read(void)//��ȡ������������
{
	u8 t=0;
	
	for (t = 0; t < 6; t++)
	{
		if(stPwm.keyValue[t]) 
		{  
			
			stPwm.keyValue[t]=0;//���
			return t+1;
			//
		}
	}
	
	return 0;
}
