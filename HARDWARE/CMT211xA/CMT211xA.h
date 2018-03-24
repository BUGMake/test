#ifndef __CMT211xA_H
#define __CMT211xA_H	 

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "sys.h"
#include "delay.h"
#include "usart.h"	  

//-----------------------------------------------------------------------------
// Pin Declarations
//-----------------------------------------------------------------------------

/*CMT211xA：TX*/
#define CMT211xA_TX  PCout(3)	// PC3
#define MOTOR_OUT    PCout(1)	// PC1
#define MOTOR_IN     PCin(2)  // PC2
#define DC280V_Control    PCout(0)	// PC0


//枚举芯片类型
//enum chipsetType {CMT2110A, CMT2113A, CMT2117A, CMT2119A};
//enum encodeType {E527, E201, ENRZ};
#define	CMT211xA_E527 (0)
#define	CMT211xA_E201 (1)
#define	CMT211xA_ENRZ (2) 

/*注意：数据越长，接收解码的成功率越低*/
#define CMT211xA_DATALength (7)//通信数据长度：str[7]


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void set_pwm_flag(u8 flag);

void CMT211xA_PORT_Init(void);
void CMT211xA_TxPacket(void);/*注意：必须先测试vDelayUs(1000);的实际延时*/
void CMT211xA_Encode(u8 ptr[], u8 length, u8 etype);
void CMT211xA_vDelay416Us_test(void);/*测试vDelayUs(416);的实际延时*/


void MOTOR_OUT_Init(void);
void MOTOR_OUT_ON(void);
void MOTOR_OUT_OFF(void);
void MOTOR_OUT_Toggle(void);

void MOTOR_IN_Init(void);
u8 MOTOR_IN_Check(void);


void DC280V_Control_Init(void);
void DC280V_Control_ON (void);
void DC280V_Control_OFF (void);


void CMT211xA_tx_test(void);
void control_work(u8 touzi_num);

		 				    
#endif
