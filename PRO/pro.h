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


//接受到牌的数量
#define CAER_REC_NUM 100
#define CAER_REC_FLAG 0xbb

#define CARD_DIRTION_NUM 4 //方位的个数 1东 2东南 3东南西 4东南西北


#define PCTL_PRO_ONLY (0)  //只是跑协议
#define REC_DATA_PRINTF (1)//接受数据是否打印
#define SEND_DATA_PRINTF (1) //发送数据是否打印
/*



CMD_CARD_NUMBER = 0,   // 牌数量
CMD_CARD_NUMBER_REQUEST = 1, //牌数量请求
CMD_CARD_SEND = 2,			 // 牌单个数据的发送
CMD_CARD_SEND_REQUEST = 3,	 //发牌请求
CMD_CARD_DATA = 4,			 //保存S1发过来的牌
CMD_CARD_DATA_REQUEST = 5,	//向S1请求一个方位的牌
CMD_DICE_RESULT = 6,		 // 骰盘结果
CMD_DICE_RESULT_REQUEST = 7, // 骰盘结果请求 暂时不用
CMD_SHUFFLE_CARD_END				= 8, //洗牌结束；(洗牌结束开始出打骰结果  打骰成功清除牌的数据)
CMD_PLAY_DICE				   = 9, // 传庄家方位 并返回会打骰结果 （如果第一次没有将打骰结果的给S1的情况才会使用 如同 骰盘结果请求）
CMD_CLEAR_PLAY_DICE 			= 10,//清除打骰
CMD_CAER_DATA_TO_S1 		   = 11,// 整个方位的发送给S1;
CMD_OPEN_CAMERA 			= 12, // 开灯
CMD_CLOSE_CAMERA			= 13, // 关灯

东南西北
//static u8 card_num[4] = {34,34,34,34};//4个方位牌的数量
static u8 card_num[4] = {0,0,0,0};//4个方位牌的数量
static u8 card_recv[4][100] = {0};
static u8 uart_to_addr[4] = {0,0,0,0};//4个方位对应哪个串口
//static u8 touzi_num = 0;//点数
static u8 touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数
//static u8 test_flag = 0;


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
data（ID VALUE）
//A83T通信协议：0xff 0xaa 方位 cmd len data [校验码低8位] [校验码高8位]
                  0    1   2    3    4   5      6              
*/

enum{
PTLC_FALSE=0,
PTLC_TRUE=!PTLC_FALSE,
};

typedef enum
{
  CMD_CARD_NUM =0,//            0=该方位的牌的数量；			A83T-->S1
  CMD_RESEND_CARD_NUM =1,//		1=请求重发牌的数量；			S1-->A83T
  CMD_SINGLE_CARD=2,//			2=发单个牌；					A83T-->S1
  CMD_RESEND_SINGLE_CARD=3,//	3=请求重新发单个牌；			S1-->A83T
  CMD_ALL_DIR_CARD=4,//         4=整个方位的牌的数据；			S1-->A83T（东）
  CMD_RESEND_ALL_DIR_CARD=5,//  5=请求重发整个方位的牌的数据；	A83T-->S1
  CMD_DICE_NUM=6,//				6=点数；						A83T-->S1
  CMD_RESEND_DICE_NUM=7,//		7=请求重发点数；				S1-->A83T
  CMD_SHUFFLE_CARD_END=8,//     8=洗牌结束:             S1-->A83T
  CMD_CLEAR_PLAY_DICE=10,////   10=清除打骰         S1-->A83T
}PCLT_ENUM;//协议枚举

/* //检测是否有结束信号
  1.请求牌的数量
  2.结束信号
  3.请求四个方位的牌
  4.发给东方位
  5.请求点数(遥控控制知道庄家)
  6.接受到点数后打色
*///contol


//方位0-4：东南西北S1
typedef enum{
	dong_DIR=0,
	nan_DIR=1,
	xi_DIR=2,
	bei_DIR=3,
	s_DIR=4,
}DIR_ENUM;


