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

	/*输出：NSS=0;SCK=1;MOSI=1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;				 			 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 				
	GPIO_SetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_7);	
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);		

	/*输入上拉：MISO;GDO0*/  
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

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
//函数名：SpisendByte(INT8U dat)
//输入：发送的数据
//输出：无
//功能描述：SPI发送一个字节
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
//函数名：void RESET_CC1100(void)
//输入：无
//输出：无
//功能描述：复位CC1100
//*****************************************************************************************
void RESET_CC1100(void) 
{
	CSN_L;//CSN = 0; 
	
	while (MISO == 1); 
	SpiTxRxByte(CCxxx0_SRES); 		//写入复位命令
	while (MISO == 1); 
	
	CSN_H;//CSN = 1; 
}

//*****************************************************************************************
//函数名：void POWER_UP_RESET_CC1100(void) 
//输入：无
//输出：无
//功能描述：上电复位CC1100
//*****************************************************************************************
void POWER_UP_RESET_CC1100(void) 
{
  CSN_H;//CSN = 1; 
	halWait(1); 
	CSN_L;//CSN = 0; 
	halWait(1); 
	CSN_H;//CSN = 1; 
	halWait(41); 
	RESET_CC1100();   		//复位CC1100
}

//*****************************************************************************************
//函数名：void halSpiWriteReg(INT8U addr, INT8U value)
//输入：地址和配置字
//输出：无
//功能描述：SPI写寄存器
//*****************************************************************************************
void halSpiWriteReg(u8 addr, u8 value) 
{	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(addr);		//写地址
	SpiTxRxByte(value);		//写入配置
	
	CSN_H;//CSN = 1;
}

//*****************************************************************************************
//函数名：void halSpiWriteBurstReg(INT8U addr, INT8U *buffer, INT8U count)
//输入：地址，写入缓冲区，写入个数
//输出：无
//功能描述：SPI连续写配置寄存器
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
//函数名：void halSpiStrobe(INT8U strobe)
//输入：命令
//输出：无
//功能描述：SPI写命令
//*****************************************************************************************
void halSpiStrobe(u8 strobe) 
{
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(strobe);		//写入命令
	
	CSN_H;//CSN = 1;
}





//*****************************************************************************************
//函数名：INT8U halSpiReadReg(INT8U addr)
//输入：地址
//输出：该寄存器的配置字
//功能描述：SPI读寄存器
//*****************************************************************************************
u8 halSpiReadReg(u8 addr) 
{
	u8 temp, value;
	temp = addr|READ_SINGLE;//读寄存器命令
	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);
	value = SpiTxRxByte(0);
	
	CSN_H;//CSN = 1;
	
	return value;
}


