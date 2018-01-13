
#ifndef FACE_PROTOCOL
#define FACE_PROTOCOL
#include "board.h"


/////////////////////协议方向
typedef enum{
PROTOCOL_TO_USART=1,
USART_TO_PROTOCOL=2,
//PROTOCOL_TO_IIC=3,
}PROTOCOL_DIRECTIOPN_ENUM;


/////////////////////命令协议
typedef enum{
FACE_FRONT_WAKEUP_CMD=0x00000001,//前端唤醒
FACE_READ_ELECTRIC_CMD=0x00000002,//读取电量
FACE_PARAMETER_SET_CMD=0x00000003,//参数设置
FACE_OPEN_CLOSE_DOOR_CMD=0x00000004,//开关门
FACE_GET_FIRMWARE_INF_CMD=0x00000005,//获取固件信息
FACE_MEASUREMENT_MODE_CMD=0x00000006,//测试模式
FACE_VOICE_MODE_CMD=0x00000007,//语音模式
FACE_EVENT_INF_CMD=0x00000008,//事件信息
FACE_MENU_OPERATION_CMD=0x00000009,//菜单操作
FACE_RECOVER_MODE_CMD=0x0000000A,//恢复出厂设置
FACE_SLEEP_MODE_CMD=0x0000000B,//休眠

FACE_UPLOAD_EVENT_INF_CMD=0x0000000C,////上传事件信息

FACE_ALL_CMD=0xFFFFFFFF,//最大
}FACE_PROTOCOL_ENUM;


////////////////////应答协议
typedef enum{	
    //////////////////接收应答数据帧
	FACE_OPERATE_FAILED_ACK=0x00,//操作失败
	FACE_OPERATE_SUCCEED_ACK=0x01,//操作成功	
	FACE_OPERATE_CRC_ERROR_ACK=0x02,//校验和错误
	FACE_THEPOS_IS_EXPAND_USER_ACK=0x03,//指定位置已经有扩展用户
	FACE_EXPAND_USER_EXIST_AXK=0x04,//扩展用户已经存在
	FACE_EXPAND_USER_NOSPACE_ACK=0x05,//扩展用户存储满
	FACE_OPERATE_TIMEOUT_ACK=0x06,//超时退出
	FACE_OTHER_ERROR_ACK=0xFD,//其他错误(数据解析错误)	
	FACE_TIMEOUT_NOACK=0xFE,//没反应超时
	FACE_WAIT_PRO=0xFF,//等待处理

	////////////////接收命令数据帧标志
    FACE_UNLOAD_EVENT_INF_ACK=0x50,//上传事件信息
	
	/////////////////
}FACE_PROTOICOL_ACK_CODE_ENUM;



///////////////////
typedef enum{
PTCL_STEP1=0,
PTCL_STEP2=1,
PTCL_STEP3=2,
}PTCL_SERVER_STEP;
//协议接收方式 主动和被动
typedef enum{
PTCL_INITISTIVE_REC=0,//主动  人脸端先发送数据，门端应答数据
PTCL_PASSIVE_REC=1,//被动 门端先发送数据，人脸端应答
}PTCL_REC_TYPE_ENYM;


//接收数据的标识定义
#define ptcl_ture 1
#define ptcl_false 0

typedef unsigned char (*FramDataPro)(PROTOCOL_DIRECTIOPN_ENUM ProtocolDir,unsigned int ptclCmd,unsigned char *pBuf,unsigned short bufLen);
/////////////
typedef struct
{
	PTCL_SERVER_STEP step;		        //当前协议所处状态 应用轮询接收数据
	unsigned int framTimeout;			//帧超过时时间 发送数据后等待接收数据的超时时间
	FramDataPro pFramCb;                //帧数据回调处理  
	unsigned char rxBuf[256];           //接收数据缓存和长度
	unsigned char rxLen;
	unsigned char rxFlag;               //接收数据标识
	unsigned char txBuf[256];           //发送数据缓存和长度
	unsigned char txLen;	
	unsigned char Cmd;                  //保存命令


	/////////////DMA driver 
	sXdmad dmad;                        //
	unsigned int dmaChannel;            //DMA driver instance
	unsigned short timeoutCnt;          //检测到数据后的等待时间
	PTCL_REC_TYPE_ENYM recType;        //数据接收方式 主动(随时可以接收到数据)和被动(一应一答)
	
	
}PTCL_SERVER_STRUCT;





