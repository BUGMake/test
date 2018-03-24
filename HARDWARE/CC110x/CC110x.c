#include "CC110x.h"
#include "sys.h" 
#include "usart.h"	  
#include "ring_buffer.h"
#include "delay.h"


/////////////////////////////////////////////////////////////////
const RF_SETTINGS rfSettings = 
{
		0x00,
    0x08,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x10,   // FREQ2     Frequency control word, high byte.
    0xA7,   // FREQ1     Frequency control word, middle byte.
    0x62,   // FREQ0     Frequency control word, low byte.
    0x5B,   // MDMCFG4   Modem configuration.
    0xF8,   // MDMCFG3   Modem configuration.
    0x03,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.

    0x00,   // CHANNR    Channel number.
    0x47,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0xB6,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end RX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x1D,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x1C,   // BSCFG     Bit synchronization Configuration.
    0xC7,   // AGCCTRL2  AGC control.
    0x00,   // AGCCTRL1  AGC control.
    0xB2,   // AGCCTRL0  AGC control.

    0xEA,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x11,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x81,   // TEST2     Various test settings.
    0x35,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.
    0x0B,   // IOCFG2    GDO2 output pin configuration.
    0x06,   // IOCFG0D   GDO0 output pin configuration. Refer to SmartRF?Studio User Manual for detailed pseudo register explanation.

    0x04,   // PKTCTRL1  Packet automation control.
    0x05,   // PKTCTRL0  Packet automation control.
    0x00,   // ADDR      Device address.
    0x0c    // PKTLEN    Packet length.
};


//#define _nop_(); {asm("nop");}
#define _nop_(); {__nop();}

void halWait(u16 timeout) 
{
	do 
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_(); 
	} while(--timeout);
}

/*PA4=NSS;PA5=SCK;PA6=MISO;PA7=MOSI;PC5=IRQ=GDO0*/
void SpiInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);	 

	/*�����NSS=0;SCK=1;MOSI=1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;				 			 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 				
	GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);	
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);		

	/*����������MISO;GDO0*/  
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);


  CSN_L;//CSN=0;
	
  SCK_L;//SCK=0;
  
  CSN_H;//CSN=1;  
}

//*****************************************************************************************
//��������SpisendByte(INT8U dat)
//���룺���͵�����
//�������
//����������SPI����һ���ֽ�
//*****************************************************************************************
u8 SpiTxRxByte(u8 dat)
{
	u8 i,temp;
	temp = 0;
	
	SCK_L;//SCK = 0;
	for(i=0; i<8; i++)
	{
		if(dat & 0x80)
		{			
			MOSI_H;//MOSI = 1;
		}
		else 	  
		  MOSI_L;//MOSI = 0;
		dat <<= 1;
	
		SCK_H;//SCK = 1; 
		_nop_();
		_nop_();

		temp <<= 1;
		if(MISO == 1)temp++; 	
		SCK_L;//SCK = 0;
		_nop_();
		_nop_();	
	}
	return temp;
}

//*****************************************************************************************
//��������void RESET_CC1100(void)
//���룺��
//�������
//������������λCC1100
//*****************************************************************************************
void RESET_CC1100(void) 
{
	CSN_L;//CSN = 0; 
	
	while (MISO == 1); 
	SpiTxRxByte(CCxxx0_SRES); 		//д�븴λ����
	while (MISO == 1); 
	
	CSN_H;//CSN = 1; 
}

//*****************************************************************************************
//��������void POWER_UP_RESET_CC1100(void) 
//���룺��
//�������
//�����������ϵ縴λCC1100
//*****************************************************************************************
void POWER_UP_RESET_CC1100(void) 
{
  CSN_H;//CSN = 1; 
	halWait(1); 
	CSN_L;//CSN = 0; 
	halWait(1); 
	CSN_H;//CSN = 1; 
	halWait(41); 
	RESET_CC1100();   		//��λCC1100
}

//*****************************************************************************************
//��������void halSpiWriteReg(INT8U addr, INT8U value)
//���룺��ַ��������
//�������
//����������SPIд�Ĵ���
//*****************************************************************************************
void halSpiWriteReg(u8 addr, u8 value) 
{	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(addr);		//д��ַ
	SpiTxRxByte(value);		//д������
	
	CSN_H;//CSN = 1;
}

//*****************************************************************************************
//��������void halSpiWriteBurstReg(INT8U addr, INT8U *buffer, INT8U count)
//���룺��ַ��д�뻺������д�����
//�������
//����������SPI����д���üĴ���
//*****************************************************************************************
void halSpiWriteBurstReg(u8 addr, u8 *buffer, u8 count) 
{
	u8 i, temp;
	temp = addr | WRITE_BURST;
		
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);
	for (i = 0; i < count; i++)
	{
		SpiTxRxByte(buffer[i]);
	}
	
	CSN_H;//CSN = 1;
}

