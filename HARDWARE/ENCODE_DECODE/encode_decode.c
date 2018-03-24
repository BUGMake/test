#include "encode_decode.h"
//#include "timer.h"
#include "CMT211xA.h" //sbit DATAOUT=P1^0;//���
//#include "CMT221xA.h" //sbit DATAIN=P2^1; //����
//#include "uart.h"	  //U16 GetCrc(const U8 *pSendBuf, U8 nEnd)
#include "delay.h"

#define DATAOUT CMT211xA_TX // PCout(3)	// PC3
//u8 DATAOUT;//sbit DATAOUT=P1^0;//���
u8 DATAIN;//sbit DATAIN=P2^1; //����

u8 SysTimer;
u8 ComTimer;

//timer0��ʱ0.1ms
void ComDelayMs(u8 ms)
{
	ComTimer = ms;
	while(ComTimer)
	{
		ComTimer--;
		delay_us(100);//0.1ms
	}
}

//������/֡ͷ
void guidance_code(void)
{
	//��ͨ��ʱĬ�������
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(firstLowTime);//����5ms

	/*������/֡ͷ*/
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(beginTime);//����9ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(beginTime);//����9ms
}

//0��
void Bit0(void)
{
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(highTime);//����1ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(lowTimebit0);//����2ms
}

//1��
void Bit1(void)
{
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(highTime);//����1ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(lowTimebit1);//����1ms
}

//������/֡β
void epilog_code(void)
{
	/*������/֡β*/
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(endTime);//����5ms
	DATAOUT = 0;
TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
	ComDelayMs(endTime);//����5ms

	//��ͨ��ʱĬ�������
	DATAOUT = 1;
TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��	
}

//����һ���ֽ�
void Send_a_byte(u8 byte)
{
	u8 i;
	for(i=8;i>0;i--)
	{
		//��λ��ǰ
		if(byte&0x01)
			Bit1();
		else
			Bit0();
		byte = byte>>1;
	}
}

//����һ�����0x11-0x66
//ͨ��Э�飺0x55 0xab [����] [��8λУ����] [��8λУ����] //5�ֽ�
//�����Ҫ40ms�����¶���֡ͷ��֡β��1���0��ı��볤�ȼ����Ż�ʱ�䳤�ȣ�
void Send_a_com(u8 com)
{
	u16 crc;
	u8 by[2];
	u8 tx_buff[comLength];

	tx_buff[0] = 0x55;
	tx_buff[1] = 0xab;
	tx_buff[2] = com;

	//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
	crc = GetCrc((const u8 *)tx_buff, comLength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	tx_buff[comLength - 2] = by[0];//У�����8λ��ǰ
	tx_buff[comLength - 1] = by[1];

	guidance_code();//������/֡ͷ

	Send_a_byte(tx_buff[0]);
	Send_a_byte(tx_buff[1]);
	Send_a_byte(tx_buff[2]);
	Send_a_byte(tx_buff[3]);
	Send_a_byte(tx_buff[4]);

	epilog_code();	//������/֡β
}


//���ս��룺ע��ͨ�ŵĿɿ��ԺͿ�������
u8 Receive_a_com(u8 rx_buff[],u8 rx_len)//����1��ʾ���յ�5���ֽڵ�����
{
	u8 byte = 0;
	u8 i,j;

	//��⵽�͵�ƽ
	if(DATAIN == 0)
	{
		SysTimer = 0;
		while(DATAIN == 0)//�ȴ��͵�ƽ����
		{
			if(SysTimer > firstLowTime*2)return 0;//�͵�ƽ����10ms����ʱ����Ҫ	
		}
		//�ߵ�ƽ��ʼ
		if(SysTimer>=firstLowTime-10 && SysTimer<=firstLowTime+10)//5ms�ĵ͵�ƽ
		{
			/*���������/֡ͷ*/
			SysTimer = 0;
			while(DATAIN == 1)//�ȴ��ߵ�ƽ����
			{
				if(SysTimer > beginTime*2)return 0;//�ߵ�ƽ����18ms����ʱ����Ҫ	
			}
			//�͵�ƽ��ʼ
			if(SysTimer>=beginTime-10 && SysTimer<=beginTime+10)//9ms�ĸߵ�ƽ
			{			
				SysTimer = 0;
				while(DATAIN == 0)//�ȴ��͵�ƽ����
				{
					if(SysTimer > beginTime*2)return 0;//�͵�ƽ����18ms����ʱ����Ҫ	
				}
				//�ߵ�ƽ��ʼ
				if(SysTimer>=beginTime-10 && SysTimer<=beginTime+10)//9ms�ĵ͵�ƽ
				{
					/*������/֡ͷok����ʼ��������*/
					for(j=0;j<rx_len;j++)//����5���ֽ�
					{
						for(i=0;i<8;i++)//����1���ֽ�
						{
							SysTimer = 0;
							while(DATAIN == 1)//�ȴ��ߵ�ƽ����
							{
								if(SysTimer > highTime*5)return 0;//�ߵ�ƽ����5ms����ʱ����Ҫ	
							}
							//�͵�ƽ��ʼ
							if(SysTimer>=1 && SysTimer<=highTime+10)//1ms�ĸߵ�ƽ
							{
								SysTimer = 0;
								while(DATAIN == 0)//�ȴ��͵�ƽ����
								{
									if(SysTimer > lowTimebit0*2)return 0;//�͵�ƽ����4ms����ʱ����Ҫ	
								}
								/*��λ��ǰ*/
								if(SysTimer>=lowTimebit1-5 && SysTimer<=lowTimebit1+5)//1ms�ĵ͵�ƽ��1��
								{
									byte = byte|0x80;/*��λ��ǰ*/
								}
								if(SysTimer>lowTimebit0-5 && SysTimer<=lowTimebit0+5)//2ms�ĵ͵�ƽ��0��
								{
								
								}
								if(i == 7)break;
								byte = byte>>1;/*��λ��ǰ������7λ*/
							}
						}
						rx_buff[j] = byte;
						byte = 0;//����
					}
					//�ߵ�ƽ��ʼ
					/*��������/֡β*/
					SysTimer = 0;
					while(DATAIN == 1)//�ȴ��ߵ�ƽ����
					{
						if(SysTimer > endTime*2)return 0;//�ߵ�ƽ����10ms����ʱ����Ҫ	
					}
					if(SysTimer>=endTime-10 && SysTimer<=endTime+10)//5ms�ĸߵ�ƽ
						return 1;
					else return 0;
				}
			}
		}
	}
	return 0;
}

