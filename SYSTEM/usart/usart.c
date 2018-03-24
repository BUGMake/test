#include "sys.h"
#include "usart.h"	  
#include "ring_buffer.h"
#include "delay.h"
#include "pro.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 

//重定义fputc函数 
int fputc(int ch, FILE *f)
{   

	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
		USART1->DR = (u8) ch;      
	return ch;
}
#endif 







#define UART_CRC16 2 //UART2_RECV();UART3_RECV();UART4_RECV();UART5_RECV();
										 //uart1 = /*接收协议：0x55 0xab [数字] [校验码低8位] [校验码高8位]*/ = uart1_crc16_test();
#define RX_NUM 5
#define TX_NUM 5
#define HEAD_A 0xff
#define HEAD_B 0xaa
#define LEN 4

//方位0-4：东南西北S1
#define EAST 0
#define SOUTH 1
#define WEST 2
#define NORTH 3
#define S1_MAIN 4
//地址


/*
包头			方位	CMD		长度	数据	结尾
0xff 0xaa		0-4		0-7		len		data	CRC16

方位0-4：东南西北S1
CMD 0-7：
0=该方位的牌的数量；			A83T-->S1
1=请求重发牌的数量；			S1-->A83T
2=发单个牌；					A83T-->S1
3=请求重新发单个牌；			S1-->A83T
4=整个方位的牌的数据；			S1-->A83T（东）
5=请求重发整个方位的牌的数据；	A83T-->S1
6=点数；						A83T-->S1
7=请求重发点数；				S1-->A83T
*/
//uart5
static RING_BUF uart5_recv_rgb;
#define UART5_RECV_SIZE 512
static char uart5_recv_buf[UART5_RECV_SIZE];

//RING_BUF uart5_send_rgb;
//#define UART5_SEND_SIZE 1024
//static char uart5_send_buf[UART5_SEND_SIZE];

static RING_BUF uart5_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart5_recDataBut[REC_DATA_BUT_SIZE];//数据缓存


//uart4
static RING_BUF uart4_recv_rgb;
#define UART4_RECV_SIZE 512
static char uart4_recv_buf[UART4_RECV_SIZE];

//RING_BUF uart4_send_rgb;
//#define UART4_SEND_SIZE 1024
//static char uart4_send_buf[UART4_SEND_SIZE];

static RING_BUF uart4_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart4_recDataBut[REC_DATA_BUT_SIZE];//数据缓存


//uart3
static RING_BUF uart3_recv_rgb;
#define UART3_RECV_SIZE 512
static char uart3_recv_buf[UART3_RECV_SIZE];

//RING_BUF uart3_send_rgb;
//#define UART3_SEND_SIZE 1024
//static char uart3_send_buf[UART3_SEND_SIZE];

static RING_BUF uart3_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart3_recDataBut[REC_DATA_BUT_SIZE];//数据缓存


//uart2
static RING_BUF uart2_recv_rgb;
#define UART2_RECV_SIZE 512
static char uart2_recv_buf[UART2_RECV_SIZE];

//RING_BUF uart2_send_rgb;
//#define UART2_SEND_SIZE 1024
//static char uart2_send_buf[UART2_SEND_SIZE];

static RING_BUF uart2_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart2_recDataBut[REC_DATA_BUT_SIZE];//数据缓存



//uart1
static RING_BUF uart1_recv_rgb;
#define UART1_RECV_SIZE 512
static char uart1_recv_buf[UART1_RECV_SIZE];

RING_BUF uart1_send_rgb;
#define UART1_SEND_SIZE 1024
static char uart1_send_buf[UART1_SEND_SIZE];

//static RING_BUF uart1_recDataRing;
//#define REC_DATA_BUT_SIZE 512
//static char uart1_recDataBut[REC_DATA_BUT_SIZE];//数据缓存




//定义协议
typedef enum {
PCTL_HEAD_STEP=0,
PCTL_ADD_STEP,
PCTL_CMD_STEP,
PCTL_LEN_STEP,
PCTL_DATA_STEP,
PCTL_CRC16_STEP,
}PCTL_STEP_ENUM;

PCTL_STEP_ENUM pctlStep;