//*****************************************************************************************
//��������void halSpiStrobe(INT8U strobe)
//���룺����
//�������
//����������SPIд����
//*****************************************************************************************
void halSpiStrobe(u8 strobe) 
{
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(strobe);		//д������
	
	CSN_H;//CSN = 1;
}





//*****************************************************************************************
//��������INT8U halSpiReadReg(INT8U addr)
//���룺��ַ
//������üĴ�����������
//����������SPI���Ĵ���
//*****************************************************************************************
u8 halSpiReadReg(u8 addr) 
{
	u8 temp, value;
	temp = addr|READ_SINGLE;//���Ĵ�������
	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);
	value = SpiTxRxByte(0);
	
	CSN_H;//CSN = 1;
	
	return value;
}


//*****************************************************************************************
//��������void halSpiReadBurstReg(INT8U addr, INT8U *buffer, INT8U count)
//���룺��ַ���������ݺ��ݴ�Ļ��������������ø���
//�������
//����������SPI����д���üĴ���
//*****************************************************************************************
void halSpiReadBurstReg(u8 addr, u8 *buffer, u8 count) 
{
	u8 i,temp;
	temp = addr | READ_BURST;		//д��Ҫ�������üĴ�����ַ�Ͷ�����
	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);   
	for (i = 0; i < count; i++) 
	{
		buffer[i] = SpiTxRxByte(0);
	}
	
	CSN_H;//CSN = 1;
}


//*****************************************************************************************
//��������INT8U halSpiReadReg(INT8U addr)
//���룺��ַ
//�������״̬�Ĵ�����ǰֵ
//����������SPI��״̬�Ĵ���
//*****************************************************************************************
u8 halSpiReadStatus(u8 addr) 
{
	u8 value,temp;
	temp = addr | READ_BURST;		//д��Ҫ����״̬�Ĵ����ĵ�ַͬʱд�������
	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);
	value = SpiTxRxByte(0);
	
	CSN_H;//CSN = 1;
	
	return value;
}
//*****************************************************************************************
//��������void halRfWriteRfSettings(RF_SETTINGS *pRfSettings)
//���룺��
//�������
//��������������CC1100�ļĴ���
//*****************************************************************************************
void halRfWriteRfSettings(void) 
{
	halSpiWriteReg(CCxxx0_FSCTRL0,  rfSettings.FSCTRL2);//���Ѽӵ�
	// Write register settings
	halSpiWriteReg(CCxxx0_FSCTRL1,  rfSettings.FSCTRL1);
	halSpiWriteReg(CCxxx0_FSCTRL0,  rfSettings.FSCTRL0);
	halSpiWriteReg(CCxxx0_FREQ2,    rfSettings.FREQ2);
	halSpiWriteReg(CCxxx0_FREQ1,    rfSettings.FREQ1);
	halSpiWriteReg(CCxxx0_FREQ0,    rfSettings.FREQ0);
	halSpiWriteReg(CCxxx0_MDMCFG4,  rfSettings.MDMCFG4);
	halSpiWriteReg(CCxxx0_MDMCFG3,  rfSettings.MDMCFG3);
	halSpiWriteReg(CCxxx0_MDMCFG2,  rfSettings.MDMCFG2);
	halSpiWriteReg(CCxxx0_MDMCFG1,  rfSettings.MDMCFG1);
	halSpiWriteReg(CCxxx0_MDMCFG0,  rfSettings.MDMCFG0);
	halSpiWriteReg(CCxxx0_CHANNR,   rfSettings.CHANNR);
	halSpiWriteReg(CCxxx0_DEVIATN,  rfSettings.DEVIATN);
	halSpiWriteReg(CCxxx0_FREND1,   rfSettings.FREND1);
	halSpiWriteReg(CCxxx0_FREND0,   rfSettings.FREND0);
	halSpiWriteReg(CCxxx0_MCSM0 ,   rfSettings.MCSM0 );
	halSpiWriteReg(CCxxx0_FOCCFG,   rfSettings.FOCCFG);
	halSpiWriteReg(CCxxx0_BSCFG,    rfSettings.BSCFG);
	halSpiWriteReg(CCxxx0_AGCCTRL2, rfSettings.AGCCTRL2);
	halSpiWriteReg(CCxxx0_AGCCTRL1, rfSettings.AGCCTRL1);
	halSpiWriteReg(CCxxx0_AGCCTRL0, rfSettings.AGCCTRL0);
	halSpiWriteReg(CCxxx0_FSCAL3,   rfSettings.FSCAL3);
	halSpiWriteReg(CCxxx0_FSCAL2,   rfSettings.FSCAL2);
	halSpiWriteReg(CCxxx0_FSCAL1,   rfSettings.FSCAL1);
	halSpiWriteReg(CCxxx0_FSCAL0,   rfSettings.FSCAL0);
	halSpiWriteReg(CCxxx0_FSTEST,   rfSettings.FSTEST);
	halSpiWriteReg(CCxxx0_TEST2,    rfSettings.TEST2);
	halSpiWriteReg(CCxxx0_TEST1,    rfSettings.TEST1);
	halSpiWriteReg(CCxxx0_TEST0,    rfSettings.TEST0);
	halSpiWriteReg(CCxxx0_IOCFG2,   rfSettings.IOCFG2);
	halSpiWriteReg(CCxxx0_IOCFG0,   rfSettings.IOCFG0);    
	halSpiWriteReg(CCxxx0_PKTCTRL1, rfSettings.PKTCTRL1);
	halSpiWriteReg(CCxxx0_PKTCTRL0, rfSettings.PKTCTRL0);
	halSpiWriteReg(CCxxx0_ADDR,     rfSettings.ADDR);
	halSpiWriteReg(CCxxx0_PKTLEN,   rfSettings.PKTLEN);
}

