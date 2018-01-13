#include "face_protocol.h"
#include "UIapi.h"

#include "stdio.h"
//#include "xdmad.h"
//#include "include/rtc.h"




PTCL_SERVER_STRUCT gPtclPro;
extern FaceObject * pFaceObject;



#ifdef LMS_PROTOICOL

//协议  FaceObject
///////////////////罗曼斯协议配置
//start
#define LMS_PROTOCOLID_PACKED_START 0xAA75  //先高后低
#define LMS_PROTOCOLID_WAKEUP_N 0x05 //唤醒字节个数
#define LMS_PROTOCOLID_WAKEUP_VALUE 0x00
#define LMS_PROTOCOLID_WAKEUP_TIME 100 //唤醒等待时间片 1ms


////////////////////////////配置串口的信息配置

#define USART_PTCL USART1
#define USART_ID_PTCL ID_USART1
#define USART_BAU_PTCL 9600 
#define PTCL_TIMEOUT 100  //被动 超时时间片 发送数据无应答的时间超时  100*10ms
#define PTCL_DATATIMEOUT 3 //有数据时等待的时间 基于时间片 3*10ms 根据的长度改变
#define PTCL_RECDATALEN 50 //接收数据的长度 DMA 协议最大长度
static const Pin PTCL_Pin[]={PIN_USART1_TXD, PIN_USART1_RXD};
////
#define PCTL_GET_TICKTIME  xTaskGetTickCountFromISR() ///获取系统时钟

typedef enum {
LMSCMDFLAG=0x01,//命令
LMSACKFLAG=0x07,//应答
}PROTCOL_PACK_FLAG_ENUM;

//参数设置类型
typedef enum{
LMS_OPEN_DOOR_TYPE=0x01,//开门方式 1B
LMS_INFRARED_TYPE=0x02,//红外方式 1B
LMS_STOP_TIME_TYPE=0x03,//停顿时间 2B
LMS_REVERSAL_TIME_TYPE=0x04,//反转时间 2B
LMS_MOTOR_TORQUE_TYPE=0x05,//电机扭矩 1B
LMS_AUTO_LOCK_TYPE=0x06,//自动上锁 1B
LMS_DOORBELL_TYPE=0x07,//门铃 1B
LMS_VOICE_VOLUME_TYPE=0x08,//语音 1B
LMS_LANGUAGE_TYPE=0x09,//语言 1B
LMS_TRMPER_TYPE=0x0A,//防拆 1B
}LMS_PARAMETER_SET_TYPE_ENUM; 

//事件类型
typedef enum
{
LMS_DOOROPEN_REMIND_TYPE=0x01,	//开门提醒 -0x01
LMS_LOW_POWERALERT_TYPE=0x02,//低电量报警
LMS_ERROR_MORE_TYPE=0x04,//错误超限类型
}LMS_EVENT_TYPE_ENUM;

//菜单类型
typedef enum{
LMS_RFID_SET=0x01,//用户卡
LMS_REMOTE_CTL_SET=0x02,//遥控器
LMS_GATEWAY_SET=0x03,//网关
LMS_TRMPER_SET=0x04,//防拆设置
}LMS_ENUM_TYPE_ENUM;


//////ack
typedef enum
{
LMS_KEY_VALUE_WUP_TYPE=0x01,//按键唤醒
LMS_AUTO_LOCK_WUP_TYPE=0x02,//自动上锁
LMS_REMOTE_WUP_TYPE=0x03,//遥控值
LMS_RFID_WUP_TYPE=0x04,//非接触射频卡
LMS_TRMPER_WUP_TYPE=0x05,//防拆按钮
}LMS_WAKEUP_TYPE_ENUM;



//////////////////时间&日期 格式2017年01月01日 12:11:00  
typedef struct
{
	unsigned char Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char week;
}PCTL_DATE_STRUCT;
typedef struct
{

	 unsigned char Hour;
	 unsigned char Minutes;
	 unsigned char Seconds;
}PCTL_TIME_STRUCT;

typedef struct
{
	unsigned char Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Minutes;
	unsigned char Seconds;
	unsigned char week;
	//PCTL_DATE_STRUCT Data;
	//PCTL_TIME_STRUCT Time;
}PCTL_CALENDAR_STRUCT;


static void PCTL_HexToBcd(unsigned char *Hex, unsigned char *Bcd, unsigned char len)
{
	unsigned char i;
	for (i = 0; i<len; i++)
	{
		if (Hex[i] <= 99)
		{
			Bcd[i] = (Hex[i] % 100) / 10;
			Bcd[i] <<= 4;
			Bcd[i] |= Hex[i] % 10;
		}
		else
		{
			Bcd[i] = Hex[i];
		}
	}
}


