#ifndef __PRO_H
#define __PRO_H
#include "sys.h"
#include "stdio.h"

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "pwm.h"
#include "adc.h"
#include "stmflash.h"   
#include "usmart.h"

#include "CMT211xA.h"
#include "CC110x.h"
#include "encode_decode.h"


//���ܵ��Ƶ�����
#define CAER_REC_NUM 100
#define CAER_REC_FLAG 0xbb

#define CARD_DIRTION_NUM 4 //��λ�ĸ��� 1�� 2���� 3������ 4��������


#define PCTL_PRO_ONLY (0)  //ֻ����Э��
#define REC_DATA_PRINTF (1)//���������Ƿ��ӡ
#define SEND_DATA_PRINTF (1) //���������Ƿ��ӡ
/*



CMD_CARD_NUMBER = 0,   // ������
CMD_CARD_NUMBER_REQUEST = 1, //����������
CMD_CARD_SEND = 2,			 // �Ƶ������ݵķ���
CMD_CARD_SEND_REQUEST = 3,	 //��������
CMD_CARD_DATA = 4,			 //����S1����������
CMD_CARD_DATA_REQUEST = 5,	//��S1����һ����λ����
CMD_DICE_RESULT = 6,		 // ���̽��
CMD_DICE_RESULT_REQUEST = 7, // ���̽������ ��ʱ����
CMD_SHUFFLE_CARD_END				= 8, //ϴ�ƽ�����(ϴ�ƽ�����ʼ���������  �����ɹ�����Ƶ�����)
CMD_PLAY_DICE				   = 9, // ��ׯ�ҷ�λ �����ػ������� �������һ��û�н���������ĸ�S1������Ż�ʹ�� ��ͬ ���̽������
CMD_CLEAR_PLAY_DICE 			= 10,//�������
CMD_CAER_DATA_TO_S1 		   = 11,// ������λ�ķ��͸�S1;
CMD_OPEN_CAMERA 			= 12, // ����
CMD_CLOSE_CAMERA			= 13, // �ص�

��������
//static u8 card_num[4] = {34,34,34,34};//4����λ�Ƶ�����
static u8 card_num[4] = {0,0,0,0};//4����λ�Ƶ�����
static u8 card_recv[4][100] = {0};
static u8 uart_to_addr[4] = {0,0,0,0};//4����λ��Ӧ�ĸ�����
//static u8 touzi_num = 0;//����
static u8 touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����
//static u8 test_flag = 0;


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

#define CMD_CARD_NUM 						0
#define CMD_RESEND_CARD_NUM 		1
#define CMD_SINGLE_CARD 				2
#define CMD_RESEND_SINGLE_CARD 	3
#define CMD_ALL_DIR_CARD 				4
#define CMD_RESEND_ALL_DIR_CARD 5
#define CMD_DICE_NUM 						6
#define CMD_RESEND_DICE_NUM 		7
#define CMD_XIPAIJIESHU 				8
#define CMD_SEND_BP 						9
#define CMD_DASEJIESHU 				  10
data��ID VALUE��
//A83Tͨ��Э�飺0xff 0xaa ��λ cmd len data [У�����8λ] [У�����8λ]
                  0    1   2    3    4   5      6              
*/

enum{
PTLC_FALSE=0,
PTLC_TRUE=!PTLC_FALSE,
};

typedef enum
{
  CMD_CARD_NUM =0,//            0=�÷�λ���Ƶ�������			A83T-->S1
  CMD_RESEND_CARD_NUM =1,//		1=�����ط��Ƶ�������			S1-->A83T
  CMD_SINGLE_CARD=2,//			2=�������ƣ�					A83T-->S1
  CMD_RESEND_SINGLE_CARD=3,//	3=�������·������ƣ�			S1-->A83T
  CMD_ALL_DIR_CARD=4,//         4=������λ���Ƶ����ݣ�			S1-->A83T������
  CMD_RESEND_ALL_DIR_CARD=5,//  5=�����ط�������λ���Ƶ����ݣ�	A83T-->S1
  CMD_DICE_NUM=6,//				6=������						A83T-->S1
  CMD_RESEND_DICE_NUM=7,//		7=�����ط�������				S1-->A83T
  CMD_SHUFFLE_CARD_END=8,//     8=ϴ�ƽ���:             S1-->A83T
  CMD_CLEAR_PLAY_DICE=10,////   10=�������         S1-->A83T
}PCLT_ENUM;//Э��ö��

/* //����Ƿ��н����ź�
  1.�����Ƶ�����
  2.�����ź�
  3.�����ĸ���λ����
  4.��������λ
  5.�������(ң�ؿ���֪��ׯ��)
  6.���ܵ��������ɫ
*///contol