//*****************************************************************************************
//��������void halRfSendPacket(INT8U *txBuffer, INT8U size)
//���룺���͵Ļ��������������ݸ���
//�������
//����������CC1100����һ������
//*****************************************************************************************

void halRfSendPacket(u8 *txBuffer, u8 size) 
{
	halSpiWriteReg(CCxxx0_TXFIFO, size);
	halSpiWriteBurstReg(CCxxx0_TXFIFO, txBuffer, size);	//д��Ҫ���͵�����

	halSpiStrobe(CCxxx0_STX);		//���뷢��ģʽ�������ݣ�����Ϊ����ģʽ��

	// Wait for GDO0 to be set -> sync transmitted
	while (GDO0 == 0);
	// Wait for GDO0 to be cleared -> end of packet
	while (GDO0 == 1);
	halSpiStrobe(CCxxx0_SFTX);
}


void setRxMode(void)
{
	halSpiStrobe(CCxxx0_SRX);		//�������״̬
}

/*
// Bit masks corresponding to STATE[2:0] in the status byte returned on MISO
#define CCxx00_STATE_BM                 0x70
#define CCxx00_FIFO_BYTES_AVAILABLE_BM  0x0F
#define CCxx00_STATE_TX_BM              0x20
#define CCxx00_STATE_TX_UNDERFLOW_BM    0x70
#define CCxx00_STATE_RX_BM              0x10
#define CCxx00_STATE_RX_OVERFLOW_BM     0x60
#define CCxx00_STATE_IDLE_BM            0x00

static INT8U RfGetRxStatus(void)
{
	INT8U temp, spiRxStatus1,spiRxStatus2;
	INT8U i=4;// ѭ�����Դ���
    temp = CCxxx0_SNOP|READ_SINGLE;//���Ĵ�������
	CSN = 0;
	while (MISO);
	SpiTxRxByte(temp);
	spiRxStatus1 = SpiTxRxByte(0);
	do
	{
		SpiTxRxByte(temp);
		spiRxStatus2 = SpiTxRxByte(0);
		if(spiRxStatus1 == spiRxStatus2)
		{
			if( (spiRxStatus1 & CCxx00_STATE_BM) == CCxx00_STATE_RX_OVERFLOW_BM)
			{
               halSpiStrobe(CCxxx0_SFRX);
			   return 0;
			}
		    return 1;
		}
		 spiRxStatus1=spiRxStatus2;
	}
	while(i--);
	CSN = 1;
    return 0;	
}
 */
u8 halRfReceivePacket(u8 *rxBuffer, u8 *length) 
{
	u8 status[2];
	u8 packetLength;
	u8 i=(*length)*4; //�������Ҫ����datarate��length������

	/*Ĭ������Ϊ����ģʽ*/
	halSpiStrobe(CCxxx0_SRX);	//�������״̬
	//delay_us(5);
	//while (!GDO1);
	//while (GDO1);
	delay_us(2);
	while (GDO0 == 1)
	{
		delay_us(2);
		--i;
		if(i<1)
			return 0; 	    
	}	 
	if ((halSpiReadStatus(CCxxx0_RXBYTES) & BYTES_IN_RXFIFO)) //����ӵ��ֽ�����Ϊ0
	{
		packetLength = halSpiReadReg(CCxxx0_RXFIFO);//������һ���ֽڣ����ֽ�Ϊ��֡���ݳ���
		if (packetLength <= *length) 		//�����Ҫ����Ч���ݳ���С�ڵ��ڽ��յ������ݰ��ĳ���
		{
			halSpiReadBurstReg(CCxxx0_RXFIFO, rxBuffer, packetLength); //�������н��յ�������
			*length = packetLength;				//�ѽ������ݳ��ȵ��޸�Ϊ��ǰ���ݵĳ���

			// Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
			halSpiReadBurstReg(CCxxx0_RXFIFO, status, 2); 	//����CRCУ��λ
			halSpiStrobe(CCxxx0_SFRX);		//��ϴ���ջ�����
			return (status[1] & CRC_OK);			//���У��ɹ����ؽ��ճɹ�
		}
		else 
		{
			*length = packetLength;
			halSpiStrobe(CCxxx0_SFRX);		//��ϴ���ջ�����
			return 0;
		}
	} 
	else
		return 0;	
}