//////////////////////////
//获取16位校验码
u16 GetCrc(const u8 *pSendBuf, u8 nEnd)
{
	u16 wCrc;
	u8 i,j;
	wCrc = 0xFFFF;
	for (i = 0; i<nEnd; i++)
	{
		wCrc ^= pSendBuf[i];
		for (j = 0; j<8; j++)
		{
			if (wCrc & 1)
			{
				wCrc >>= 1;
				wCrc ^= 0xA001;
			}
			else
			{
				wCrc >>= 1;
			}
		}
	}
	return wCrc;
}
/*UART CRC16 test*/
////////////////////////


//初始化循环buffer给uart用
 void init_rgb(void)
{
	init_ring_buf(&uart5_recv_rgb,uart5_recv_buf,UART5_RECV_SIZE);
//	init_ring_buf(&uart5_send_rgb,uart5_send_buf,UART5_SEND_SIZE);

	init_ring_buf(&uart4_recv_rgb,uart4_recv_buf,UART4_RECV_SIZE);
//	init_ring_buf(&uart4_send_rgb,uart4_send_buf,UART4_SEND_SIZE);

	init_ring_buf(&uart3_recv_rgb,uart3_recv_buf,UART3_RECV_SIZE);
//	init_ring_buf(&uart3_send_rgb,uart3_send_buf,UART3_SEND_SIZE);

	init_ring_buf(&uart2_recv_rgb,uart2_recv_buf,UART2_RECV_SIZE);
//	init_ring_buf(&uart2_send_rgb,uart2_send_buf,UART2_SEND_SIZE);

	init_ring_buf(&uart1_recv_rgb,uart1_recv_buf,UART1_RECV_SIZE);
	init_ring_buf(&uart1_send_rgb,uart1_send_buf,UART1_SEND_SIZE);
	
//	init_ring_buf(&uart1_recDataRing,uart1_recDataBut,REC_DATA_BUT_SIZE);//初始化接收数据
	init_ring_buf(&uart2_recDataRing,uart2_recDataBut,REC_DATA_BUT_SIZE);//初始化接收数据
	init_ring_buf(&uart3_recDataRing,uart3_recDataBut,REC_DATA_BUT_SIZE);//初始化接收数据
	init_ring_buf(&uart4_recDataRing,uart4_recDataBut,REC_DATA_BUT_SIZE);//初始化接收数据
	init_ring_buf(&uart5_recDataRing,uart5_recDataBut,REC_DATA_BUT_SIZE);//初始化接收数据

	pctlStep=PCTL_HEAD_STEP;
}

static void UART5_RECV(void)
{
	char data;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) 
	{
		data = USART_ReceiveData(UART5);
		push_buf(&uart5_recv_rgb,&data,1);//将串口接收到的单字节数据写进循环buffer里面
		USART_ClearFlag(UART5,USART_FLAG_RXNE);
		USART_ClearITPendingBit(UART5,USART_FLAG_RXNE);
	}
}

static void UART4_RECV(void)
{
	char data;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) 
	{
		data = USART_ReceiveData(UART4);
		push_buf(&uart4_recv_rgb,&data,1);//将串口接收到的单字节数据写进循环buffer里面
		USART_ClearFlag(UART4,USART_FLAG_RXNE);
		USART_ClearITPendingBit(UART4,USART_FLAG_RXNE);
	}
}

static void UART3_RECV(void)
{
	char data;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) 
	{
		data = USART_ReceiveData(USART3);
		push_buf(&uart3_recv_rgb,&data,1);//将串口接收到的单字节数据写进循环buffer里面
		USART_ClearFlag(USART3,USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART3,USART_FLAG_RXNE);
	}
}

static void UART2_RECV(void)
{
	char data;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		data = USART_ReceiveData(USART2);
		push_buf(&uart2_recv_rgb,&data,1);//将串口接收到的单字节数据写进循环buffer里面
		USART_ClearFlag(USART2,USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART2,USART_FLAG_RXNE);
	}
}

