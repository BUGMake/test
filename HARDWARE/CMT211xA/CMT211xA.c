#include "CMT211xA.h"
#include "encode_decode.h"


/*CMT211xA��TX*/
//u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
u16 CMT211xA_SymbolTime = 416;
u8 TxBuf[64];//Tx Buffer
u8 TxBufLength;//Tx Buffer active length


/*CMT211xA��TX*/
//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
// Return Value : None
// Parameters   : None
//-----------------------------------------------------------------------------
void CMT211xA_PORT_Init(void)//IO��ʼ��
{ 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 			 //IO-->PC.3 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //�����趨������ʼ��GPIOC.3
 //GPIO_SetBits(GPIOC,GPIO_Pin_3);						 						 //PC.3 �����
 GPIO_ResetBits(GPIOC,GPIO_Pin_3);											 //PC.3 �����
}

void SetTDAT(void)//����� 
{
	CMT211xA_TX = 1;
}
void ClrTDAT(void)//����� 
{
	CMT211xA_TX = 0;
}


/*ע�������vDelayUs()�ܹؼ���ֱ��Ӱ����뷢��ͽ��ս���ĳɹ���*/
/*ע�⣺�����Ȳ���vDelayUs(416);��ʵ����ʱ*/
//vDelayUs(1) = 1us
//vDelayUs(416) = 416us //�ؼ���Ϊ�����416us����ʱ
void CMT211xA_vDelayUs(u16 x)
{
	/*�����������ʱ��ʹ��vDelayUs(416) = 416us������vDelayUs(1) = 2us����vDelayUs(208) = 416us*/
	
//	x = x*3;/*1.13us*/
//	while(--x);
	
	delay_us(x);//����ԭ��ʹ��SysTick��ʱ����Ƶ�us��ʱ
}
void CMT211xA_vDelay416Us_test(void)
{
	/*����vDelayUs(416);��ʵ����ʱ*/
	/*ע�������vDelayUs(416)�ܹؼ���ֱ��Ӱ����뷢��ͽ��ս���ĳɹ���*/
	while(1)
	{
		CMT211xA_TX = !CMT211xA_TX;/*PC3ȡ�����*/
		CMT211xA_vDelayUs(416);//vDelayUs(416) = 416us	 vDelayUs(1) = 1us
		//vDelayUs(416) = 416us //�ؼ���Ϊ�����416us����ʱ
	}
}


static u8 mode = 0;//0=���߳��+����ͨ��һ�壻1=CMT211xA
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

	
	
	if(pwm_flag && mode==0)//���߳��+����ͨ��һ��
	{
		TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��
		delay_ms(20);//����20ms
	}
	//��ͨ��ʱĬ�������
	//�Զ���������/֡ͷ��5ms�ߡ�5ms�͡�5ms�ߡ���
	SetTDAT();//Data�ڸߵ�ƽ
	TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��
	delay_ms(5);//����5ms
	ClrTDAT();//Data�ڵ͵�ƽ
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��
	delay_ms(5);//����5ms
	SetTDAT();//Data�ڸߵ�ƽ
	TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��
	delay_ms(5);//����5ms
	ClrTDAT();//Data�ڵ͵�ƽ
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��
	delay_ms(5);//����5ms
	
	
	
	/*����TxBuf[64]������������ݣ��Ѿ�����radio.vEncode(str, 7, E527)�������룩*/
	for(i=0; i<TxBufLength; i++)
	{
		for(j=0x80; j!=0; j>>=1)//����һ���ֽ�=8λ
		{
			if(TxBuf[i]&j)
			{
				SetTDAT();//Data�ڸߵ�ƽ�����ܳ���3.6V
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر�	//S1-TZ-F-Code	���߳��+����ͨ��һ��
			}
			else
			{
				ClrTDAT();//Data�ڵ͵�ƽ
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��	//S1-TZ-F-Code	���߳��+����ͨ��һ��
			}


			/* arduino��ʱ��0��ʱSymbolTime=416us */
//			OCR0A  = bittime;
//			while((TIFR0&0x02)!=0x02);
//			TIFR0 = 0x02;		//Clear flag

			
			/*ע�������vDelayUs()�ܹؼ���ֱ��Ӱ����뷢��ĳɹ���*/
			if(mode==1)
				CMT211xA_vDelayUs(416);//416us	vDelayUs(1) = 1us  STM32F103RCT6 //S1-TZ-A-Code	 CMT211xA
			else if(mode==0)
				CMT211xA_vDelayUs(516);//416us	vDelayUs(1) = 1us  STM32F103RCT6 //S1-TZ-F-Code	 ���߳��+����ͨ��һ��
			/*�������ʱ���� = 416us��400us ~ 500us��= ���ն˽��ս���ɹ��ʽϵ͵�ʱ����Ե�������������*/
			//vDelayUs(416) = 416us //�ؼ���Ϊ�����416us����ʱ
		}
	}

//	OCR0A  = 0;
//	TCCR0A = 0x00;			
//	TCCR0B = 0x00;
	ClrTDAT();//Data�ڵ͵�ƽ 
}