static void PCTL_GetTimeData(unsigned char *buf,unsigned char len)
{
    unsigned short year;
	PCTL_CALENDAR_STRUCT tm;
	RTC_GetTime( RTC, &tm.Hour, &tm.Minutes, &tm.Seconds);
	RTC_GetDate( RTC, &year, &tm.Month, &tm.Day, &tm.week);
	year-=2000;
	tm.Year=year&0xFF;
	PCTL_HexToBcd((unsigned char*)&tm,buf,len);
}

static  void PCTL_MemCopy(unsigned char *dest, unsigned char *src, unsigned short size)
{
    while(size--) *dest++ = *src++;
}
static void PCTL_MemSet(unsigned char *dest, unsigned char value, unsigned short size)
{
	while(size--) *dest++ = value;
}

static unsigned char crcGetCrc(unsigned char *pData,unsigned short len)
{
   unsigned char i,CRC;
   CRC=pData[0];
   for(i=1;i<len;i++)
   {
     CRC^=pData[i];
   }   
   return CRC;
}

static void diaplayInfo(unsigned char *dat,unsigned char len)
{
   unsigned char i;
   for(i=0;i<len-1;i++)
   	printf("%02X-",dat[i]);
   printf("%02X\r\n",dat[i]);
}

extern  void PTCL_UsartSend(unsigned char *dat,unsigned char len);