//main�������ã�ģ����ں���
int cc110x_module_entery(void)
{
	u8 leng = 8; //Ԥ�ƽ���8�ֽ�
	u8 RxBuf[8]={0,0,0,0,0,0,0,0};
	
	if(halRfReceivePacket(RxBuf,&leng))	//������յ�������
	{						
//		if(RxBuf[1]==1) //�жϽ��յ������ݣ�TxBuf[1] = 1~6; //6������
//		{
//			/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
//			TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  		
//			
//			printf("\n\rCC110x rx successful!\n\r");  
//		}
		
		return RxBuf[1];
		
//		switch(RxBuf[1])//�жϽ��յ������ݣ�TxBuf[1] = 1~6; //6������
//		{
//			case 1://PD3
//					
//				break;

//			case 2://PD2 = ��
//					
//				break;

//			case 3://PD1 = ��ɫ
//					
//				break;

//			case 4://PC3
//					
//				break;

//			case 5://PB4 = ϴ�ƽ���
//					
//				break;

//			case 6://PB5 = ����
//					
//				break;
//			
//			
//			default:break;/*error*/	
//		}
//		
//		RxBuf[1] = 0xff; /*���Buf*/
	}
		
	return 0;
}
/*
	if( Remote_KEY_Down(KEY1_PORT,KEY1_PIN)==0 )//PD3
	{
		TxBuf[1] = 1 ;
		tf = 1 ; 	
	}
	if( Remote_KEY_Down(KEY2_PORT,KEY2_PIN)==0 )//PD2 = ��
	{
		TxBuf[1] = 2 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY3_PORT,KEY3_PIN)==0 )//PD1 = ��ɫ
	{
		TxBuf[1] = 3 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY4_PORT,KEY4_PIN)==0 )//PC3
	{
		TxBuf[1] = 4 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY5_PORT,KEY5_PIN)==0 )//PB4 = ϴ�ƽ���
	{
		TxBuf[1] = 5 ;
		tf = 1 ; 	
	}
	if( Remote_KEY_Down(KEY6_PORT,KEY6_PIN)==0 )//PB5 = ����
	{
		TxBuf[1] = 6 ;
		tf = 1 ; 	
	}
*/

/*CC110x test*/
void CC110x_test(void)
{
	u8 leng =0;
	u8 tf =0;
	u8 TxBuf[8]={0};	 //8�ֽڣ������Ҫ���������ݰ�,����ȷ����
	u8 RxBuf[8]={0};
		
	//setRxMode(); /*Ĭ������Ϊ����ģʽ*/
	
	if(tf==0)
	{
		/*sys ms delay*/
		static unsigned long prev_t = 0;
		unsigned long cut_t = get_systick();
		if((cut_t - prev_t)<5000)
		{
			return;
		}//ϵͳdalay 5000ms����ִ�к�������
		prev_t = cut_t;
		/*sys ms delay*/

		/*5000ms����һ������*/
		TxBuf[1] = 1; /*��TxBuf[]���鸳ֵ*/
		tf = 1; 
	}
	
	/*����*/
	if(tf==1) 
	{	
		/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
		TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  	
	
		//USART_printf = 1;/*ѡ�񴮿�1��Ϊprintf���*/
		printf("\n\rCC110x tx test!\n\r");  

		halRfSendPacket(TxBuf,8); // ��������TxBuf[]�����8���ֽ�����
		TxBuf[1] = 0xff; /*���Buf*/
		TxBuf[2] = 0xff; /*���Buf*/
		tf = 0;		
	}
	
	/*����*/
	leng = 8; //Ԥ�ƽ���8�ֽ�
	if(halRfReceivePacket(RxBuf,&leng))	//������յ�������
	{						
		if(RxBuf[1]==1) //�жϽ��յ������ݣ�TxBuf[1] = 1; 
		{
			/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
			TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  		
			
			//USART_printf = 1;/*ѡ�񴮿�1��Ϊprintf���*/
			printf("\n\rCC110x rx successful!\n\r");  
			
			/*test*/
			//test_flag = 2;
			//touzi_num = 0x22;	  
			printf("\n\r\n\rtest num = 22\n\r\n\r");		
			/*test*/		
			
		}
		RxBuf[1] = 0xff; /*���Buf*/
		RxBuf[2] = 0xff; /*���Buf*/
	}
}
		

