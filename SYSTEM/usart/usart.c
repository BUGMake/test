#include "sys.h"
#include "usart.h"	  
#include "ring_buffer.h"
#include "delay.h"
#include "pro.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{   

	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
		USART1->DR = (u8) ch;      
	return ch;
}
#endif 







#define UART_CRC16 2 //UART2_RECV();UART3_RECV();UART4_RECV();UART5_RECV();
										 //uart1 = /*����Э�飺0x55 0xab [����] [У�����8λ] [У�����8λ]*/ = uart1_crc16_test();
#define RX_NUM 5
#define TX_NUM 5
#define HEAD_A 0xff
#define HEAD_B 0xaa
#define LEN 4

//��λ0-4����������S1
#define EAST 0
#define SOUTH 1
#define WEST 2
#define NORTH 3
#define S1_MAIN 4
//��ַ


/*
��ͷ			��λ	CMD		����	����	��β
0xff 0xaa		0-4		0-7		len		data	CRC16

��λ0-4����������S1
CMD 0-7��
0=�÷�λ���Ƶ�������			A83T-->S1
1=�����ط��Ƶ�������			S1-->A83T
2=�������ƣ�					A83T-->S1
3=�������·������ƣ�			S1-->A83T
4=������λ���Ƶ����ݣ�			S1-->A83T������
5=�����ط�������λ���Ƶ����ݣ�	A83T-->S1
6=������						A83T-->S1
7=�����ط�������				S1-->A83T
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
static char uart5_recDataBut[REC_DATA_BUT_SIZE];//���ݻ���


//uart4
static RING_BUF uart4_recv_rgb;
#define UART4_RECV_SIZE 512
static char uart4_recv_buf[UART4_RECV_SIZE];

//RING_BUF uart4_send_rgb;
//#define UART4_SEND_SIZE 1024
//static char uart4_send_buf[UART4_SEND_SIZE];

static RING_BUF uart4_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart4_recDataBut[REC_DATA_BUT_SIZE];//���ݻ���


//uart3
static RING_BUF uart3_recv_rgb;
#define UART3_RECV_SIZE 512
static char uart3_recv_buf[UART3_RECV_SIZE];

//RING_BUF uart3_send_rgb;
//#define UART3_SEND_SIZE 1024
//static char uart3_send_buf[UART3_SEND_SIZE];

static RING_BUF uart3_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart3_recDataBut[REC_DATA_BUT_SIZE];//���ݻ���


//uart2
static RING_BUF uart2_recv_rgb;
#define UART2_RECV_SIZE 512
static char uart2_recv_buf[UART2_RECV_SIZE];

//RING_BUF uart2_send_rgb;
//#define UART2_SEND_SIZE 1024
//static char uart2_send_buf[UART2_SEND_SIZE];

static RING_BUF uart2_recDataRing;
#define REC_DATA_BUT_SIZE 512
static char uart2_recDataBut[REC_DATA_BUT_SIZE];//���ݻ���



//uart1
static RING_BUF uart1_recv_rgb;
#define UART1_RECV_SIZE 512
static char uart1_recv_buf[UART1_RECV_SIZE];

RING_BUF uart1_send_rgb;
#define UART1_SEND_SIZE 1024
static char uart1_send_buf[UART1_SEND_SIZE];

//static RING_BUF uart1_recDataRing;
//#define REC_DATA_BUT_SIZE 512
//static char uart1_recDataBut[REC_DATA_BUT_SIZE];//���ݻ���




//����Э��
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
//��ȡ16λУ����
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


//��ʼ��ѭ��buffer��uart��
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
	
//	init_ring_buf(&uart1_recDataRing,uart1_recDataBut,REC_DATA_BUT_SIZE);//��ʼ����������
	init_ring_buf(&uart2_recDataRing,uart2_recDataBut,REC_DATA_BUT_SIZE);//��ʼ����������
	init_ring_buf(&uart3_recDataRing,uart3_recDataBut,REC_DATA_BUT_SIZE);//��ʼ����������
	init_ring_buf(&uart4_recDataRing,uart4_recDataBut,REC_DATA_BUT_SIZE);//��ʼ����������
	init_ring_buf(&uart5_recDataRing,uart5_recDataBut,REC_DATA_BUT_SIZE);//��ʼ����������

	pctlStep=PCTL_HEAD_STEP;
}

static void UART5_RECV(void)
{
	char data;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) 
	{
		data = USART_ReceiveData(UART5);
		push_buf(&uart5_recv_rgb,&data,1);//�����ڽ��յ��ĵ��ֽ�����д��ѭ��buffer����
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
		push_buf(&uart4_recv_rgb,&data,1);//�����ڽ��յ��ĵ��ֽ�����д��ѭ��buffer����
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
		push_buf(&uart3_recv_rgb,&data,1);//�����ڽ��յ��ĵ��ֽ�����д��ѭ��buffer����
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
		push_buf(&uart2_recv_rgb,&data,1);//�����ڽ��յ��ĵ��ֽ�����д��ѭ��buffer����
		USART_ClearFlag(USART2,USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART2,USART_FLAG_RXNE);
	}
}

static void UART1_RECV(void)//����һ���ֽ�
{
	char data;
	//if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	//{
		data = USART_ReceiveData(USART1);
		push_buf(&uart1_recv_rgb,&data,1);//�����ڽ��յ��ĵ��ֽ�����д��ѭ��buffer����
		USART_ClearFlag(USART1,USART_FLAG_RXNE);
		USART_ClearITPendingBit(USART1,USART_FLAG_RXNE);
	//}
}
static void UART1_SEND()//����һ���ֽ�
{
	int len;
	char ch;
	
	len = pop_buf_t(&uart1_send_rgb,&ch,1);//��һ���ֽڳ���
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

/*��������*/
//static u8 card_num[4] = {34,34,34,34};//4����λ�Ƶ�����
static u8 card_num[4] = {0,0,0,0};//4����λ�Ƶ�����
static u8 card_recv[4][100] = {0};
static u8 uart_to_addr[4] = {0,0,0,0};//4����λ��Ӧ�ĸ�����
//static u8 touzi_num = 0;//����
static u8 touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����
//static u8 test_flag = 0;

