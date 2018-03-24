#include "CMT211xA.h"
#include "encode_decode.h"


/*CMT211xA：TX*/
//u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
u16 CMT211xA_SymbolTime = 416;
u8 TxBuf[64];//Tx Buffer
u8 TxBufLength;//Tx Buffer active length


/*CMT211xA：TX*/
//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
// Return Value : None
// Parameters   : None
//-----------------------------------------------------------------------------
void CMT211xA_PORT_Init(void)//IO初始化
{ 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 			 //IO-->PC.3 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //根据设定参数初始化GPIOC.3
 //GPIO_SetBits(GPIOC,GPIO_Pin_3);						 						 //PC.3 输出高
 GPIO_ResetBits(GPIOC,GPIO_Pin_3);											 //PC.3 输出低
}

void SetTDAT(void)//输出高 
{
	CMT211xA_TX = 1;
}
void ClrTDAT(void)//输出低 
{
	CMT211xA_TX = 0;
}


/*注意这里的vDelayUs()很关键，直接影响编码发射和接收解码的成功率*/
/*注意：必须先测试vDelayUs(416);的实际延时*/
//vDelayUs(1) = 1us
//vDelayUs(416) = 416us //关键是为了这个416us的延时
void CMT211xA_vDelayUs(u16 x)
{
	/*调整这里的延时，使得vDelayUs(416) = 416us，比如vDelayUs(1) = 2us，则vDelayUs(208) = 416us*/
	
//	x = x*3;/*1.13us*/
//	while(--x);
	
	delay_us(x);//正点原子使用SysTick定时器设计的us延时
}
void CMT211xA_vDelay416Us_test(void)
{
	/*测试vDelayUs(416);的实际延时*/
	/*注意这里的vDelayUs(416)很关键，直接影响编码发射和接收解码的成功率*/
	while(1)
	{
		CMT211xA_TX = !CMT211xA_TX;/*PC3取反输出*/
		CMT211xA_vDelayUs(416);//vDelayUs(416) = 416us	 vDelayUs(1) = 1us
		//vDelayUs(416) = 416us //关键是为了这个416us的延时
	}
}


static u8 mode = 0;//0=无线充电+无线通信一体；1=CMT211xA
static u8 pwm_flag = 0;
void set_pwm_flag(u8 flag)
{
	pwm_flag = flag;
}

void CMT211xA_TxPacket(void)
{
	u8 i, j;
	u8 bittime;

//	Twi.vTWIInit();	
//	Twi.vTWIReset();				//step 1
//	vTwiOff();							//step 2
//
//	TIFR0  = 0x00;
//	TIMSK0 = 0x00;
//	OCR0A  = 0x00;
//	TCCR0A = 0x0E;		
//	TCCR0B = 0x00; 
//	TCNT0  = 0x00;

	if(CMT211xA_SymbolTime<16)
	{
		bittime = (CMT211xA_SymbolTime<<4);	//unit = 1/16 us	
		bittime -= 4;				//compensate
//		TCCR0B = 0x01;				//CLK/1 = 16MHz/1 = 16MHz Start	
	}				
	else if(CMT211xA_SymbolTime<128)
	{
		bittime = (CMT211xA_SymbolTime<<1);	//unit = 1/2 us
		bittime -= 1;				//compensate
//		TCCR0B = 0x02;				//CLK/8 = 16MHz/8 = 2MHz Start	
	}
	else if(CMT211xA_SymbolTime<1024)
	{
		bittime = (CMT211xA_SymbolTime>>2);	//unit = 4us
//		TCCR0B = 0x03;				//CLK/64 = 16MHz/64 
	}
	else if(CMT211xA_SymbolTime<4096)
	{
		bittime = (CMT211xA_SymbolTime>>4);	//unit = 16us
//		TCCR0B = 0x04;				//CLK/256 = 16MHz/256
	}
	else if(CMT211xA_SymbolTime<16384)
	{
		bittime = (CMT211xA_SymbolTime>>6);	//unit = 64us
//		TCCR0B  = 0x05;				//CLK/1024 = 16MHz/1024
	}
	else
	{
		bittime = 255;
//		TCCR0B  = 0x05;
	}

	
	
	if(pwm_flag && mode==0)//无线充电+无线通信一体
	{
		TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体
		delay_ms(20);//拉低20ms
	}
	//不通信时默认输出低
	//自定义引导码/帧头：5ms高、5ms低、5ms高、低
	SetTDAT();//Data口高电平
	TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体
	delay_ms(5);//拉高5ms
	ClrTDAT();//Data口低电平
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体
	delay_ms(5);//拉低5ms
	SetTDAT();//Data口高电平
	TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体
	delay_ms(5);//拉高5ms
	ClrTDAT();//Data口低电平
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体
	delay_ms(5);//拉低5ms
	
	
	
	/*发送TxBuf[64]数组里面的数据（已经经过radio.vEncode(str, 7, E527)函数编码）*/
	for(i=0; i<TxBufLength; i++)
	{
		for(j=0x80; j!=0; j>>=1)//发送一个字节=8位
		{
			if(TxBuf[i]&j)
			{
				SetTDAT();//Data口高电平：不能超过3.6V
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体
			}
			else
			{
				ClrTDAT();//Data口低电平
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体
			}


			/* arduino定时器0延时SymbolTime=416us */
//			OCR0A  = bittime;
//			while((TIFR0&0x02)!=0x02);
//			TIFR0 = 0x02;		//Clear flag

			
			/*注意这里的vDelayUs()很关键，直接影响编码发射的成功率*/
			if(mode==1)
				CMT211xA_vDelayUs(416);//416us	vDelayUs(1) = 1us  STM32F103RCT6 //S1-TZ-A-Code	 CMT211xA
			else if(mode==0)
				CMT211xA_vDelayUs(516);//416us	vDelayUs(1) = 1us  STM32F103RCT6 //S1-TZ-F-Code	 无线充电+无线通信一体
			/*这里的延时必须 = 416us（400us ~ 500us）= 接收端接收解码成功率较低的时候可以调整这里来调试*/
			//vDelayUs(416) = 416us //关键是为了这个416us的延时
		}
	}

//	OCR0A  = 0;
//	TCCR0A = 0x00;			
//	TCCR0B = 0x00;
	ClrTDAT();//Data口低电平 
}