//��λ0-4����������S1
typedef enum{
	dong_DIR=0,
	nan_DIR=1,
	xi_DIR=2,
	bei_DIR=3,
	s_DIR=4,
}DIR_ENUM;


typedef enum
{
  PRO_CARD_NUM_TO_A83T=0,//�����Ƶ�����
  PRO_CARD_ALL_CARD_TO_A83T=1,//�����ĸ���λ����
  PRO_CARD_SEND_TO_A83T_DONG=2,//��������λ
  PRO_DICE_TO_A83T=3,//�������(ң�ؿ���֪��ׯ��)
  PRO_PLAY_DICE=4,//���ܵ��������ɫ
  PRO_END=5,//���ƽ���
}S1_PRO_ENUM;//������


typedef enum
{
USART1_TYPE=1,
USART2_TYPE=2,//dong
USART3_TYPE=3,//nan
USART4_TYPE=4,//xi
USART5_TYPE=5,//bei
}USART_ENUM;

//��ɫ��������
typedef enum{
NORMAL_TYPE=0,//������ɫ
CNTROL_TYPE=1,//���ƴ�ɫ
}DICE_ENUM;

//ɫ�ӽṹ��
//typedef struct{
// DICE_ENUM dice1_type;//��ɫ����
//
//}DICE_STRUCT;

#define DICE_TIMER_TICK   (180*1000)//ÿ�δ�ɫҪ�ȴ�3����
#define WASH_TIMER_TICK  1000 //1s
#define TEMP_TIMER_TICK  5000 //5s
#define CHARGE_TIMER_TICK 10000 //10s


//Ӳ���ṹ��
typedef struct{
////////////////////Ӳ����־
//1.���߳��
//2.�¶Ȳɼ�
//3.���״̬
//4.������ȡ
//5.ϴ�ƽ����źŻ�ȡ

 u8 chargeFlag;//����־
 u16 chargeFlagCnt;//������߼���
 u32 chargeCnt;//��ɫ����
 
 u8 tempFlag;//�¶ȼ��
 u16 tempValue;
 u16 tempCnt;//�¶ȼ�����
 //ϴ�Ƶ����Ǳ�ʾ����ϴ�ƣ�ϴ�Ƶ���ʱ��
 u8 washedFalg;//ϴ�ƽ�����־
 u8 washTimes;//
 u8 washTimesCnt;//
 u16 washCnt;//ϴ�Ƽ���

 
 u8 keylastValue;//�ϴεİ���ֵ
 u8 keyGetValue;//������ȡ
 u8 kwylastValueFlash;//
 u8 keyGetFlag;//
 
}HARDWARE_STRUCT;


typedef struct
{
 PCLT_ENUM PCLT_enum;//Э��
 S1_PRO_ENUM PRO_step;//������
 u8 card_recv[4][CAER_REC_NUM];//���ܵ��Ƶ����� �ĸ���λ
 u8 card_dice[4][25];//������4����λ��= ��λ+����+����+����  20���0xAA�������
 u8 card_num[4][1];//4����λ�Ƶ�����
 u8 card_num_cnt;//��λ���� һ����4
 u8 card_all_num;//�ĸ���λ�Ƶ�����
 
 u8 uasrt_dir[4][1];//����
 u8 diceFishedFlag;//��ɫ���� 
 u8 diceTimes;//��ɫ����
 u8 diceTimesBack;//��ɫ����
 u8 diceContolFlag;//������ɫ


 ////����A83T
 USART_ENUM send_usart;//���͵Ĵ���
 DIR_ENUM send_dir;//���͵ķ�λ 
 u8 send_data_len;//Ҫ���ͳ���
 u8 send_data[200];//Ҫ���͵�����

 HARDWARE_STRUCT st_hardWare;
 unsigned long timeoutCnt;//��ʱ
}S1_PRO_STRUCT;//������


#define CARD_DICE_REC_FLAG 0xaa
#define CARD_DICE_REC_DIR 20 //λ��



/*
��ɫ
1.�¶ȹ��߲���ɫ
2.���ʱ�䲻������ɫ
��һ�δ�ɫ
������ɫ
���ƴ�ɫ
�ڶ��δ�ɫ
if ��һ����������ɫ
������ɫ
if ��һ���ǿ��ƴ�ɫ
���ƴ�ɫ
*/

extern S1_PRO_STRUCT S1_pro;
u8 S1_TO_A83T(void);
u8 A83T_TO_S1(u8 usart,u8 *data,u8 len);
void S1_HARDWARE_Pro(void);
void S1_CTRL_PrO(void);
void S1_Init(void);

#endif










