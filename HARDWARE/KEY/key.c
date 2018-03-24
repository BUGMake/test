#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//按键输入 驱动代码		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/06
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									   
//////////////////////////////////////////////////////////////////////////////////	 
 	    


/*6个按键和洗牌结束信号解码*/

#define KEY_VALUE_GPIO     ((GPIOB->IDR&GPIO_Pin_12))       
#define CARD_VALUE_GPIO    ((GPIOB->IDR&GPIO_Pin_13))

PWM_STRUCT stPwm;

//配置中断初始化函数
//STCP     PB0
//clk      PC4
//key_in   PB12
//DS_OUT   PB13
void keyValueInit(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	//输入
	GPIO_InitTypeDef GPIO_InitStructure;
 

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;////PB0 STCP
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;////PC4 clk
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC4

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;////PB12 key_in
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB12
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;////PB13 DS_OUT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB13
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

	//GPIOB.0 中断线以及中断初始化配置   下降沿触发
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);

	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	//GPIOC.4	  中断线以及中断初始化配置  下降沿触发	//KEY0
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource4);
	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键WK_UP所在的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); 

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//使能按键KEY2所在的外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);
	
}
//PB0 移位锁存
void EXTI0_IRQHandler(void) //
{
	if(EXTI->PR & EXTI_Line0)
	{
	if((GPIOB->IDR&GPIO_Pin_0)==0)//下降沿触发 
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
	
	else if((GPIOB->IDR&GPIO_Pin_0)) //上升沿中断
	{
			
		stPwm.cardFlag=KEY_FALSE;
		stPwm.cardStep=0;
		stPwm.cardWashStep=0;
		//按键处理
		if(stPwm.keyStep==0)//按键
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
		//清除中断
	EXTI->PR = EXTI_Line0;
	//
}
//clk    PC4
void EXTI4_IRQHandler(void)//
{	  
   if(EXTI->PR & EXTI_Line4)
	//if()//上升沿触发
	{   
		if((GPIOC->IDR&GPIO_Pin_4))//上升沿触发
		{
	    //运行			
		if(stPwm.cardStep<16&&stPwm.cardFlag==KEY_TRUE&&stPwm.keyFlag==KEY_FALSE)
		{			
			stPwm.cardStep++;
	        //第2个脉冲是洗牌 高电平点亮 //第12个脉冲是洗牌控制信号 高电平有效 //第9个脉冲是灯 低电平点亮  
	    if(stPwm.cardStep==2&&stPwm.cardWashStep==0&&CARD_VALUE_GPIO==0/*低电平*/)//保存
			{
				stPwm.cardWashStep=1;
			}
			else if(stPwm.cardStep==12&&stPwm.cardWashStep==1&&CARD_VALUE_GPIO==0/*低电平*/)
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
		//按键
		if(stPwm.keyStep<7&&stPwm.cardFlag==KEY_FALSE&&stPwm.keyFlag==KEY_TRUE)
		{						
	        if(stPwm.keyStep>=1&&KEY_VALUE_GPIO==0)//高电平按下
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




 
















			
			
			
//按键初始化函数 
//PA15设置成输入
void KEY_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTC时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;//PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA15
	
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC5
// 
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PA0
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
//	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA0	
} 
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//WKUP_PRES，WK_UP按下 
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0||KEY1==0||WK_UP==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
		else if(KEY1==0)return KEY1_PRES;
		else if(WK_UP==1)return WKUP_PRES; 
	}else if(KEY0==1&&KEY1==1&&WK_UP==0)key_up=1; 	     
	return 0;// 无按键按下
}

//xi 1
//bei 2
//dong 3
//nan 4
u8 KEY_Read(void)//获取东南西北按键
{
	u8 t=0;
	
	for (t = 0; t < 6; t++)
	{
		if(stPwm.keyValue[t]) 
		{  
			
			stPwm.keyValue[t]=0;//清除
			return t+1;
			//
		}
	}
	
	return 0;
}