/*编码*/
void CMT211xA_Encode(u8 ptr[], u8 length, u8 etype)//CMT211xA_Encode(str, 7, 0);
{
	u8 i, j, k;	
	
	switch(etype)// 0 1 2
	{
		case CMT211xA_E527:
			
			if(mode==1)
				TxBuf[0] = 0x80; //S1-TZ-A-Code	 CMT211xA
			else if(mode==0)						
				TxBuf[0] = 0x00;//S1-TZ-F-Code  无线充电+无线通信一体
		
			TxBuf[1] = 0x00;
			TxBuf[2] = 0x00;
			TxBuf[3] = 0x00;		
			for(i=4; i<(4+(length<<2)); i++)	//1XX0
				TxBuf[i] = 0x88;
			k = 4;
			for(j=0; j<length; j++)
			{
				for(i=0x80; i!=0; )
				{
					if((ptr[j]&i)!=0)
						TxBuf[k] |= 0x60;
					i >>= 1;
					if((ptr[j]&i)!=0)
						TxBuf[k] |= 0x06;
					i >>= 1;
					k++;
				}
			}
			TxBufLength = ((length<<2)+4);
		break;	
				
		case CMT211xA_E201:
			for(i=0; i<4; i++)			//
				TxBuf[i] = 0x00;		//4 byte 0x00
			TxBuf[4] = 0x0A;			//10 pulse preamble	
			for(i=5; i<(5+2); i++)
				TxBuf[i] = 0xAA;		
			TxBuf[i++] = 0x00;			//Sync
			TxBuf[i++] = 0x00;
			k = i+(length*3);
			j = i;		
			for( ; j<k ; )				
			{
				TxBuf[j++] = 0x92;		//1x01x01xB
				TxBuf[j++] = 0x49;		//01x01x01B
				TxBuf[j++] = 0x24;		//x01x01x0B
			}
			
			for(j=0; j<length; j++)
			{
				if((ptr[j]&0x80)==0x00)
					TxBuf[i] |= 0x40;
				if((ptr[j]&0x40)==0x00)
					TxBuf[i] |= 0x08;
				if((ptr[j]&0x20)==0x00)
					TxBuf[i] |= 0x01;
				i++;
				if((ptr[j]&0x10)==0x00)
					TxBuf[i] |= 0x20;
				if((ptr[j]&0x08)==0x00)
					TxBuf[i] |= 0x04;
				i++;
				if((ptr[j]&0x04)==0x00)
					TxBuf[i] |= 0x80;
				if((ptr[j]&0x02)==0x00)
					TxBuf[i] |= 0x10;
				if((ptr[j]&0x01)==0x00)
					TxBuf[i] |= 0x02;
				i++;
			}
			TxBuf[i++] = 0x92;
			TxBufLength = i;
		break;

		case CMT211xA_ENRZ: 
			
		default:
			for(i=0; i<length; i++)		//do nothing
				TxBuf[i] = ptr[i];
			TxBufLength = length;
		break;
	}
}