static void UART1_RECV(void)//接收一个字节
{
	char data;
	//if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	//{
		data = USART_ReceiveData(USART1);
		push_buf(&uart1_recv_rgb,&data,1);//将串口接收到的单字节数据写进循环buffer里面
		USART_ClearFlag(USART1,USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	//}
}
static void UART1_SEND()//发送一个字节
{
	int len;
	char ch;
	
	len = pop_buf_t(&uart1_send_rgb,&ch,1);//读一个字节出来
	if(len>0)
	{
		USART_SendData(USART1,ch);
	}
	else
	{
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	}
	USART_ClearFlag(USART1,USART_IT_TXE);
	USART_ClearITPendingBit(USART1, USART_IT_TXE);  
}

/*东南西北*/
//static u8 card_num[4] = {34,34,34,34};//4个方位牌的数量
static u8 card_num[4] = {0,0,0,0};//4个方位牌的数量
static u8 card_recv[4][100] = {0};
static u8 uart_to_addr[4] = {0,0,0,0};//4个方位对应哪个串口
//static u8 touzi_num = 0;//点数
static u8 touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数
//static u8 test_flag = 0;

#define CMD_addr 						2
#define CMD_cmd 						3
#define CMD_len 						4

#define FINISH_FLAG 				0xFA
#define GET_ALL_FLAG 				0xFB
#define A83T_ASK_FLAG 			0xFC



//解析数据包
//解析成一个一个完整的数据包
//A83T通信协议：0xff 0xaa 方位 cmd len data [校验码低8位] [校验码高8位]
 int handle_data_uart(u8 uart,unsigned char *pData,int plen)//uart表示处理哪个串口的数据
{
	char len=0;
	char dataLen=0;
	
	switch(pctlStep)
	{
		
		case PCTL_HEAD_STEP://解析包头：FF AA
			if(plen>=2)
			{
				if(pData[0]==HEAD_A && pData[1]==HEAD_B)//检测到帧头：FF AA
				{
					pctlStep=PCTL_ADD_STEP;
					len+=2;
				}	
				else
				{
					while(plen>=2) 
					{
						plen-=1;
						dataLen+=1;
						if(pData[dataLen-1]==HEAD_A && pData[dataLen]==HEAD_B)//检测到帧头：FF AA
						{
							pctlStep=PCTL_ADD_STEP;
							len+=2;
							break;
						}	
					}	
					//return 	dataLen;						
				}	
			}
			else
			{
				pctlStep=PCTL_HEAD_STEP;
				//printf("head e\r\n");
				break;
			}			
			
		case PCTL_ADD_STEP://解析方位：0-4
			if(plen>=3)
			{
				pctlStep=PCTL_CMD_STEP;
				len+=1;
			}
			else
			{
				pctlStep=PCTL_HEAD_STEP;
				
				//printf("dir e\r\n");
				
				break;    
			}			
			
		case PCTL_CMD_STEP://解析命令：0-7
			if(plen>=4)
			{
				pctlStep=PCTL_LEN_STEP;
				len+=1;
			}
			else
			{
				pctlStep=PCTL_HEAD_STEP;
				//printf("cmd e\r\n");
				break;	
			}	

		case PCTL_LEN_STEP://解析长度：len
			if(plen>=5)
			{
				pctlStep=PCTL_DATA_STEP;
				len+=1;
				//dataLen=pData[len-1];
			}
			else
			{
				pctlStep=PCTL_HEAD_STEP;
				
				//printf("len e\r\n");
		
				break;	
			}		
		
		case PCTL_DATA_STEP://解析数据：data
			if(plen>=pData[len-1]+len)
			{
				pctlStep=PCTL_CRC16_STEP;
				len+=pData[len-1];
			}
			else
			{
				pctlStep=PCTL_HEAD_STEP;
				//printf("data e\r\n");
				break;	 
			}		

		case PCTL_CRC16_STEP://解析CRC16结尾
			if(plen>=len+2)
			{
				pctlStep=PCTL_HEAD_STEP;
				len+=2;

				//处理一个完整的数据包
				//A83T通信协议：0xff 0xaa 方位 cmd len data [校验码低8位] [校验码高8位]
				//{
				//u8 k=0;
				//for(k=0;k<len;k++)
				//printf("%x ",pData[dataLen+k]);
				//}
				//if(S1_pro.PRO_step==PRO_DICE_TO_A83T) printf("--1112\r\n");
				//printf("----\r\n");
				A83T_TO_S1(uart,pData+dataLen,len);//协议处理 void Pctl_handlePro(char *pData,int len)
				return 	dataLen+len;	
			}
			else
			{
	           //if(S1_pro.PRO_step==PRO_DICE_TO_A83T) 
			   //	{
              //
				// {	u8 k=0;
				  // for(k=0;k<len;k++)
				    //printf("%x ",pData[dataLen+k]);
				//}
				 //printf("=====\r\n");
				  //pctlStep=PCTL_HEAD_STEP;
                   //return 	dataLen+len; 
				//}
				pctlStep=PCTL_HEAD_STEP;
				break;	 
			}				
		
		default:
			break;
	} 
	return dataLen;
}



#define uart_1 1
#define uart_2 2
#define uart_3 3
#define uart_4 4
#define uart_5 5


#define READ_UART_BUF_SIZE 512
void usart_pro(void)
{
	  static char read_uart_buf[READ_UART_BUF_SIZE]={0};
	  static int proDataLen=0;//一个包数据长度：不固定长度
	  static int len;	
		/*uart5*/
	  //sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//初始化buffer为全0
	  len = uart5_read(read_uart_buf,READ_UART_BUF_SIZE);//从uart5读数据放到read_uart_buf里面
	  push_buf(&uart5_recDataRing,read_uart_buf,len);//将read_uart_buf里面的（全部）数据放到循环buffer里面
	  len=pop_buf(&uart5_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//从循环buffer里面（全部）读出来放在read_uart_buf里面 
	  proDataLen=handle_data_uart(uart_5,(unsigned char *)read_uart_buf,len);//处理uart5的数据：解析成一个一个的数据包
	  push_buf(&uart5_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//解析完一个数据包之后，read_uart_buf里面剩下的数据又放回循环buffer里面  
	  //如果没有解析到数据包，则丢掉前面的数据，再把read_uart_buf里面剩下的全部数据又放回循环buffer里面
		
		/*uart4*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//初始化buffer为全0
	  len = uart4_read(read_uart_buf,READ_UART_BUF_SIZE);//从uart4读数据放到read_uart_buf里面
	  push_buf(&uart4_recDataRing,read_uart_buf,len);//将read_uart_buf里面的（全部）数据放到循环buffer里面
	  len=pop_buf(&uart4_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//从循环buffer里面（全部）读出来放在read_uart_buf里面
	  proDataLen=handle_data_uart(uart_4,(unsigned char *)read_uart_buf,len);//处理uart4的数据：解析成一个一个的数据包
	  push_buf(&uart4_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//解析完一个数据包之后，read_uart_buf里面剩下的数据又放回循环buffer里面  
		//如果没有解析到数据包，则丢掉前面的数据，再把read_uart_buf里面剩下的全部数据又放回循环buffer里面
	
		/*uart3*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//初始化buffer为全0
		len = uart3_read(read_uart_buf,READ_UART_BUF_SIZE);//从uart3读数据放到read_uart_buf里面
		push_buf(&uart3_recDataRing,read_uart_buf,len);//将read_uart_buf里面的（全部）数据放到循环buffer里面
	  len=pop_buf(&uart3_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//从循环buffer里面（全部）读出来放在read_uart_buf里面 
	  proDataLen=handle_data_uart(uart_3,(unsigned char *)read_uart_buf,len);//处理uart3的数据：解析成一个一个的数据包
	  push_buf(&uart3_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//解析完一个数据包之后，read_uart_buf里面剩下的数据又放回循环buffer里面  
		//如果没有解析到数据包，则丢掉前面的数据，再把read_uart_buf里面剩下的全部数据又放回循环buffer里面
	
		/*uart2*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//初始化buffer为全0	
		len = uart2_read(read_uart_buf,READ_UART_BUF_SIZE);//从uart2读数据放到read_uart_buf里面
		push_buf(&uart2_recDataRing,read_uart_buf,len);//将read_uart_buf里面的（全部）数据放到循环buffer里?
		len=pop_buf(&uart2_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//从循环buffer里面（全部）读出来放在read_uart_buf里面	
		proDataLen=handle_data_uart(uart_2,(unsigned char *)read_uart_buf,len);//处理uart2的数据：解析成一个一个的数据包			 
		push_buf(&uart2_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//解析完一个数据包之后，read_uart_buf里面剩下的数据又放回循环buffer里面  
		//如果没有解析到数据包，则丢掉前面的数据，再把read_uart_buf里面剩下的全部数据又放回循环buffer里面

}


//读数据
int uart5_read(char *src,int data_len)
{
	return pop_buf(&uart5_recv_rgb,src,data_len);
}
int uart4_read(char *src,int data_len)
{
	return pop_buf(&uart4_recv_rgb,src,data_len);
}
int uart3_read(char *src,int data_len)
{
	return pop_buf(&uart3_recv_rgb,src,data_len);
}
int uart2_read(char *src,int data_len)
{
	return pop_buf(&uart2_recv_rgb,src,data_len);
}

int usart1_read(char *src,int data_len)
{
	return pop_buf_t(&uart1_recv_rgb,src,data_len);
}

int usart1_write(char *src,int data_len)
{
#if 0 //DMA发送
	int len;
	len = push_buf_t(&uart1_send_rgb,src,data_len);
	if(DMA_GetChanleStatu(DMA1_Channel2,DISABLE))
	{
		//dma5_send();
	}
	return len;
#else //中断发送（硬件初始化要开中断发送，中断接收函数要处理发送的数据）
	int len;
	len = push_buf_t(&uart1_send_rgb,src,data_len);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	return len;
#endif	
}

//*********************************************************************************************************************************************************************************





 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
void uart1_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

 	 USART_Init(USART1, &USART_InitStructure); //初始化串口1
  	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);//开启发送中断
  	USART_Cmd(USART1, ENABLE);                    //使能串口1 
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
			UART1_SEND();//发送一个字节
} 
#endif	










/*UART2*/
#if EN_USART2_RX   //如果使能了接收
//串口2中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART2_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART2_RX_STA=0;       //接收状态标记	  
  
void uart2_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//使能GPIOA时钟
  
	//USART2_TX   GPIOA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
   
  //USART2_RX	  GPIOA.3初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

  //USART2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//子优先级4
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART2, &USART_InitStructure); //初始化串口2
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART2, ENABLE);                    //使能串口2 
}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
		UART2_RECV();
} 
#endif