/********************************************************
函数名:PTCL_CmdAnalysis
函数作用:对协议处理
输入参数: ProtocolDir: 1:PROTOCOL_TO_USART  2:USART_TO_PROTOCOL
          ptclCmd:应用层发过来的命令
          pBuf:数据指针
          len:数据长度

********************************************************/
unsigned char PTCL_CmdAnalysis(PROTOCOL_DIRECTIOPN_ENUM ProtocolDir,unsigned int ptclCmd,unsigned char *pBuf,unsigned short bufLen)
{
        unsigned short cmdLen=bufLen;
		unsigned char  *pCmdBuf=pBuf;
		unsigned char crc=0,i=0;

		unsigned int t1;//等待时间
        FACE_PROTOICOL_ACK_CODE_ENUM ack=0;
        unsigned short head=0;
		unsigned char len=0;
		unsigned char cmd=0;
		gPtclPro.txLen=0;
		
		//if(bufLen>256) return 0;
		switch(ProtocolDir)
		{
		   case PROTOCOL_TO_USART://
		   {


	            i=0; 
			    //1.唤醒字节
			    PCTL_MemSet(gPtclPro.txBuf,LMS_PROTOCOLID_WAKEUP_VALUE,LMS_PROTOCOLID_WAKEUP_N);
			    i+=LMS_PROTOCOLID_WAKEUP_N;
			    //2.包头
			    gPtclPro.txBuf[i++]=(LMS_PROTOCOLID_PACKED_START>>8)&0xFF;
			    gPtclPro.txBuf[i++]=(LMS_PROTOCOLID_PACKED_START)&0xFF;	
                //3.命令字
				gPtclPro.txBuf[i++]=0x00;
				//gPtclPro.txBuf[i++]=(unsigned char)((ptclCmd)&0xFF);
				//4.长度
				gPtclPro.txBuf[i++]=0x00;
                //5.包的标识
                gPtclPro.txBuf[i++]=LMSCMDFLAG; 				
		   		switch(ptclCmd)//应用层的命令
				{
                   case FACE_FRONT_WAKEUP_CMD://前端唤醒
		                //时间 6Byte
		                PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
						i+=6;
		                gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_FRONT_WAKEUP_CMD&0xFF;
		                gPtclPro.txBuf[i++]=pFaceObject->wakeUpFlag;//唤醒标志
		                				
				   break;
				   case FACE_READ_ELECTRIC_CMD://读取电量
				   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_READ_ELECTRIC_CMD&0xFF;
				   break;
				   case FACE_PARAMETER_SET_CMD://参数设置
				       gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_PARAMETER_SET_CMD&0xFF;				   
					   gPtclPro.txBuf[i++]=pFaceObject->parameterSetType;//设置类型
					   switch(gPtclPro.txBuf[i-1])
					   {
                         case LMS_OPEN_DOOR_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->doorTypeFlag;//开门方式
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_doorTypeFlag;//开门方式
						 	break;
						 case LMS_INFRARED_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->infraedType;//红外方式  
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_Infrared;
						 	break;
						 case LMS_STOP_TIME_TYPE:
						 	//////////////////////先高后低
						 //	gPtclPro.txBuf[i++]=(pFaceObject->waitTime>>8)&0xFF;//停顿时间1-9s 2byte 
						 	//gPtclPro.txBuf[i++]=pFaceObject->waitTime&0xFF;//停顿时间1-9s 2byte 
						 	gPtclPro.txBuf[i++]=(pFaceObject->usermanage->Maintain_PauseTime>>8)&0xFF;//停顿时间1-9s 2byte 
							gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_PauseTime&0xFF;//停顿时间1-9s 2byte 
						 	break;
						 case LMS_REVERSAL_TIME_TYPE:
						 	//////////////////////先高后低
						 //	gPtclPro.txBuf[i++]=(pFaceObject->reversalTime>>8)&0xFF;//反转时间 200-900ms 2byte
						 //	gPtclPro.txBuf[i++]=pFaceObject->reversalTime&0xFF;//反转时间 200-900ms 2byte

							gPtclPro.txBuf[i++]=(pFaceObject->usermanage->Maintain_reversalTime>>8)&0xFF;//反转时间 200-900ms 2byte
							gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_reversalTime&0xFF;//反转时间 200-900ms 2byte
						 	break;
						 case LMS_MOTOR_TORQUE_TYPE:
						 //	gPtclPro.txBuf[i++]=pFaceObject->motorTorqueType;//电机扭矩  
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_motorTorqueType;//电机扭矩  
						 	break;
						 case LMS_AUTO_LOCK_TYPE:
						 //	gPtclPro.txBuf[i++]=pFaceObject->autoLockType;//自动上锁 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->AutoLock;//自动上锁 
						 	break;
						 case LMS_DOORBELL_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->doorBellTpe;//门铃 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Voice_Bell_TypeAndValue;//门铃 
						 	break;
						 case LMS_VOICE_VOLUME_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->voiceVolumeType;//音量 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Voice_Speech_Value;//音量 
						 	break;
						 case LMS_LANGUAGE_TYPE:
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->language;//语言
						 	break;
						 case LMS_TRMPER_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->antiTamperFlag;//防拆 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_antiTamperFlag;//防拆 
						 	break;
                         default:   break;
					   }
					   

	               break; 
				   case FACE_OPEN_CLOSE_DOOR_CMD://开关门
				        gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_OPEN_CLOSE_DOOR_CMD&0xFF;
                        gPtclPro.txBuf[i++]=pFaceObject->openCloseFlag;//开关门标志
				        PCTL_MemCopy(&gPtclPro.txBuf[i],pFaceObject->openCloseReserve,7);//保留 7B
						i+=7;						

				   break;
				   case FACE_GET_FIRMWARE_INF_CMD://获取固件信息
					   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_GET_FIRMWARE_INF_CMD&0xFF;

					break;
				   case FACE_MEASUREMENT_MODE_CMD:////测试模式
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_MEASUREMENT_MODE_CMD&0xFF;
                       // gPtclPro.txBuf[i++]=pFaceObject->measurementState;//状态 
						gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_Cycl_Demonstration;//状态 

				   break;
				   case FACE_VOICE_MODE_CMD://语音模式
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_VOICE_MODE_CMD&0xFF;
				   break;
                   
				   case FACE_EVENT_INF_CMD://事件信息
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_EVENT_INF_CMD&0xFF;
				        gPtclPro.txBuf[i++]=pFaceObject->eventType;//事件类型
				        switch(gPtclPro.txBuf[i-1])
						{
							case LMS_DOOROPEN_REMIND_TYPE:
								gPtclPro.txBuf[i++]=pFaceObject->user1;//用户1
								gPtclPro.txBuf[i++]=pFaceObject->user2;//用户2
								//时间
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								gPtclPro.txBuf[i++]=pFaceObject->openDoorType;//开门方式 
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeUp;//上电池电量
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeDown;//下电池电量
								
								break;
							case LMS_LOW_POWERALERT_TYPE:
								//时间
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeUp;//上电池电量
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeDown;//下电池电量
								break;
							case LMS_ERROR_MORE_TYPE:
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								//时间
								gPtclPro.txBuf[i++]=pFaceObject->errorMoreType;//人脸-1 密码-2 用户卡-3
								break;
							default :  break;	
						}

				   break;
				   case FACE_MENU_OPERATION_CMD:////菜单操作
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_MENU_OPERATION_CMD&0xFF;
				        gPtclPro.txBuf[i++]=pFaceObject->menuDataType;//数据类型 
				        gPtclPro.txBuf[i++]=pFaceObject->menuOperate;//添加or关闭-0x01 删除or开启-0x02  清空-0x03
				        
				   break;
				   case FACE_RECOVER_MODE_CMD://恢复出厂设置
						gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_RECOVER_MODE_CMD&0xFF;
				   break;

				   case FACE_SLEEP_MODE_CMD://休眠
					   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_SLEEP_MODE_CMD&0xFF;

				   break;

                   case FACE_UPLOAD_EVENT_INF_CMD:////上传事件信息 
                        
                        gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_UPLOAD_EVENT_INF_CMD&0xFF;//命令
						gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]=LMSACKFLAG;//更改包标识符
						switch(pBuf[0])
						{	
						   case FACE_OPERATE_FAILED_ACK://操作失败
						        gPtclPro.txBuf[i++]=LMS_OPERATE_FAILED_ACK&0xFF;
 								break;
						   case FACE_OPERATE_SUCCEED_ACK://操作成功
						        gPtclPro.txBuf[i++]=LMS_OPERATE_SUCCEED_ACK&0xFF;
						        break;
						   case FACE_OPERATE_CRC_ERROR_ACK://校验和错误
						        gPtclPro.txBuf[i++]=LMS_OPERATE_CRC_ERROR_ACK&0xFF;
						   break;
						}
						
				   break;
				   
                   default:    
				   break;
				}

	         	    gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+2]=i+1-3-LMS_PROTOCOLID_WAKEUP_N;//重新计算长度
	                crc=crcGetCrc(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+2],i-LMS_PROTOCOLID_WAKEUP_N-2);
					gPtclPro.txBuf[i++]=crc;//校验
                    gPtclPro.Cmd=gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3];//保存命令
				//////////////传输
				    gPtclPro.txLen=i;//要发送的数据个数
				    if(gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]==LMSCMDFLAG)//主动发送数据需要唤醒字节
					{
				    //唤醒字节
				    PTCL_UsartSend(gPtclPro.txBuf,LMS_PROTOCOLID_WAKEUP_N);//发送唤醒字节
					t1=PCTL_GET_TICKTIME;
					while(t1+LMS_PROTOCOLID_WAKEUP_TIME>PCTL_GET_TICKTIME);//等待唤醒时间
					
					//真正的数据
				    PTCL_UsartSend(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N],gPtclPro.txLen-LMS_PROTOCOLID_WAKEUP_N);
					gPtclPro.recType=PTCL_INITISTIVE_REC;//主动模式
					}
					else if(gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]==LMSACKFLAG)//发送应答数据
					{
					
					//真正的数据
				    PTCL_UsartSend(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N],gPtclPro.txLen-LMS_PROTOCOLID_WAKEUP_N);
					gPtclPro.recType=PTCL_PASSIVE_REC;//被动模式	
					}
					
		   break;
		   }

		   
           case USART_TO_PROTOCOL://接收数据
		   {
                
				//1.唤醒字节
				//while(pCmdBuf[i++]==0x00)//唤醒字节
                //if(i>256) 
                //{
                // return FACE_OTHER_ERROR_ACK;
				//}
		        //2.包头
		        head=((unsigned short)pCmdBuf[i++])<<8;
				head|=pCmdBuf[i++];
		        if(head!=LMS_PROTOCOLID_PACKED_START)
				return FACE_OTHER_ERROR_ACK;//head error	
				//3.长度
				len=pCmdBuf[i++];
				//判断校验是否正确
                crc=crcGetCrc(&pCmdBuf[i-1],(unsigned short)len);
				if(crc!=pCmdBuf[i-1+len])
				return FACE_OPERATE_CRC_ERROR_ACK;//crc error	
		        //4.命令
		        cmd=pCmdBuf[i++];
				//if(cmd!=gPtclPro.Cmd)
				//return FACE_OTHER_ERROR_ACK;//命令错误
		        //5.包标识
		        //if(pCmdBuf[i++]!=LMSACKFLAG)					
				//return FACE_OTHER_ERROR_ACK;//packflag error
				if(pCmdBuf[i]==LMSACKFLAG&&gPtclPro.recType==PTCL_INITISTIVE_REC) //应答数据
				{
				i++;
				///命令
			        if(cmd!=gPtclPro.Cmd)
				    return FACE_OTHER_ERROR_ACK;//命令错误
				//6.确认码
					switch(pCmdBuf[i++])
					{
	                	case LMS_OPERATE_FAILED_ACK://操作失败
	                	ack=FACE_OPERATE_FAILED_ACK;
	                	break;
						case LMS_OPERATE_SUCCEED_ACK://操作成功
						
							switch(cmd)
						    {
						   		case LMS_FRONT_WAKEUP_CMD://前端唤醒
						   		pFaceObject->wakeUpType=pCmdBuf[i++];//按键唤醒-0x01  自动上锁-0x02 遥控唤醒-0x03  非接触射频卡-0x04 防拆-0x05
								switch(pFaceObject->wakeUpType)
								{
	                             	case LMS_KEY_VALUE_WUP_TYPE:
									   
	                                   pFaceObject->wUpTypeKeyValue=pCmdBuf[i++];//开门-0x42 锁门-0x43 门铃-0x45
										
										break;
									case LMS_AUTO_LOCK_WUP_TYPE:
										pFaceObject->wUpTypeAutoCloseDoor=pCmdBuf[i++];//自动上锁
										
										break;
									case LMS_REMOTE_WUP_TYPE:
										pFaceObject->wUpTypeRemoteControl=pCmdBuf[i++];//遥控值
										break;
									case LMS_RFID_WUP_TYPE:
										//高低位
										pFaceObject->wUpTypeRFID[0]=pCmdBuf[i++];//RFID 4Byte 卡号 高字节在前 如174C526D
										pFaceObject->wUpTypeRFID[1]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[2]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[3]=pCmdBuf[i++];
										break;
									case LMS_TRMPER_WUP_TYPE:
										pFaceObject->wUpTypeAntiTamper=pCmdBuf[i++];//防拆//1有效 0无效
										break;
									default:
										ack=FACE_OTHER_ERROR_ACK;
										break;	
	                            
								}							
	                           
	                          //  pFaceObject->expandUserN=pCmdBuf[i++];////扩展用户数
	                            pFaceObject->usermanage->Remote_And_ExternalModule=pCmdBuf[i++];////扩展用户数
								//参数配置
	                          //  pFaceObject->doorTypeFlag=pCmdBuf[i++];//开门方式  0-左开门 1-右开门
	                            pFaceObject->usermanage->Maintain_doorTypeFlag=pCmdBuf[i++];//开门方式  0-左开门 1-右开门
	                           // pFaceObject->infraedType=pCmdBuf[i++];//红外方式 
	                            pFaceObject->usermanage->Maintain_Infrared=pCmdBuf[i++];//红外方式 
	                          //  pFaceObject->waitTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//停顿时间
	                            pFaceObject->usermanage->Maintain_PauseTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//停顿时间
	                           // pFaceObject->reversalTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//反转时间 
	                            pFaceObject->usermanage->Maintain_reversalTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//反转时间 
							//	pFaceObject->motorTorqueType=pCmdBuf[i++];//电机扭矩 
								pFaceObject->usermanage->Maintain_motorTorqueType=pCmdBuf[i++];//电机扭矩 
							//	pFaceObject->autoLockType=pCmdBuf[i++];//自动上锁 
								pFaceObject->usermanage->AutoLock=pCmdBuf[i++];//自动上锁
								//pFaceObject->doorBellTpe=pCmdBuf[i++];//门铃 
								pFaceObject->usermanage->Voice_Bell_TypeAndValue=pCmdBuf[i++];//门铃 
								//pFaceObject->voiceVolumeType=pCmdBuf[i++];//音量 
								pFaceObject->usermanage->Voice_Speech_Value=pCmdBuf[i++];//音量 
								pFaceObject->usermanage->language=pCmdBuf[i++];//语言							
								//pFaceObject->antiTamperFlag=pCmdBuf[i++];//防拆 
								pFaceObject->usermanage->Maintain_antiTamperFlag=pCmdBuf[i++];//防拆 
	                            //////保留
	                            pFaceObject->batteryPos=pCmdBuf[i++];////电池位置
	                            pFaceObject->powerVolumeUp=pCmdBuf[i++];//上电池电量
								pFaceObject->powerVolumeDown=pCmdBuf[i++];//下电池电量
								break;
								case LMS_READ_ELECTRIC_CMD://读取电量

								
					        	break;
								case LMS_PARAMETER_SET_CMD://参数设置
								if(pFaceObject->parameterSetType!=pCmdBuf[i++])//参数设置类型
								return FACE_OTHER_ERROR_ACK;
								
								break;
								case LMS_OPEN_CLOSE_DOOR_CMD://开关门
	                            if(pFaceObject->openCloseFlag!=pCmdBuf[i++])//开关门标志 开门-0x4F 关门-0x43
								return FACE_OTHER_ERROR_ACK;
								break;

								case LMS_GET_FIRMWARE_INF_CMD://获取固件信息
								//版本 7B
								PCTL_MemCopy(pFaceObject->vertionFirware,&pCmdBuf[i],7);i+=7;
								//系列号 7B
								PCTL_MemCopy(pFaceObject->serialNumber,&pCmdBuf[i],7);i+=7;
								break;
								case LMS_MEASUREMENT_MODE_CMD://测试模式
								
								break;
								case LMS_VOICE_MODE_CMD://语音模式
								break;
								case LMS_EVENT_INF_CMD://事件信息
								break;
								case LMS_MENU_OPERATION_CMD://菜单操作
								
								if(pFaceObject->menuDataType!=pCmdBuf[i++])
								return  FACE_OTHER_ERROR_ACK;
	 							switch(pFaceObject->menuDataType)
								{
	 								case LMS_RFID_SET://添加的用户id
										pFaceObject->wUpTypeRFID[0]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[1]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[2]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[3]=pCmdBuf[i++];
										break;
	                                case LMS_REMOTE_CTL_SET:
										
										break;
									case LMS_GATEWAY_SET:
										
									break;
									default:
										ack=FACE_OTHER_ERROR_ACK;
									break;
								}
								
								break;
								case LMS_RECOVER_MODE_CMD://恢复出厂设置
								
								break;
								case LMS_SLEEP_MODE_CMD://休眠
								
								break;
						    }
	 
	                    ack=FACE_OPERATE_SUCCEED_ACK;
						break;
						case LMS_OPERATE_CRC_ERROR_ACK://校验和错误
						ack=FACE_OPERATE_CRC_ERROR_ACK;
						break;
						
						case LMS_THEPOS_IS_EXPAND_USER_ACK://指定位置已经有扩展用户
		                ack=FACE_THEPOS_IS_EXPAND_USER_ACK;
						break;
						
						case LMS_EXPAND_USER_EXIST_AXK://扩展用户已经存在
			            ack=FACE_EXPAND_USER_EXIST_AXK;
						break;
						
						case LMS_EXPAND_USER_NOSPACE_ACK://扩展用户存储满
			            ack=FACE_EXPAND_USER_NOSPACE_ACK;
						break;
						
						case LMS_OPERATE_TIMEOUT_ACK://超时退出
			            ack=FACE_OPERATE_TIMEOUT_ACK;
						break;
					
						default: 
							ack=FACE_OTHER_ERROR_ACK;
							break;   
					}
				}
				else if(pCmdBuf[i]==LMSCMDFLAG&&gPtclPro.recType==PTCL_PASSIVE_REC)//命令数据
				{
				  i++;
				  switch(cmd)
				  {
						case LMS_UPLOAD_EVENT_INF_CMD://上传事件信息
						pFaceObject->wakeUpType=pCmdBuf[i++];//按键唤醒-0x01  自动上锁-0x02 遥控唤醒-0x03  非接触射频卡-0x04 防拆-0x05
						switch(pFaceObject->wakeUpType)
						{
                         	case LMS_KEY_VALUE_WUP_TYPE:
							   
                               pFaceObject->wUpTypeKeyValue=pCmdBuf[i++];//开门-0x42 锁门-0x43 门铃-0x45
								
								break;
							case LMS_AUTO_LOCK_WUP_TYPE:
								pFaceObject->wUpTypeAutoCloseDoor=pCmdBuf[i++];//自动上锁
								
								break;
							case LMS_REMOTE_WUP_TYPE:
								pFaceObject->wUpTypeRemoteControl=pCmdBuf[i++];//遥控值
								break;
							case LMS_RFID_WUP_TYPE:
								//高低位
								pFaceObject->wUpTypeRFID[0]=pCmdBuf[i++];//RFID 4Byte 卡号 高字节在前 如174C526D
								pFaceObject->wUpTypeRFID[1]=pCmdBuf[i++];
								pFaceObject->wUpTypeRFID[2]=pCmdBuf[i++];
								pFaceObject->wUpTypeRFID[3]=pCmdBuf[i++];
								break;
							case LMS_TRMPER_WUP_TYPE:
								pFaceObject->wUpTypeAntiTamper=pCmdBuf[i++];//防拆//1有效 0无效
								break;
							default: 
							ack=FACE_OTHER_ERROR_ACK;
							break;	
                        
						}
						//保留
				        pFaceObject->batteryPos=pCmdBuf[i++];////电池位置
                        pFaceObject->powerVolumeUp=pCmdBuf[i++];//上电池电量
						pFaceObject->powerVolumeDown=pCmdBuf[i++];//下电池电量
						ack=FACE_UNLOAD_EVENT_INF_ACK;	
						break;
					  default:
					  	ack=FACE_OTHER_ERROR_ACK;
					  	break;
				  }
				}
				else//包标识错误
				{
                  return FACE_OTHER_ERROR_ACK;//packflag error
				}
				return ack;//
				break;
		   }
		   default:

		   break;
		}
        return 1;
}






