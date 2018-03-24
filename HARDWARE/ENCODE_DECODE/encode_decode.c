#include "encode_decode.h"
//#include "timer.h"
#include "CMT211xA.h" //sbit DATAOUT=P1^0;//输出
//#include "CMT221xA.h" //sbit DATAIN=P2^1; //输入
//#include "uart.h"	  //U16 GetCrc(const U8 *pSendBuf, U8 nEnd)
#include "delay.h"

#define DATAOUT CMT211xA_TX // PCout(3)	// PC3
//u8 DATAOUT;//sbit DATAOUT=P1^0;//输出
u8 DATAIN;//sbit DATAIN=P2^1; //输入

u8 SysTimer;
u8 ComTimer;

//timer0延时0.1ms
void ComDelayMs(u8 ms)
{
	ComTimer = ms;
	while(ComTimer)
	{
		ComTimer--;
		delay_us(100);//0.1ms
	}
}

//引导码/帧头
void guidance_code(void)
{
	//不通信时默认输出高
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(firstLowTime);//拉低5ms

	/*引导码/帧头*/
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(beginTime);//拉高9ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(beginTime);//拉低9ms
}

//0码
void Bit0(void)
{
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(highTime);//拉高1ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(lowTimebit0);//拉低2ms
}

//1码
void Bit1(void)
{
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(highTime);//拉高1ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(lowTimebit1);//拉低1ms
}

//结束码/帧尾
void epilog_code(void)
{
	/*结束码/帧尾*/
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(endTime);//拉高5ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能	//S1-TZ-F-Code	无线充电+无线通信一体	
	ComDelayMs(endTime);//拉低5ms

	//不通信时默认输出高
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭	//S1-TZ-F-Code	无线充电+无线通信一体	
}

//发送一个字节
void Send_a_byte(u8 byte)
{
	u8 i;
	for(i=8;i>0;i--)
	{
		//低位在前
		if(byte&0x01)
			Bit1();
		else
			Bit0();
		byte = byte>>1;
	}
}

//发送一个命令：0x11-0x66
//通信协议：0x55 0xab [命令] [低8位校验码] [高8位校验码] //5字节
//大概需要40ms（重新定义帧头、帧尾、1码和0码的编码长度即可优化时间长度）
void Send_a_com(u8 com)
{
	u16 crc;
	u8 by[2];
	u8 tx_buff[comLength];

	tx_buff[0] = 0x55;
	tx_buff[1] = 0xab;
	tx_buff[2] = com;

	//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
	crc = GetCrc((const u8 *)tx_buff, comLength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	tx_buff[comLength - 2] = by[0];//校验码低8位在前
	tx_buff[comLength - 1] = by[1];

	guidance_code();//引导码/帧头

	Send_a_byte(tx_buff[0]);
	Send_a_byte(tx_buff[1]);
	Send_a_byte(tx_buff[2]);
	Send_a_byte(tx_buff[3]);
	Send_a_byte(tx_buff[4]);

	epilog_code();	//结束码/帧尾
}


//接收解码：注意通信的可靠性和抗干扰性
u8 Receive_a_com(u8 rx_buff[],u8 rx_len)//返回1表示接收到5个字节的数据
{
	u8 byte = 0;
	u8 i,j;

	//检测到低电平
	if(DATAIN == 0)
	{
		SysTimer = 0;
		while(DATAIN == 0)//等待低电平结束
		{
			if(SysTimer > firstLowTime*2)return 0;//低电平大于10ms，超时，不要	
		}
		//高电平开始
		if(SysTimer>=firstLowTime-10 && SysTimer<=firstLowTime+10)//5ms的低电平
		{
			/*检测引导码/帧头*/
			SysTimer = 0;
			while(DATAIN == 1)//等待高电平结束
			{
				if(SysTimer > beginTime*2)return 0;//高电平大于18ms，超时，不要	
			}
			//低电平开始
			if(SysTimer>=beginTime-10 && SysTimer<=beginTime+10)//9ms的高电平
			{			
				SysTimer = 0;
				while(DATAIN == 0)//等待低电平结束
				{
					if(SysTimer > beginTime*2)return 0;//低电平大于18ms，超时，不要	
				}
				//高电平开始
				if(SysTimer>=beginTime-10 && SysTimer<=beginTime+10)//9ms的低电平
				{
					/*引导码/帧头ok，开始解码数据*/
					for(j=0;j<rx_len;j++)//解码5个字节
					{
						for(i=0;i<8;i++)//解码1个字节
						{
							SysTimer = 0;
							while(DATAIN == 1)//等待高电平结束
							{
								if(SysTimer > highTime*5)return 0;//高电平大于5ms，超时，不要	
							}
							//低电平开始
							if(SysTimer>=1 && SysTimer<=highTime+10)//1ms的高电平
							{
								SysTimer = 0;
								while(DATAIN == 0)//等待低电平结束
								{
									if(SysTimer > lowTimebit0*2)return 0;//低电平大于4ms，超时，不要	
								}
								/*低位在前*/
								if(SysTimer>=lowTimebit1-5 && SysTimer<=lowTimebit1+5)//1ms的低电平：1码
								{
									byte = byte|0x80;/*低位在前*/
								}
								if(SysTimer>lowTimebit0-5 && SysTimer<=lowTimebit0+5)//2ms的低电平：0码
								{
								
								}
								if(i == 7)break;
								byte = byte>>1;/*低位在前，右移7位*/
							}
						}
						rx_buff[j] = byte;
						byte = 0;//清零
					}
					//高电平开始
					/*检测结束码/帧尾*/
					SysTimer = 0;
					while(DATAIN == 1)//等待高电平结束
					{
						if(SysTimer > endTime*2)return 0;//高电平大于10ms，超时，不要	
					}
					if(SysTimer>=endTime-10 && SysTimer<=endTime+10)//5ms的高电平
						return 1;
					else return 0;
				}
			}
		}
	}
	return 0;
}

