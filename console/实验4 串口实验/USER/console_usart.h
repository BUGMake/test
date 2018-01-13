
#ifndef CONSOLE_USART_
#define CONSOLE_USART_
#include "queue.h"


//暂时只支持char
#define CONSOLE_PARM_LEN_MAX  15 //参数的长度  
#define CONSOLE_PARM_NUM_MAX  5 //参数的个数
#define CONSOLE_FUNCNAME_LEN_MAX  25 //函数名长度
#define CONSOLE_FRAME_DATA_LEN_MAX   100 //一帧数据的长度 命令名+参数
//是否支持时间  
#define CONSOLE_TIME_GET  0
//#define CONSOLE_TIME_GET  getSystickCount()

//////
#define CONSOLE_REC_BUF_MAX 128

///////
#define CONSOLE_PRINT printf
#define CONSOLE_PRINTK
#define CONSOLE_PRINTTIME

typedef enum{
CONSOLE_LS_CMD=0,//ls 列出命令表命令
CONSOLE_ADD_CMD=1,//添加命令 add 函数名=命令
CONSOLE_DEL_CMD=2,//删除命令 del 命令
CONSOLE_EXE_CMD=3,//执行命令
}CONSOLE_CMD_ENUM;

//串口命令解析返回值
typedef enum{
CONSOLE_REC_PRO_OK=0,//成功
CONSOLE_REC_PRO_ERROR=1,//失败
}CONSOLE_REC_TYPE_ENUM;
//命令类型
typedef enum{
CONSOLE_FUNC_CMD=0,//函数命令
CONSOLE_SYS_CMD=1,//系统命令
}CONSOLE_CMD_TYPE_ENUM;

//函数成员 
typedef struct{
  void *func;//函数指针
  const  char *name;//函数名(查找字符串执行相应得函数功能)
}CONSOLE_NAMETAB_STRUCT;

///系统命令
typedef enum {
CONSOLE_SYS_LS_CMD=0,
CONSOLE_SYS_LIST_CMD=1,
CONSOLE_SYS_RUNTIME_CMD=2,
}CONSOLE_SYS_CMD_ENUM;

//console 管理器
typedef struct{


unsigned char funcNum;//函数的总个数  初始化时计算个数
unsigned char funcId;//对应函数的id   id定会小于函数的总个数

unsigned char frameDataBuf[CONSOLE_FRAME_DATA_LEN_MAX];//一帧数据
unsigned char frameDataBuflen;//一帧数据的长度


unsigned char funcName[CONSOLE_FUNCNAME_LEN_MAX];//函数名字符串存放 
unsigned char funcNameLen;//长度

unsigned char funcParmn;//函数参数的数量 
unsigned char funcParmTbl[CONSOLE_PARM_NUM_MAX][CONSOLE_PARM_LEN_MAX];//保存函数的参数

unsigned char runTimeFlag;//时间开启标识
unsigned int  runTinme;//运行时间 基于系统的滴答时钟


CONSOLE_CMD_TYPE_ENUM cmdType;//命令类型 系统or函数命令
CONSOLE_SYS_CMD_ENUM sysCmd;  //系统命令

CONSOLE_NAMETAB_STRUCT *funs;//函数名指针

void (*funcEntry)(void);//终端程序入口 
CONSOLE_REC_TYPE_ENUM (*cmdRecPro)(unsigned char *,unsigned short);//命令解析
void (*funcExe)(void);//单个函数执行

}CONSOLE_USART_STRUCT;

extern void consoleInit(void);
extern void consoleProEntry(void);
extern unsigned char consoleRecBuf[CONSOLE_REC_BUF_MAX];
//队列管理
extern QueueStruct stconsoleQueue;
//定义变量
extern CONSOLE_USART_STRUCT stConsoleDev;//终端设备管理器


#endif /*CONSOLE_USART_*/

