/*����*/
void CMT211xA_Encode(u8 ptr[], u8 length, u8 etype)//CMT211xA_Encode(str, 7, 0);
{
	u8 i, j, k;	
	
	switch(etype)// 0 1 2
	{
		case CMT211xA_E527:
			
			if(mode==1)
				TxBuf[0] = 0x80; //S1-TZ-A-Code	 CMT211xA
			else if(mode==0)						
				TxBuf[0] = 0x00;//S1-TZ-F-Code  ���߳��+����ͨ��һ��
		
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





/*����������*/
void MOTOR_OUT_Init(void)//PC1
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 			 //IO-->PC.1 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //�����趨������ʼ��GPIOC.1
 GPIO_ResetBits(GPIOC,GPIO_Pin_1);						 					 //PC.1 �����
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
	MOTOR_OUT = !MOTOR_OUT;/*ȡ�����*/
}

/*���������*/
void MOTOR_IN_Init(void)//PC2
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTCʱ��

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;		//PC2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);				//��ʼ��GPIOC2
}
u8 MOTOR_IN_Check(void)
{		
	if(MOTOR_IN == 0)
	{
		delay_ms(10);//ȥ���� 
		if(MOTOR_IN == 0)
		{
			/* �ȴ������ͷ� */
			//while(MOTOR_IN == 0);   
			return 0;
		}
	  else   
			return 1;//�ް�������
	}
  else
  	return 1;//�ް�������
}
/*
ʹ�÷�����
if( MOTOR_IN_Check()==0 )//�а�������
{
	LED = !LED;
}
*/



/*280V��ѹ�������*/
void DC280V_Control_Init(void)//PC0
{
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 			 //IO-->PC.0 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 				 //�����趨������ʼ��GPIOC.0
 GPIO_SetBits(GPIOC,GPIO_Pin_0);						 					   //PC.0 �����
}
void DC280V_Control_ON (void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);//PC.0 �����
}
void DC280V_Control_OFF (void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_0);//PC.0 ����� 
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
	}//ϵͳdalay 5000ms����ִ�к�������
	prev_t = cut_t;
	/*sys ms delay*/
	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  
	
	//USART_printf = 1;/*ѡ�񴮿�1��Ϊprintf���*/
	printf("\r\nCMT211xA tx test!\r\n");  
			
	/*��������*/
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = 0x11;//����A����4λ������B����4λ
	//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]

	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//У�����8λ��ǰ
	str[CMT211xA_DATALength - 1] = by[1];

	//������3�Σ���Ӧ��
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//����ǰ����
	CMT211xA_TxPacket();//����������ݰ� = 70ms
	/*ע�⣺��������CMT211xA_vDelayUs_test();����vDelayUs(1000);��ʵ����ʱ*/
}


//���ƴ�ɫ

//��ɸ��ͨ��Э�� 0x55 0xab 0xxx crc0 crc1
//����0.�����߳��1.���ݴ��2.���߷�������    3.�򿪹رյ��    4.�����߳��
void control_work(u8 touzi_num)
{
  static u16 crc;
  static u8 by[2];
  static u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  
	//USART_printf = 1;/*ѡ�񴮿�1��Ϊprintf���*/
	printf("\n\rtest num = %2X\n\r",touzi_num);/*���2λ16������*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*�����߳��*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  				

	/*���͵�������*/	
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = touzi_num;//����A����4λ������B����4λ
	//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
	
	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);//
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//У�����8λ��ǰ
	str[CMT211xA_DATALength - 1] = by[1];

	//������3�Σ���Ӧ��
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//����ǰ����
	CMT211xA_TxPacket();//����������ݰ� �� 70ms
	delay_ms(500);
//			delay_ms(200);
//			CMT211xA_TxPacket();//����������ݰ� �� 70ms
//			delay_ms(200);
//			CMT211xA_TxPacket();//����������ݰ� �� 70ms
	/*ע�⣺��������CMT211xA_TxPacket();����vDelayUs(1000);��ʵ����ʱ*/
	
	//Send_a_com(touzi_num);//test

				
	/*��ת�����*/
	MOTOR_OUT_ON ();
	delay_ms(1000);/*����ת��*/	
//DC280V_Control_ON ();//�쳣����	
	delay_ms(1000);/*����ת��*/	
	//ע�⣺STM32��72M������,nms<=1864 
	
//	/*������Ȧ*/
//	DC280V_Control_ON ();
	delay_ms(100);/*����ת��*/	//20180202-
	/*ע�⣺��֤ = S1ת��̫�������£����Խ��յ�����������*/
	
	/*��ת�����*/
	MOTOR_OUT_OFF ();
	
	/*�ر�΢���������ʱ*/
	delay_ms(500);/*����ת��*/
	//delay_ms(100);//20180202+

	/*������Ȧ*/
//	DC280V_Control_ON ();//test��20180205-

	delay_ms(500);/*����ת��*/
	//delay_ms(800);/*����ת��*/
	//delay_ms(1000);/*����ת��*/
//delay_ms(1000);//�쳣����		
	DC280V_Control_OFF ();//�رմ���Ȧ����
	//ע�⣺Ҫ���Ժ�ת��ֹͣ����Ȧ�ų�ͨ�ϵ�ʱ���߼���ϵ -- �ܹؼ���
				
	/*�����߳��*/
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��
}