/*
函数:配置DMA接收数据 
协议在接收数据时启用DMA方式
*/

static void PTCL_UsartDmaConfigure(void)
{
    unsigned int  status;
    sXdmadCfg xdmadCfg;
	XDMAD_Initialize(&gPtclPro.dmad,0);
	gPtclPro.dmaChannel=XDMAD_AllocateChannel( &gPtclPro.dmad, USART_ID_PTCL, XDMAD_TRANSFER_MEMORY);//
	XDMAD_PrepareChannel( &gPtclPro.dmad,gPtclPro.dmaChannel);


		xdmadCfg.mbr_ubc = PTCL_RECDATALEN;  //定义长度
		xdmadCfg.mbr_sa = (uint32_t)&USART_PTCL->US_RHR; 
		xdmadCfg.mbr_da = (uint32_t) gPtclPro.rxBuf;
		xdmadCfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
						   XDMAC_CC_MBSIZE_SINGLE |
						   XDMAC_CC_DSYNC_PER2MEM |
						   XDMAC_CC_CSIZE_CHK_1 |
						   XDMAC_CC_DWIDTH_BYTE |
						   XDMAC_CC_SIF_AHB_IF1 |
						   XDMAC_CC_DIF_AHB_IF0 |					 
						   XDMAC_CC_SAM_FIXED_AM |
						   XDMAC_CC_DAM_INCREMENTED_AM |
						   XDMAC_CC_PERID(XDMAIF_Get_ChannelNumber( 0, USART_ID_PTCL, XDMAD_TRANSFER_RX ));
	  
		xdmadCfg.mbr_bc =  0;
		xdmadCfg.mbr_ds =  0;
		xdmadCfg.mbr_sus = 0;
		xdmadCfg.mbr_dus = 0; 
		XDMAD_ConfigureTransfer( &gPtclPro.dmad, gPtclPro.dmaChannel, &xdmadCfg, 0, 0);
		XDMAD_StartTransfer( &gPtclPro.dmad, gPtclPro.dmaChannel);
	
}