typedef enum
{
  PRO_CARD_NUM_TO_A83T=0,//请求牌的数量
  PRO_CARD_ALL_CARD_TO_A83T=1,//请求四个方位的牌
  PRO_CARD_SEND_TO_A83T_DONG=2,//发给东方位
  PRO_DICE_TO_A83T=3,//请求点数(遥控控制知道庄家)
  PRO_PLAY_DICE=4,//接受到点数后打色
  PRO_END=5,//打牌结束
}S1_PRO_ENUM;//处理步骤


typedef enum
{
USART1_TYPE=1,
USART2_TYPE=2,//dong
USART3_TYPE=3,//nan
USART4_TYPE=4,//xi
USART5_TYPE=5,//bei
}USART_ENUM;

//打色控制类型
typedef enum{
NORMAL_TYPE=0,//正常打色
CNTROL_TYPE=1,//控制打色
}DICE_ENUM;

//色子结构体
//typedef struct{
// DICE_ENUM dice1_type;//打色类型
//
//}DICE_STRUCT;

#define DICE_TIMER_TICK   (180*1000)//每次打色要等待3分钟
#define WASH_TIMER_TICK  1000 //1s
#define TEMP_TIMER_TICK  5000 //5s
#define CHARGE_TIMER_TICK 10000 //10s


//硬件结构体
typedef struct{
////////////////////硬件标志
//1.无线充电
//2.温度采集
//3.马达状态
//4.按键获取
//5.洗牌结束信号获取

 u8 chargeFlag;//充电标志
 u16 chargeFlagCnt;//充电无线计数
 u32 chargeCnt;//打色计数
 
 u8 tempFlag;//温度检测
 u16 tempValue;
 u16 tempCnt;//温度检测计数
 //洗牌等亮是表示正在洗牌，洗牌灯灭时表
 u8 washedFalg;//洗牌结束标志
 u8 washTimes;//
 u8 washTimesCnt;//
 u16 washCnt;//洗牌计数

 
 u8 keylastValue;//上次的按键值
 u8 keyGetValue;//按键获取
 u8 kwylastValueFlash;//
 u8 keyGetFlag;//
 
}HARDWARE_STRUCT;


typedef struct
{
 PCLT_ENUM PCLT_enum;//协议
 S1_PRO_ENUM PRO_step;//处理步骤
 u8 card_recv[4][CAER_REC_NUM];//接受到牌的数量 四个方位
 u8 card_dice[4][25];//点数（4个方位）= 方位+次数+个数+点数  20存放0xAA代表接受
 u8 card_num[4][1];//4个方位牌的数量
 u8 card_num_cnt;//方位个数 一般是4
 u8 card_all_num;//四个方位牌的总数
 
 u8 uasrt_dir[4][1];//串口
 u8 diceFishedFlag;//打色结束 
 u8 diceTimes;//打色次数
 u8 diceTimesBack;//打色次数
 u8 diceContolFlag;//正常打色


 ////发送A83T
 USART_ENUM send_usart;//发送的串口
 DIR_ENUM send_dir;//发送的方位 
 u8 send_data_len;//要发送长度
 u8 send_data[200];//要发送的数据

 HARDWARE_STRUCT st_hardWare;
 unsigned long timeoutCnt;//超时
}S1_PRO_STRUCT;//处理函数


#define CARD_DICE_REC_FLAG 0xaa
#define CARD_DICE_REC_DIR 20 //位置



/*
打色
1.温度过高不打色
2.充电时间不够不打色
第一次打色
正常打色
控制打色
第二次打色
if 第一次是正常打色
正常打色
if 第一次是控制打色
控制打色
*/

extern S1_PRO_STRUCT S1_pro;
u8 S1_TO_A83T(void);
u8 A83T_TO_S1(u8 usart,u8 *data,u8 len);
void S1_HARDWARE_Pro(void);
void S1_CTRL_PrO(void);
void S1_Init(void);

#endif