#define CMD_addr 						2
#define CMD_cmd 						3
#define CMD_len 						4

#define FINISH_FLAG 				0xFA
#define GET_ALL_FLAG 				0xFB
#define A83T_ASK_FLAG 			0xFC



//�������ݰ�
//������һ��һ�����������ݰ�
//A83Tͨ��Э�飺0xff 0xaa ��λ cmd len data [У�����8λ] [У�����8λ]
 int handle_data_uart(u8 uart,unsigned char *pData,int plen)//uart��ʾ�����ĸ����ڵ�����
{
	char len=0;
	char dataLen=0;
	
	switch(pctlStep)
	{
		
		case PCTL_HEAD_STEP://������ͷ��FF AA
			if(plen>=2)
			{
				if(pData[0]==HEAD_A && pData[1]==HEAD_B)//��⵽֡ͷ��FF AA
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
						if(pData[dataLen-1]==HEAD_A && pData[dataLen]==HEAD_B)//��⵽֡ͷ��FF AA
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
			
		case PCTL_ADD_STEP://������λ��0-4
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
			
		case PCTL_CMD_STEP://�������0-7
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

		case PCTL_LEN_STEP://�������ȣ�len
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
		
		case PCTL_DATA_STEP://�������ݣ�data
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

		case PCTL_CRC16_STEP://����CRC16��β
			if(plen>=len+2)
			{
				pctlStep=PCTL_HEAD_STEP;
				len+=2;

				//����һ�����������ݰ�
				//A83Tͨ��Э�飺0xff 0xaa ��λ cmd len data [У�����8λ] [У�����8λ]
				//{
				//u8 k=0;
				//for(k=0;k<len;k++)
				//printf("%x ",pData[dataLen+k]);
				//}
				//if(S1_pro.PRO_step==PRO_DICE_TO_A83T) printf("--1112\r\n");
				//printf("----\r\n");
				A83T_TO_S1(uart,pData+dataLen,len);//Э�鴦�� void Pctl_handlePro(char *pData,int len)
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
	  static int proDataLen=0;//һ�������ݳ��ȣ����̶�����
	  static int len;	
		/*uart5*/
	  //sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//��ʼ��bufferΪȫ0
	  len = uart5_read(read_uart_buf,READ_UART_BUF_SIZE);//��uart5�����ݷŵ�read_uart_buf����
	  push_buf(&uart5_recDataRing,read_uart_buf,len);//��read_uart_buf����ģ�ȫ�������ݷŵ�ѭ��buffer����
	  len=pop_buf(&uart5_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//��ѭ��buffer���棨ȫ��������������read_uart_buf���� 
	  proDataLen=handle_data_uart(uart_5,(unsigned char *)read_uart_buf,len);//����uart5�����ݣ�������һ��һ�������ݰ�
	  push_buf(&uart5_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//������һ�����ݰ�֮��read_uart_buf����ʣ�µ������ַŻ�ѭ��buffer����  
	  //���û�н��������ݰ����򶪵�ǰ������ݣ��ٰ�read_uart_buf����ʣ�µ�ȫ�������ַŻ�ѭ��buffer����
		
		/*uart4*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//��ʼ��bufferΪȫ0
	  len = uart4_read(read_uart_buf,READ_UART_BUF_SIZE);//��uart4�����ݷŵ�read_uart_buf����
	  push_buf(&uart4_recDataRing,read_uart_buf,len);//��read_uart_buf����ģ�ȫ�������ݷŵ�ѭ��buffer����
	  len=pop_buf(&uart4_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//��ѭ��buffer���棨ȫ��������������read_uart_buf����
	  proDataLen=handle_data_uart(uart_4,(unsigned char *)read_uart_buf,len);//����uart4�����ݣ�������һ��һ�������ݰ�
	  push_buf(&uart4_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//������һ�����ݰ�֮��read_uart_buf����ʣ�µ������ַŻ�ѭ��buffer����  
		//���û�н��������ݰ����򶪵�ǰ������ݣ��ٰ�read_uart_buf����ʣ�µ�ȫ�������ַŻ�ѭ��buffer����
	
		/*uart3*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//��ʼ��bufferΪȫ0
		len = uart3_read(read_uart_buf,READ_UART_BUF_SIZE);//��uart3�����ݷŵ�read_uart_buf����
		push_buf(&uart3_recDataRing,read_uart_buf,len);//��read_uart_buf����ģ�ȫ�������ݷŵ�ѭ��buffer����
	  len=pop_buf(&uart3_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//��ѭ��buffer���棨ȫ��������������read_uart_buf���� 
	  proDataLen=handle_data_uart(uart_3,(unsigned char *)read_uart_buf,len);//����uart3�����ݣ�������һ��һ�������ݰ�
	  push_buf(&uart3_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//������һ�����ݰ�֮��read_uart_buf����ʣ�µ������ַŻ�ѭ��buffer����  
		//���û�н��������ݰ����򶪵�ǰ������ݣ��ٰ�read_uart_buf����ʣ�µ�ȫ�������ַŻ�ѭ��buffer����
	
		/*uart2*/
		//sz_memset(read_uart_buf,0,READ_UART_BUF_SIZE);//��ʼ��bufferΪȫ0	
		len = uart2_read(read_uart_buf,READ_UART_BUF_SIZE);//��uart2�����ݷŵ�read_uart_buf����
		push_buf(&uart2_recDataRing,read_uart_buf,len);//��read_uart_buf����ģ�ȫ�������ݷŵ�ѭ��buffer��?
		len=pop_buf(&uart2_recDataRing,read_uart_buf,REC_DATA_BUT_SIZE);//��ѭ��buffer���棨ȫ��������������read_uart_buf����	
		proDataLen=handle_data_uart(uart_2,(unsigned char *)read_uart_buf,len);//����uart2�����ݣ�������һ��һ�������ݰ�			 
		push_buf(&uart2_recDataRing,&read_uart_buf[proDataLen],len-proDataLen);//������һ�����ݰ�֮��read_uart_buf����ʣ�µ������ַŻ�ѭ��buffer����  
		//���û�н��������ݰ����򶪵�ǰ������ݣ��ٰ�read_uart_buf����ʣ�µ�ȫ�������ַŻ�ѭ��buffer����

}


//������
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
#if 0 //DMA����
	int len;
	len = push_buf_t(&uart1_send_rgb,src,data_len);
	if(DMA_GetChanleStatu(DMA1_Channel2,DISABLE))
	{
		//dma5_send();
	}
	return len;
#else //�жϷ��ͣ�Ӳ����ʼ��Ҫ���жϷ��ͣ��жϽ��պ���Ҫ�����͵����ݣ�
	int len;
	len = push_buf_t(&uart1_send_rgb,src,data_len);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	return len;
#endif	
}

//*********************************************************************************************************************************************************************************





 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
void uart1_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

 	 USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);//���������ж�
  	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
			UART1_SEND();//����һ���ֽ�
} 
#endif	










/*UART2*/
#if EN_USART2_RX   //���ʹ���˽���
//����2�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART2_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART2_RX_STA=0;       //����״̬���	  
  
void uart2_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//ʹ��GPIOAʱ��
  
	//USART2_TX   GPIOA.2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
   
  //USART2_RX	  GPIOA.3��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.3  

  //USART2 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//�����ȼ�4
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART2, &USART_InitStructure); //��ʼ������2
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2 
}