/*
函数:串口初始化

*/

void PTCL_UsartInit(void)
{
   uint32_t mode =US_MR_USART_MODE_NORMAL | US_MR_USCLKS_MCK
                    | US_MR_CHMODE_NORMAL | US_MR_CLKO
                    | US_MR_CHRL_8_BIT | US_MR_NBSTOP_1_BIT| US_MR_PAR_NO;

    PIO_Configure(PTCL_Pin, PIO_LISTSIZE(PTCL_Pin));
       /* Enable the peripheral clock in the PMC */
    PMC_EnablePeripheral( USART_ID_PTCL ) ;
    /* Configure the USART in the desired mode @USART_SPI_CLK bauds*/	
    USART_Configure( USART_PTCL, mode, USART_BAU_PTCL, BOARD_MCK/2 ) ;	
    /* Enable receiver & transmitter */    
    USART_SetReceiverEnabled( USART_PTCL, 1 ) ;
    USART_SetTransmitterEnabled( USART_PTCL, 1 ) ; 
	PTCL_UsartDmaConfigure();
}	

/*
函数:向串口发数据
参数:
dat:数据 len:长度
*/
static void PTCL_UsartSend(unsigned char *dat,unsigned char len)
{
      unsigned char i;
      if(len<=0) return ;
      for(i=0;i<len;i++)
      USART_PutChar(USART_PTCL,dat[i]);
	  //gPtclPro.recType=PTCL_PASSIVE_REC;
}	