/*UART3*/
#if EN_USART3_RX   //如果使能了接收
//串口3中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART3_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART3_RX_STA=0;       //接收状态标记	  
  
void uart3_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//使能GPIOB时钟
  
	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10
   
  //USART3_RX	  GPIOB.11初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.11  

  //USART3 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;		//子优先级5
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART3, &USART_InitStructure); //初始化串口3
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART3, ENABLE);                    //使能串口3
}

void USART3_IRQHandler(void)                	//串口3中断服务程序
{
		UART3_RECV();
} 
#endif
/*UART4*/
#if EN_UART4_RX   //如果使能了接收
//串口4中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 UART4_RX_BUF[UART4_REC_LEN];     //接收缓冲,最大UART4_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 UART4_RX_STA=0;       //接收状态标记	  
void uart4_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//使能UART4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能GPIOC时钟
  
	//UART4_TX   GPIOC.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.10
   
  //UART4_RX	  GPIOC.11初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.11  

  //UART4 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;		//子优先级6
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(UART4, &USART_InitStructure); //初始化串口4
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(UART4, ENABLE);                    //使能串口4
}

void UART4_IRQHandler(void)                	//串口4中断服务程序
{
		UART4_RECV();	
}
#endif


/*UART5*/
#if EN_UART5_RX   //如果使能了接收
//串口5中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 UART5_RX_BUF[UART5_REC_LEN];     //接收缓冲,最大UART5_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 UART5_RX_STA=0;       //接收状态标记	 
void uart5_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//使能UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);	//使能GPIOC和GPIOD时钟
  	
	//UART5_TX   GPIOC.12
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC.12
   
  //UART5_RX	  GPIOD.2初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOD.2  

  //UART5 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;		//子优先级7
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(UART5, &USART_InitStructure); //初始化串口5
  USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(UART5, ENABLE);                    //使能串口5
}

void UART5_IRQHandler(void)                	//串口5中断服务程序
{
		UART5_RECV();
} 
#endif





void all_uart_init(void)
{
	init_rgb();//初始化循环buffer给uart用
	
	uart2_init(115200);	 	//串口2初始化为9600（PA2-U2-TX;PA3-U2-RX）
	uart3_init(115200);	 	//串口3初始化为9600（PB10-U3-TX;PB11-U3-RX）
	uart4_init(115200);	 	//串口4初始化为9600（PC10-U4-TX;PC11-U4-RX）
	uart5_init(115200);	 	//串口5初始化为9600（PC12-U5-TX;PD2-U5-RX）
	
//	uart2_init(9600);	 	//串口2初始化为9600（PA2-U2-TX;PA3-U2-RX）
//	uart3_init(9600);	 	//串口3初始化为9600（PB10-U3-TX;PB11-U3-RX）
//	uart4_init(9600);	 	//串口4初始化为9600（PC10-U4-TX;PC11-U4-RX）
//	uart5_init(9600);	 	//串口5初始化为9600（PC12-U5-TX;PD2-U5-RX）
}