//*****************************************************************************************
//函数名：void halSpiReadBurstReg(INT8U addr, INT8U *buffer, INT8U count)
//输入：地址，读出数据后暂存的缓冲区，读出配置个数
//输出：无
//功能描述：SPI连续写配置寄存器
//*****************************************************************************************
void halSpiReadBurstReg(u8 addr, u8 *buffer, u8 count) 
{
	u8 i,temp;
	temp = addr | READ_BURST;		//写入要读的配置寄存器地址和读命令
	
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
//函数名：INT8U halSpiReadReg(INT8U addr)
//输入：地址
//输出：该状态寄存器当前值
//功能描述：SPI读状态寄存器
//*****************************************************************************************
u8 halSpiReadStatus(u8 addr) 
{
	u8 value,temp;
	temp = addr | READ_BURST;		//写入要读的状态寄存器的地址同时写入读命令
	
	CSN_L;//CSN = 0;
	
	while (MISO == 1);
	SpiTxRxByte(temp);
	value = SpiTxRxByte(0);
	
	CSN_H;//CSN = 1;
	
	return value;
}
//*****************************************************************************************
//函数名：void halRfWriteRfSettings(RF_SETTINGS *pRfSettings)
//输入：无
//输出：无
//功能描述：配置CC1100的寄存器
//*****************************************************************************************
void halRfWriteRfSettings(void) 
{
	halSpiWriteReg(CCxxx0_FSCTRL0,  rfSettings.FSCTRL2);//自已加的
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
//函数名：void halRfSendPacket(INT8U *txBuffer, INT8U size)
//输入：发送的缓冲区，发送数据个数
//输出：无
//功能描述：CC1100发送一组数据
//*****************************************************************************************

void halRfSendPacket(u8 *txBuffer, u8 size) 
{
	halSpiWriteReg(CCxxx0_TXFIFO, size);
	halSpiWriteBurstReg(CCxxx0_TXFIFO, txBuffer, size);	//写入要发送的数据

	halSpiStrobe(CCxxx0_STX);		//进入发送模式发送数据（设置为接收模式）

	// Wait for GDO0 to be set -> sync transmitted
	while (GDO0 == 0);
	// Wait for GDO0 to be cleared -> end of packet
	while (GDO0 == 1);
	halSpiStrobe(CCxxx0_SFTX);
}


void setRxMode(void)
{
	halSpiStrobe(CCxxx0_SRX);		//进入接收状态
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
	INT8U i=4;// 循环测试次数
    temp = CCxxx0_SNOP|READ_SINGLE;//读寄存器命令
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
	u8 i=(*length)*4; //具体多少要根据datarate和length来决定

	/*默认设置为接收模式*/
	halSpiStrobe(CCxxx0_SRX);	//进入接收状态
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
	if ((halSpiReadStatus(CCxxx0_RXBYTES) & BYTES_IN_RXFIFO)) //如果接的字节数不为0
	{
		packetLength = halSpiReadReg(CCxxx0_RXFIFO);//读出第一个字节，此字节为该帧数据长度
		if (packetLength <= *length) 		//如果所要的有效数据长度小于等于接收到的数据包的长度
		{
			halSpiReadBurstReg(CCxxx0_RXFIFO, rxBuffer, packetLength); //读出所有接收到的数据
			*length = packetLength;				//把接收数据长度的修改为当前数据的长度

			// Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
			halSpiReadBurstReg(CCxxx0_RXFIFO, status, 2); 	//读出CRC校验位
			halSpiStrobe(CCxxx0_SFRX);		//清洗接收缓冲区
			return (status[1] & CRC_OK);			//如果校验成功返回接收成功
		}
		else 
		{
			*length = packetLength;
			halSpiStrobe(CCxxx0_SFRX);		//清洗接收缓冲区
			return 0;
		}
	} 
	else
		return 0;	
}


//main函数调用：模块入口函数
int cc110x_module_entery(void)
{
	u8 leng = 8; //预计接收8字节
	u8 RxBuf[8]={0,0,0,0,0,0,0,0};
	
	if(halRfReceivePacket(RxBuf,&leng))	//处理接收到的数据
	{						
//		if(RxBuf[1]==1) //判断接收到的数据：TxBuf[1] = 1~6; //6个按键
//		{
//			/*注意：使用USART1时必须要关闭PA8-PWM输出*/
//			TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  		
//			
//			printf("\n\rCC110x rx successful!\n\r");  
//		}
		
		return RxBuf[1];
		
//		switch(RxBuf[1])//判断接收到的数据：TxBuf[1] = 1~6; //6个按键
//		{
//			case 1://PD3
//					
//				break;

//			case 2://PD2 = 东
//					
//				break;

//			case 3://PD1 = 打色
//					
//				break;

//			case 4://PC3
//					
//				break;

//			case 5://PB4 = 洗牌结束
//					
//				break;

//			case 6://PB5 = 升降
//					
//				break;
//			
//			
//			default:break;/*error*/	
//		}
//		
//		RxBuf[1] = 0xff; /*清除Buf*/
	}
		
	return 0;
}
/*
	if( Remote_KEY_Down(KEY1_PORT,KEY1_PIN)==0 )//PD3
	{
		TxBuf[1] = 1 ;
		tf = 1 ; 	
	}
	if( Remote_KEY_Down(KEY2_PORT,KEY2_PIN)==0 )//PD2 = 东
	{
		TxBuf[1] = 2 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY3_PORT,KEY3_PIN)==0 )//PD1 = 打色
	{
		TxBuf[1] = 3 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY4_PORT,KEY4_PIN)==0 )//PC3
	{
		TxBuf[1] = 4 ;
		tf = 1 ; 	
	}	
	if( Remote_KEY_Down(KEY5_PORT,KEY5_PIN)==0 )//PB4 = 洗牌结束
	{
		TxBuf[1] = 5 ;
		tf = 1 ; 	
	}
	if( Remote_KEY_Down(KEY6_PORT,KEY6_PIN)==0 )//PB5 = 升降
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
	u8 TxBuf[8]={0};	 //8字节，如果需要更长的数据包,请正确设置
	u8 RxBuf[8]={0};
		
	//setRxMode(); /*默认设置为接收模式*/
	
	if(tf==0)
	{
		/*sys ms delay*/
		static unsigned long prev_t = 0;
		unsigned long cut_t = get_systick();
		if((cut_t - prev_t)<5000)
		{
			return;
		}//系统dalay 5000ms，才执行后面的语句
		prev_t = cut_t;
		/*sys ms delay*/

		/*5000ms发送一次数据*/
		TxBuf[1] = 1; /*给TxBuf[]数组赋值*/
		tf = 1; 
	}
	
	/*发送*/
	if(tf==1) 
	{	
		/*注意：使用USART1时必须要关闭PA8-PWM输出*/
		TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  	
	
		//USART_printf = 1;/*选择串口1作为printf输出*/
		printf("\n\rCC110x tx test!\n\r");  

		halRfSendPacket(TxBuf,8); // 发送数组TxBuf[]里面的8个字节数据
		TxBuf[1] = 0xff; /*清除Buf*/
		TxBuf[2] = 0xff; /*清除Buf*/
		tf = 0;		
	}
	
	/*接收*/
	leng = 8; //预计接收8字节
	if(halRfReceivePacket(RxBuf,&leng))	//处理接收到的数据
	{						
		if(RxBuf[1]==1) //判断接收到的数据：TxBuf[1] = 1; 
		{
			/*注意：使用USART1时必须要关闭PA8-PWM输出*/
			TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  		
			
			//USART_printf = 1;/*选择串口1作为printf输出*/
			printf("\n\rCC110x rx successful!\n\r");  
			
			/*test*/
			//test_flag = 2;
			//touzi_num = 0x22;	  
			printf("\n\r\n\rtest num = 22\n\r\n\r");		
			/*test*/		
			
		}
		RxBuf[1] = 0xff; /*清除Buf*/
		RxBuf[2] = 0xff; /*清除Buf*/
	}
}
		