/*************************************
函数:PTCL_CMDSEND 发送命令函数
参数:
ptclCmd:对应的命令  
FACE_PROTOCOL_ENUM
value:要应答时调用的应答码 
**************************************/
void PTCL_CmdSend(unsigned int ptclCmd,unsigned char value)
{
    unsigned char ret=0;
    ret=gPtclPro.pFramCb(PROTOCOL_TO_USART,ptclCmd,&value,0);
	//if(ret)
	//gPtclPro.step=PTCL_STEP2;
	//return ret;
}

/*
函数:采用DMA接收数据的方式处理
返回值: FACE_PROTOICOL_ACK_CODE_ENUM
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
FACE_UNLOAD_EVENT_INF_ACK=0x50,//上传事件信息

*/
unsigned char PTCL_UsartProDma(void)
{
   unsigned char ret;
   unsigned int t1;
   if(gPtclPro.recType==PTCL_INITISTIVE_REC)
   {

     t1=PCTL_GET_TICKTIME;
      while(1)
	{
		  
		  if(t1+10<PCTL_GET_TICKTIME)//10ms时间片等待时间
		  {
		      t1=PCTL_GET_TICKTIME;
			  gPtclPro.framTimeout++;
			  if(gPtclPro.framTimeout>PTCL_TIMEOUT)//发送数据接收超时
			  {
			     ret=FACE_TIMEOUT_NOACK;
			     gPtclPro.framTimeout=0;
				 gPtclPro.recType=PTCL_PASSIVE_REC;
				 PTCL_UsartDmaConfigure();//重新配置接收DMA
				 //gPtclPro.rxLen=0;
				 //PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
				 return ret;
			  }
		  }
		  if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//有数据
	      {
	        t1=PCTL_GET_TICKTIME;
	        while(t1+10*PTCL_DATATIMEOUT>PCTL_GET_TICKTIME);	
			//gPtclPro.timeoutCnt++;
			gPtclPro.framTimeout=0;
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//刷新FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//处理
			//gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//重新配置接收DMA
			gPtclPro.recType=PTCL_PASSIVE_REC;
			return ret;	   
	      }
	 }
	}
   else
   {
        
		if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//有数据
		{
		    t1=PCTL_GET_TICKTIME;
	        while(t1+10*PTCL_DATATIMEOUT>PCTL_GET_TICKTIME);	
			//gPtclPro.timeoutCnt++;
			gPtclPro.framTimeout=0;//
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//刷新FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//处理
			//gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//重新配置接收DMA
			gPtclPro.recType=PTCL_PASSIVE_REC;
			return ret;
		}
		
   }
   return  FACE_WAIT_PRO;//等待处理
 /*  
   if(gPtclPro.recType==PTCL_PASSIVE_REC)//被动
   {
      gPtclPro.framTimeout++;
	  if(gPtclPro.framTimeout>PTCL_TIMEOUT)//发送数据接收超时
	  {
	     ret=FACE_TIMEOUT_NOACK;
	     gPtclPro.framTimeout=0;
		 gPtclPro.recType=PTCL_INITISTIVE_REC;
		 return ret;
	  } 
	}
   
   if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//有数据
   {
		gPtclPro.timeoutCnt++;
		gPtclPro.framTimeout=0;//
		if(gPtclPro.timeoutCnt>=PTCL_DATATIMEOUT)//等待一个时间片
		{
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//刷新FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//处理
			gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//重新配置接收DMA
			gPtclPro.recType=PTCL_INITISTIVE_REC;
			return ret;
		}
   
   }
   return  FACE_WAIT_PRO;//等待处理
   */

}	