void USART2_IRQHandler(void)                	//����2�жϷ������
{
		UART2_RECV();
} 
#endif


/*UART3*/
#if EN_USART3_RX   //���ʹ���˽���
//����3�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //���ջ���,���USART3_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART3_RX_STA=0;       //����״̬���	  
  
void uart3_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//ʹ��USART3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//ʹ��GPIOBʱ��
  
	//USART3_TX   GPIOB.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10
   
  //USART3_RX	  GPIOB.11��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.11  

  //USART3 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;		//�����ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART3, &USART_InitStructure); //��ʼ������3
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3
}

void USART3_IRQHandler(void)                	//����3�жϷ������
{
		UART3_RECV();
} 
#endif
/*UART4*/
#if EN_UART4_RX   //���ʹ���˽���
//����4�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 UART4_RX_BUF[UART4_REC_LEN];     //���ջ���,���UART4_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 UART4_RX_STA=0;       //����״̬���	  
void uart4_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//ʹ��UART4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��GPIOCʱ��
  
	//UART4_TX   GPIOC.10
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PC.10
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.10
   
  //UART4_RX	  GPIOC.11��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.11  

  //UART4 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;		//�����ȼ�6
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(UART4, &USART_InitStructure); //��ʼ������4
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ���4
}

void UART4_IRQHandler(void)                	//����4�жϷ������
{
		UART4_RECV();	
}
#endif