#define LMS_PROTOICOL






#ifdef LMS_PROTOICOL//罗曼斯

//发送命令返回值
enum{

LMS_CMDANALYSIS_ERROR=0,
LMS_CMDANALYSIS_OK=1,
};



typedef enum{
LMS_FRONT_WAKEUP_CMD=0xA0,//前端唤醒
LMS_READ_ELECTRIC_CMD=0xA1,//读取电量
LMS_PARAMETER_SET_CMD=0xA2,//参数设置
LMS_OPEN_CLOSE_DOOR_CMD=0xA3,//开关门
LMS_GET_FIRMWARE_INF_CMD=0xA4,//获取固件信息
LMS_MEASUREMENT_MODE_CMD=0xA5,//测试模式
LMS_VOICE_MODE_CMD=0xA6,//语音模式
LMS_EVENT_INF_CMD=0xA7,//事件信息
LMS_MENU_OPERATION_CMD=0xA8,//菜单操作
LMS_RECOVER_MODE_CMD=0xA9,//恢复出厂设置
LMS_SLEEP_MODE_CMD=0xAF,//休眠

LMS_UPLOAD_EVENT_INF_CMD=0xB0,//上传事件信息
}LMS_PROTOICOL_CMD_ENUM;



typedef enum{
LMS_OPERATE_FAILED_ACK=0x00,//操作失败
LMS_OPERATE_SUCCEED_ACK=0x01,//操作成功
LMS_OPERATE_CRC_ERROR_ACK=0xe0,//校验和错误
LMS_THEPOS_IS_EXPAND_USER_ACK=0xe1,//指定位置已经有扩展用户
LMS_EXPAND_USER_EXIST_AXK=0xe2,//扩展用户已经存在
LMS_EXPAND_USER_NOSPACE_ACK=0xe3,//扩展用户存储满
LMS_OPERATE_TIMEOUT_ACK=0xe4,//超时退出
}LMS_PROTOICOL_ACK_CODE_ENUM;
//命令包
typedef struct
{
   unsigned short cmd_pack_wakeup_len;//唤醒字节长度
   unsigned char*cmd_pack_wakeup_data;
   unsigned char cmd_pack_start[2];//包头 0xAA75
   unsigned char cmd_pack_len;
   LMS_PROTOICOL_CMD_ENUM cmd_pack_enum;
   unsigned char cmd_pack_sign;//标识 0x01
   unsigned short cmd_pack_data_len;
   unsigned char *cmd_pack_data;
   unsigned char cmd_pack_crc_sum;
}FACE_PROTOCOL_PACK_CMD_STRUCT;
//应答包格式
typedef struct
{
   unsigned short ack_pack_wakeup_len;//唤醒字节长度
   unsigned char*ack_pack_wakeup_data;
   unsigned char ack_pack_start[2];//包头 0xAA75
   unsigned char ack_pack_len;
   LMS_PROTOICOL_ACK_CODE_ENUM ack_pack_enum;
   unsigned char ack_pack_sign;//标识 0x01
   unsigned short ack_pack_data_len;
   unsigned char *ack_pack_data;
   unsigned char ack_pack_crc_sum;
}FACE_PROTOCOL_PACK_ACK_STRUCT;



extern void PTCL_UsartInit(void);//串口初始化函数
extern void PTCL_StructInit(void);//初始化函数

extern void PTCL_CmdSend(unsigned int ptclCmd,unsigned char value);//命令函数
extern unsigned char PTCL_UsartProDma(void);//接收数据函数

//extern void PTCL_UsartRec(void);//轮询接收串口数据函数
//extern unsigned char PTCL_UsartPro(void);//对数据处理函数












#endif













#endif /* FACE_PROTOCOL */