/*
函数:对协议结构体初始化
调用协议时必须先初始化
*/
void PTCL_StructInit(void)
{
	//gPtclPro.step=PTCL_STEP1;
    gPtclPro.framTimeout=0;
	gPtclPro.rxLen=0;
	PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
	gPtclPro.txLen=0;
	PCTL_MemSet(gPtclPro.txBuf, 0, 256);
	gPtclPro.rxFlag=ptcl_false;	
	gPtclPro.pFramCb=PTCL_CmdAnalysis;
	gPtclPro.timeoutCnt=0;//时间计时0
	gPtclPro.recType=PTCL_PASSIVE_REC;//被动形式
}






///////////////////////////////////////////////////////////////
/*
函数:轮询接收数据
有超时处理
*/
//void PTCL_UsartRec(void)
//{
//     unsigned char value;
//     if(USART_IsRxReady(USART_PTCL)&&gPtclPro.step==PTCL_STEP2)//是否有数据
//	 {
//        value=USART_GetChar(USART_PTCL);
//		if(value==0x00&&gPtclPro.rxBuf[0]!=0xAA)
//        return ;
//		gPtclPro.rxBuf[gPtclPro.rxLen++]=value;
//	 }	
//	 if(PCTL_GET_TICKTIME>gPtclPro.framTimeout+PTCL_TIMEOUT&&gPtclPro.step==PTCL_STEP2)//判断超时时间
//	 {
//	    gPtclPro.step=PTCL_STEP1;
//        gPtclPro.rxFlag=ptcl_ture;//时间到
//	 }
//}
/*
数据接收处理函数
返回值: FACE_PROTOICOL_ACK_CODE_ENUM
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
*/
//unsigned char PTCL_UsartPro(void)
//{
//   unsigned char ret;
//   //发送协议时等待处理
//   if(gPtclPro.rxFlag==ptcl_ture)
//   {
//     gPtclPro.rxFlag=ptcl_false;
//	 if(gPtclPro.rxLen<=0)//没有接收到数据超时
//	 {
//       ret=FACE_TIMEOUT_NOACK;
//	 }
//	 else //接收到数据进行处理
//	 {
//      ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,gPtclPro.rxLen);//处理
//      gPtclPro.framTimeout=0;
//	  gPtclPro.rxLen=0;
//	  PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
//	 }	
//	 return ret;
//  }  
//  return FACE_WAIT_PRO;//等待处理
//}	
////////////////////////////////////////////////////////////////////////////////



#endif