/*UART5*/
#if EN_UART5_RX   //���ʹ���˽���
//����5�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 UART5_RX_BUF[UART5_REC_LEN];     //���ջ���,���UART5_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 UART5_RX_STA=0;       //����״̬���	 
void uart5_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//ʹ��UART5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);	//ʹ��GPIOC��GPIODʱ��
  	
	//UART5_TX   GPIOC.12
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //PC.12
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOC.12
   
  //UART5_RX	  GPIOD.2��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOD.2  

  //UART5 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;		//�����ȼ�7
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(UART5, &USART_InitStructure); //��ʼ������5
  USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ���5
}

void UART5_IRQHandler(void)                	//����5�жϷ������
{
		UART5_RECV();
} 
#endif





void all_uart_init(void)
{
	init_rgb();//��ʼ��ѭ��buffer��uart��
	
	uart2_init(115200);	 	//����2��ʼ��Ϊ9600��PA2-U2-TX;PA3-U2-RX��
	uart3_init(115200);	 	//����3��ʼ��Ϊ9600��PB10-U3-TX;PB11-U3-RX��
	uart4_init(115200);	 	//����4��ʼ��Ϊ9600��PC10-U4-TX;PC11-U4-RX��
	uart5_init(115200);	 	//����5��ʼ��Ϊ9600��PC12-U5-TX;PD2-U5-RX��
	
//	uart2_init(9600);	 	//����2��ʼ��Ϊ9600��PA2-U2-TX;PA3-U2-RX��
//	uart3_init(9600);	 	//����3��ʼ��Ϊ9600��PB10-U3-TX;PB11-U3-RX��
//	uart4_init(9600);	 	//����4��ʼ��Ϊ9600��PC10-U4-TX;PC11-U4-RX��
//	uart5_init(9600);	 	//����5��ʼ��Ϊ9600��PC12-U5-TX;PD2-U5-RX��
}