/*马达输出控制*/
void MOTOR_OUT_Init(void)//PC1
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 			 //IO-->PC.1 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //根据设定参数初始化GPIOC.1
 GPIO_ResetBits(GPIOC,GPIO_Pin_1);						 					 //PC.1 输出低
}
void MOTOR_OUT_ON(void)
{
	MOTOR_OUT = 1;
	//GPIO_SetBits(GPIOC,GPIO_Pin_1);	
}
void MOTOR_OUT_OFF(void)
{
	MOTOR_OUT = 0;
	//GPIO_ResetBits(GPIOC,GPIO_Pin_1);	
}
void MOTOR_OUT_Toggle(void)
{
	MOTOR_OUT = !MOTOR_OUT;/*取反输出*/
}

/*马达输入检测*/
void MOTOR_IN_Init(void)//PC2
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTC时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;		//PC2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);				//初始化GPIOC2
}
u8 MOTOR_IN_Check(void)
{		
	if(MOTOR_IN == 0)
	{
		delay_ms(10);//去抖动 
		if(MOTOR_IN == 0)
		{
			/* 等待按键释放 */
			//while(MOTOR_IN == 0);   
			return 0;
		}
	  else   
			return 1;//无按键按下
	}
  else
  	return 1;//无按键按下
}
/*
使用方法：
if( MOTOR_IN_Check()==0 )//有按键按下
{
	LED = !LED;
}
*/



/*280V高压输出控制*/
void DC280V_Control_Init(void)//PC0
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 			 //IO-->PC.0 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //根据设定参数初始化GPIOC.0
 GPIO_SetBits(GPIOC,GPIO_Pin_0);						 					   //PC.0 输出高
}
void DC280V_Control_ON (void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);//PC.0 输出低
}
void DC280V_Control_OFF (void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_0);//PC.0 输出高 
}

/*CMT211xA tx test*/
void CMT211xA_tx_test(void)
{
  u16 crc;
  u8 by[2];
	u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
	
	/*sys ms delay*/
	static unsigned long prev_t = 0;
	unsigned long cut_t = get_systick();
	if((cut_t - prev_t)<5000)
	{
		return;
	}//系统dalay 5000ms，才执行后面的语句
	prev_t = cut_t;
	/*sys ms delay*/
	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  
	
	//USART_printf = 1;/*选择串口1作为printf输出*/
	printf("\r\nCMT211xA tx test!\r\n");  
			
	/*点数命令*/
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = 0x11;//骰子A：低4位；骰子B：高4位
	//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]

	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//校验码低8位在前
	str[CMT211xA_DATALength - 1] = by[1];

	//连续发3次，无应答
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//发射前编码
	CMT211xA_TxPacket();//发射编码数据包 = 70ms
	/*注意：必须先在CMT211xA_vDelayUs_test();测试vDelayUs(1000);的实际延时*/
}


//控制打色

//和筛子通信协议 0x55 0xab 0xxx crc0 crc1
//步骤0.关无线充电1.数据打包2.无线发送数据    3.打开关闭电机    4.开无线充电
void control_work(u8 touzi_num)
{
  static u16 crc;
  static u8 by[2];
  static u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  
	//USART_printf = 1;/*选择串口1作为printf输出*/
	printf("\n\rtest num = %2X\n\r",touzi_num);/*输出2位16进制数*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*关无线充电*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  				

	/*发送点数命令*/	
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = touzi_num;//骰子A：低4位；骰子B：高4位
	//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
	
	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);//
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//校验码低8位在前
	str[CMT211xA_DATALength - 1] = by[1];

	//连续发3次，无应答
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//发射前编码
	CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
	delay_ms(500);
//			delay_ms(200);
//			CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
//			delay_ms(200);
//			CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
	/*注意：必须先在CMT211xA_TxPacket();测试vDelayUs(1000);的实际延时*/
	
	//Send_a_com(touzi_num);//test

				
	/*开转盘马达*/
	MOTOR_OUT_ON ();
	delay_ms(1000);/*常规转盘*/	
//DC280V_Control_ON ();//异常测试	
	delay_ms(1000);/*常规转盘*/	
	//注意：STM32对72M条件下,nms<=1864 
	
//	/*开大线圈*/
//	DC280V_Control_ON ();
	delay_ms(100);/*常规转盘*/	//20180202-
	/*注意：验证 = S1转动太快的情况下，可以接收到无线命令吗？*/
	
	/*关转盘马达*/
	MOTOR_OUT_OFF ();
	
	/*关闭微型马达后的延时*/
	delay_ms(500);/*常规转盘*/
	//delay_ms(100);//20180202+

	/*开大线圈*/
//	DC280V_Control_ON ();//test：20180205-

	delay_ms(500);/*常规转盘*/
	//delay_ms(800);/*常规转盘*/
	//delay_ms(1000);/*常规转盘*/
//delay_ms(1000);//异常测试		
	DC280V_Control_OFF ();//关闭大线圈控制
	//注意：要调试好转盘停止和线圈磁场通断的时序逻辑关系 -- 很关键！
				
	/*开无线充电*/
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能
}
