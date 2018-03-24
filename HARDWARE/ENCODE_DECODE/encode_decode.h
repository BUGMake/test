#ifndef __ENCODE_DECODE_H
#define __ENCODE_DECODE_H	 

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "sys.h"


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
#define comLength (5)//ͨ�����ݳ���

/*timer0�ж�һ����0.1ms*/
#define firstLowTime   (50)
#define beginTime      (90)
#define endTime        (50)
#define highTime       (10)
#define lowTimebit1    (10)
#define lowTimebit0    (20)


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void ComDelayMs(u8 ms);
void guidance_code(void);
void Bit0(void);
void Bit1(void);
void epilog_code(void);
void Send_a_byte(u8 byte);
void Send_a_com(u8 com);//����һ�����0x11-0x66
u8 Receive_a_com(u8 rx_buff[],u8 rx_len);//����1��ʾ���յ�5���ֽڵ�����

		 				    
#endif

