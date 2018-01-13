#include "face_protocol.h"
#include "UIapi.h"

#include "stdio.h"
//#include "xdmad.h"
//#include "include/rtc.h"




PTCL_SERVER_STRUCT gPtclPro;
extern FaceObject * pFaceObject;



#ifdef LMS_PROTOICOL

//Э��  FaceObject
///////////////////����˹Э������
//start
#define LMS_PROTOCOLID_PACKED_START 0xAA75  //�ȸߺ��
#define LMS_PROTOCOLID_WAKEUP_N 0x05 //�����ֽڸ���
#define LMS_PROTOCOLID_WAKEUP_VALUE 0x00
#define LMS_PROTOCOLID_WAKEUP_TIME 100 //���ѵȴ�ʱ��Ƭ 1ms


////////////////////////////���ô��ڵ���Ϣ����

#define USART_PTCL USART1
#define USART_ID_PTCL ID_USART1
#define USART_BAU_PTCL 9600 
#define PTCL_TIMEOUT 100  //���� ��ʱʱ��Ƭ ����������Ӧ���ʱ�䳬ʱ  100*10ms
#define PTCL_DATATIMEOUT 3 //������ʱ�ȴ���ʱ�� ����ʱ��Ƭ 3*10ms ���ݵĳ��ȸı�
#define PTCL_RECDATALEN 50 //�������ݵĳ��� DMA Э����󳤶�
static const Pin PTCL_Pin[]={PIN_USART1_TXD, PIN_USART1_RXD};
////
#define PCTL_GET_TICKTIME  xTaskGetTickCountFromISR() ///��ȡϵͳʱ��

typedef enum {
LMSCMDFLAG=0x01,//����
LMSACKFLAG=0x07,//Ӧ��
}PROTCOL_PACK_FLAG_ENUM;

//������������
typedef enum{
LMS_OPEN_DOOR_TYPE=0x01,//���ŷ�ʽ 1B
LMS_INFRARED_TYPE=0x02,//���ⷽʽ 1B
LMS_STOP_TIME_TYPE=0x03,//ͣ��ʱ�� 2B
LMS_REVERSAL_TIME_TYPE=0x04,//��תʱ�� 2B
LMS_MOTOR_TORQUE_TYPE=0x05,//���Ť�� 1B
LMS_AUTO_LOCK_TYPE=0x06,//�Զ����� 1B
LMS_DOORBELL_TYPE=0x07,//���� 1B
LMS_VOICE_VOLUME_TYPE=0x08,//���� 1B
LMS_LANGUAGE_TYPE=0x09,//���� 1B
LMS_TRMPER_TYPE=0x0A,//���� 1B
}LMS_PARAMETER_SET_TYPE_ENUM; 

//�¼�����
typedef enum
{
LMS_DOOROPEN_REMIND_TYPE=0x01,	//�������� -0x01
LMS_LOW_POWERALERT_TYPE=0x02,//�͵�������
LMS_ERROR_MORE_TYPE=0x04,//����������
}LMS_EVENT_TYPE_ENUM;

//�˵�����
typedef enum{
LMS_RFID_SET=0x01,//�û���
LMS_REMOTE_CTL_SET=0x02,//ң����
LMS_GATEWAY_SET=0x03,//����
LMS_TRMPER_SET=0x04,//��������
}LMS_ENUM_TYPE_ENUM;


//////ack
typedef enum
{
LMS_KEY_VALUE_WUP_TYPE=0x01,//��������
LMS_AUTO_LOCK_WUP_TYPE=0x02,//�Զ�����
LMS_REMOTE_WUP_TYPE=0x03,//ң��ֵ
LMS_RFID_WUP_TYPE=0x04,//�ǽӴ���Ƶ��
LMS_TRMPER_WUP_TYPE=0x05,//����ť
}LMS_WAKEUP_TYPE_ENUM;



//////////////////ʱ��&���� ��ʽ2017��01��01�� 12:11:00  
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
������:PTCL_CmdAnalysis
��������:��Э�鴦��
�������: ProtocolDir: 1:PROTOCOL_TO_USART  2:USART_TO_PROTOCOL
          ptclCmd:Ӧ�ò㷢����������
          pBuf:����ָ��
          len:���ݳ���

********************************************************/
unsigned char PTCL_CmdAnalysis(PROTOCOL_DIRECTIOPN_ENUM ProtocolDir,unsigned int ptclCmd,unsigned char *pBuf,unsigned short bufLen)
{
        unsigned short cmdLen=bufLen;
		unsigned char  *pCmdBuf=pBuf;
		unsigned char crc=0,i=0;

		unsigned int t1;//�ȴ�ʱ��
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
			    //1.�����ֽ�
			    PCTL_MemSet(gPtclPro.txBuf,LMS_PROTOCOLID_WAKEUP_VALUE,LMS_PROTOCOLID_WAKEUP_N);
			    i+=LMS_PROTOCOLID_WAKEUP_N;
			    //2.��ͷ
			    gPtclPro.txBuf[i++]=(LMS_PROTOCOLID_PACKED_START>>8)&0xFF;
			    gPtclPro.txBuf[i++]=(LMS_PROTOCOLID_PACKED_START)&0xFF;	
                //3.������
				gPtclPro.txBuf[i++]=0x00;
				//gPtclPro.txBuf[i++]=(unsigned char)((ptclCmd)&0xFF);
				//4.����
				gPtclPro.txBuf[i++]=0x00;
                //5.���ı�ʶ
                gPtclPro.txBuf[i++]=LMSCMDFLAG; 				
		   		switch(ptclCmd)//Ӧ�ò������
				{
                   case FACE_FRONT_WAKEUP_CMD://ǰ�˻���
		                //ʱ�� 6Byte
		                PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
						i+=6;
		                gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_FRONT_WAKEUP_CMD&0xFF;
		                gPtclPro.txBuf[i++]=pFaceObject->wakeUpFlag;//���ѱ�־
		                				
				   break;
				   case FACE_READ_ELECTRIC_CMD://��ȡ����
				   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_READ_ELECTRIC_CMD&0xFF;
				   break;
				   case FACE_PARAMETER_SET_CMD://��������
				       gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_PARAMETER_SET_CMD&0xFF;				   
					   gPtclPro.txBuf[i++]=pFaceObject->parameterSetType;//��������
					   switch(gPtclPro.txBuf[i-1])
					   {
                         case LMS_OPEN_DOOR_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->doorTypeFlag;//���ŷ�ʽ
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_doorTypeFlag;//���ŷ�ʽ
						 	break;
						 case LMS_INFRARED_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->infraedType;//���ⷽʽ  
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_Infrared;
						 	break;
						 case LMS_STOP_TIME_TYPE:
						 	//////////////////////�ȸߺ��
						 //	gPtclPro.txBuf[i++]=(pFaceObject->waitTime>>8)&0xFF;//ͣ��ʱ��1-9s 2byte 
						 	//gPtclPro.txBuf[i++]=pFaceObject->waitTime&0xFF;//ͣ��ʱ��1-9s 2byte 
						 	gPtclPro.txBuf[i++]=(pFaceObject->usermanage->Maintain_PauseTime>>8)&0xFF;//ͣ��ʱ��1-9s 2byte 
							gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_PauseTime&0xFF;//ͣ��ʱ��1-9s 2byte 
						 	break;
						 case LMS_REVERSAL_TIME_TYPE:
						 	//////////////////////�ȸߺ��
						 //	gPtclPro.txBuf[i++]=(pFaceObject->reversalTime>>8)&0xFF;//��תʱ�� 200-900ms 2byte
						 //	gPtclPro.txBuf[i++]=pFaceObject->reversalTime&0xFF;//��תʱ�� 200-900ms 2byte

							gPtclPro.txBuf[i++]=(pFaceObject->usermanage->Maintain_reversalTime>>8)&0xFF;//��תʱ�� 200-900ms 2byte
							gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_reversalTime&0xFF;//��תʱ�� 200-900ms 2byte
						 	break;
						 case LMS_MOTOR_TORQUE_TYPE:
						 //	gPtclPro.txBuf[i++]=pFaceObject->motorTorqueType;//���Ť��  
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_motorTorqueType;//���Ť��  
						 	break;
						 case LMS_AUTO_LOCK_TYPE:
						 //	gPtclPro.txBuf[i++]=pFaceObject->autoLockType;//�Զ����� 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->AutoLock;//�Զ����� 
						 	break;
						 case LMS_DOORBELL_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->doorBellTpe;//���� 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Voice_Bell_TypeAndValue;//���� 
						 	break;
						 case LMS_VOICE_VOLUME_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->voiceVolumeType;//���� 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Voice_Speech_Value;//���� 
						 	break;
						 case LMS_LANGUAGE_TYPE:
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->language;//����
						 	break;
						 case LMS_TRMPER_TYPE:
						 	//gPtclPro.txBuf[i++]=pFaceObject->antiTamperFlag;//���� 
						 	gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_antiTamperFlag;//���� 
						 	break;
                         default:   break;
					   }
					   

	               break; 
				   case FACE_OPEN_CLOSE_DOOR_CMD://������
				        gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_OPEN_CLOSE_DOOR_CMD&0xFF;
                        gPtclPro.txBuf[i++]=pFaceObject->openCloseFlag;//�����ű�־
				        PCTL_MemCopy(&gPtclPro.txBuf[i],pFaceObject->openCloseReserve,7);//���� 7B
						i+=7;						

				   break;
				   case FACE_GET_FIRMWARE_INF_CMD://��ȡ�̼���Ϣ
					   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_GET_FIRMWARE_INF_CMD&0xFF;

					break;
				   case FACE_MEASUREMENT_MODE_CMD:////����ģʽ
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_MEASUREMENT_MODE_CMD&0xFF;
                       // gPtclPro.txBuf[i++]=pFaceObject->measurementState;//״̬ 
						gPtclPro.txBuf[i++]=pFaceObject->usermanage->Maintain_Cycl_Demonstration;//״̬ 

				   break;
				   case FACE_VOICE_MODE_CMD://����ģʽ
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_VOICE_MODE_CMD&0xFF;
				   break;
                   
				   case FACE_EVENT_INF_CMD://�¼���Ϣ
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_EVENT_INF_CMD&0xFF;
				        gPtclPro.txBuf[i++]=pFaceObject->eventType;//�¼�����
				        switch(gPtclPro.txBuf[i-1])
						{
							case LMS_DOOROPEN_REMIND_TYPE:
								gPtclPro.txBuf[i++]=pFaceObject->user1;//�û�1
								gPtclPro.txBuf[i++]=pFaceObject->user2;//�û�2
								//ʱ��
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								gPtclPro.txBuf[i++]=pFaceObject->openDoorType;//���ŷ�ʽ 
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeUp;//�ϵ�ص���
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeDown;//�µ�ص���
								
								break;
							case LMS_LOW_POWERALERT_TYPE:
								//ʱ��
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeUp;//�ϵ�ص���
								gPtclPro.txBuf[i++]=pFaceObject->powerVolumeDown;//�µ�ص���
								break;
							case LMS_ERROR_MORE_TYPE:
								PCTL_GetTimeData(&gPtclPro.txBuf[i],6);
								i+=6;
								//ʱ��
								gPtclPro.txBuf[i++]=pFaceObject->errorMoreType;//����-1 ����-2 �û���-3
								break;
							default :  break;	
						}

				   break;
				   case FACE_MENU_OPERATION_CMD:////�˵�����
				   		gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_MENU_OPERATION_CMD&0xFF;
				        gPtclPro.txBuf[i++]=pFaceObject->menuDataType;//�������� 
				        gPtclPro.txBuf[i++]=pFaceObject->menuOperate;//���or�ر�-0x01 ɾ��or����-0x02  ���-0x03
				        
				   break;
				   case FACE_RECOVER_MODE_CMD://�ָ���������
						gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_RECOVER_MODE_CMD&0xFF;
				   break;

				   case FACE_SLEEP_MODE_CMD://����
					   gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_SLEEP_MODE_CMD&0xFF;

				   break;

                   case FACE_UPLOAD_EVENT_INF_CMD:////�ϴ��¼���Ϣ 
                        
                        gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3]=LMS_UPLOAD_EVENT_INF_CMD&0xFF;//����
						gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]=LMSACKFLAG;//���İ���ʶ��
						switch(pBuf[0])
						{	
						   case FACE_OPERATE_FAILED_ACK://����ʧ��
						        gPtclPro.txBuf[i++]=LMS_OPERATE_FAILED_ACK&0xFF;
 								break;
						   case FACE_OPERATE_SUCCEED_ACK://�����ɹ�
						        gPtclPro.txBuf[i++]=LMS_OPERATE_SUCCEED_ACK&0xFF;
						        break;
						   case FACE_OPERATE_CRC_ERROR_ACK://У��ʹ���
						        gPtclPro.txBuf[i++]=LMS_OPERATE_CRC_ERROR_ACK&0xFF;
						   break;
						}
						
				   break;
				   
                   default:    
				   break;
				}

	         	    gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+2]=i+1-3-LMS_PROTOCOLID_WAKEUP_N;//���¼��㳤��
	                crc=crcGetCrc(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+2],i-LMS_PROTOCOLID_WAKEUP_N-2);
					gPtclPro.txBuf[i++]=crc;//У��
                    gPtclPro.Cmd=gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3];//��������
				//////////////����
				    gPtclPro.txLen=i;//Ҫ���͵����ݸ���
				    if(gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]==LMSCMDFLAG)//��������������Ҫ�����ֽ�
					{
				    //�����ֽ�
				    PTCL_UsartSend(gPtclPro.txBuf,LMS_PROTOCOLID_WAKEUP_N);//���ͻ����ֽ�
					t1=PCTL_GET_TICKTIME;
					while(t1+LMS_PROTOCOLID_WAKEUP_TIME>PCTL_GET_TICKTIME);//�ȴ�����ʱ��
					
					//����������
				    PTCL_UsartSend(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N],gPtclPro.txLen-LMS_PROTOCOLID_WAKEUP_N);
					gPtclPro.recType=PTCL_INITISTIVE_REC;//����ģʽ
					}
					else if(gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N+3+1]==LMSACKFLAG)//����Ӧ������
					{
					
					//����������
				    PTCL_UsartSend(&gPtclPro.txBuf[LMS_PROTOCOLID_WAKEUP_N],gPtclPro.txLen-LMS_PROTOCOLID_WAKEUP_N);
					gPtclPro.recType=PTCL_PASSIVE_REC;//����ģʽ	
					}
					
		   break;
		   }

		   
           case USART_TO_PROTOCOL://��������
		   {
                
				//1.�����ֽ�
				//while(pCmdBuf[i++]==0x00)//�����ֽ�
                //if(i>256) 
                //{
                // return FACE_OTHER_ERROR_ACK;
				//}
		        //2.��ͷ
		        head=((unsigned short)pCmdBuf[i++])<<8;
				head|=pCmdBuf[i++];
		        if(head!=LMS_PROTOCOLID_PACKED_START)
				return FACE_OTHER_ERROR_ACK;//head error	
				//3.����
				len=pCmdBuf[i++];
				//�ж�У���Ƿ���ȷ
                crc=crcGetCrc(&pCmdBuf[i-1],(unsigned short)len);
				if(crc!=pCmdBuf[i-1+len])
				return FACE_OPERATE_CRC_ERROR_ACK;//crc error	
		        //4.����
		        cmd=pCmdBuf[i++];
				//if(cmd!=gPtclPro.Cmd)
				//return FACE_OTHER_ERROR_ACK;//�������
		        //5.����ʶ
		        //if(pCmdBuf[i++]!=LMSACKFLAG)					
				//return FACE_OTHER_ERROR_ACK;//packflag error
				if(pCmdBuf[i]==LMSACKFLAG&&gPtclPro.recType==PTCL_INITISTIVE_REC) //Ӧ������
				{
				i++;
				///����
			        if(cmd!=gPtclPro.Cmd)
				    return FACE_OTHER_ERROR_ACK;//�������
				//6.ȷ����
					switch(pCmdBuf[i++])
					{
	                	case LMS_OPERATE_FAILED_ACK://����ʧ��
	                	ack=FACE_OPERATE_FAILED_ACK;
	                	break;
						case LMS_OPERATE_SUCCEED_ACK://�����ɹ�
						
							switch(cmd)
						    {
						   		case LMS_FRONT_WAKEUP_CMD://ǰ�˻���
						   		pFaceObject->wakeUpType=pCmdBuf[i++];//��������-0x01  �Զ�����-0x02 ң�ػ���-0x03  �ǽӴ���Ƶ��-0x04 ����-0x05
								switch(pFaceObject->wakeUpType)
								{
	                             	case LMS_KEY_VALUE_WUP_TYPE:
									   
	                                   pFaceObject->wUpTypeKeyValue=pCmdBuf[i++];//����-0x42 ����-0x43 ����-0x45
										
										break;
									case LMS_AUTO_LOCK_WUP_TYPE:
										pFaceObject->wUpTypeAutoCloseDoor=pCmdBuf[i++];//�Զ�����
										
										break;
									case LMS_REMOTE_WUP_TYPE:
										pFaceObject->wUpTypeRemoteControl=pCmdBuf[i++];//ң��ֵ
										break;
									case LMS_RFID_WUP_TYPE:
										//�ߵ�λ
										pFaceObject->wUpTypeRFID[0]=pCmdBuf[i++];//RFID 4Byte ���� ���ֽ���ǰ ��174C526D
										pFaceObject->wUpTypeRFID[1]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[2]=pCmdBuf[i++];
										pFaceObject->wUpTypeRFID[3]=pCmdBuf[i++];
										break;
									case LMS_TRMPER_WUP_TYPE:
										pFaceObject->wUpTypeAntiTamper=pCmdBuf[i++];//����//1��Ч 0��Ч
										break;
									default:
										ack=FACE_OTHER_ERROR_ACK;
										break;	
	                            
								}							
	                           
	                          //  pFaceObject->expandUserN=pCmdBuf[i++];////��չ�û���
	                            pFaceObject->usermanage->Remote_And_ExternalModule=pCmdBuf[i++];////��չ�û���
								//��������
	                          //  pFaceObject->doorTypeFlag=pCmdBuf[i++];//���ŷ�ʽ  0-���� 1-�ҿ���
	                            pFaceObject->usermanage->Maintain_doorTypeFlag=pCmdBuf[i++];//���ŷ�ʽ  0-���� 1-�ҿ���
	                           // pFaceObject->infraedType=pCmdBuf[i++];//���ⷽʽ 
	                            pFaceObject->usermanage->Maintain_Infrared=pCmdBuf[i++];//���ⷽʽ 
	                          //  pFaceObject->waitTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//ͣ��ʱ��
	                            pFaceObject->usermanage->Maintain_PauseTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//ͣ��ʱ��
	                           // pFaceObject->reversalTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//��תʱ�� 
	                            pFaceObject->usermanage->Maintain_reversalTime=((unsigned short)pCmdBuf[i])<<8|pCmdBuf[i+1];i+=2;//��תʱ�� 
							//	pFaceObject->motorTorqueType=pCmdBuf[i++];//���Ť�� 
								pFaceObject->usermanage->Maintain_motorTorqueType=pCmdBuf[i++];//���Ť�� 
							//	pFaceObject->autoLockType=pCmdBuf[i++];//�Զ����� 
								pFaceObject->usermanage->AutoLock=pCmdBuf[i++];//�Զ�����
								//pFaceObject->doorBellTpe=pCmdBuf[i++];//���� 
								pFaceObject->usermanage->Voice_Bell_TypeAndValue=pCmdBuf[i++];//���� 
								//pFaceObject->voiceVolumeType=pCmdBuf[i++];//���� 
								pFaceObject->usermanage->Voice_Speech_Value=pCmdBuf[i++];//���� 
								pFaceObject->usermanage->language=pCmdBuf[i++];//����							
								//pFaceObject->antiTamperFlag=pCmdBuf[i++];//���� 
								pFaceObject->usermanage->Maintain_antiTamperFlag=pCmdBuf[i++];//���� 
	                            //////����
	                            pFaceObject->batteryPos=pCmdBuf[i++];////���λ��
	                            pFaceObject->powerVolumeUp=pCmdBuf[i++];//�ϵ�ص���
								pFaceObject->powerVolumeDown=pCmdBuf[i++];//�µ�ص���
								break;
								case LMS_READ_ELECTRIC_CMD://��ȡ����

								
					        	break;
								case LMS_PARAMETER_SET_CMD://��������
								if(pFaceObject->parameterSetType!=pCmdBuf[i++])//������������
								return FACE_OTHER_ERROR_ACK;
								
								break;
								case LMS_OPEN_CLOSE_DOOR_CMD://������
	                            if(pFaceObject->openCloseFlag!=pCmdBuf[i++])//�����ű�־ ����-0x4F ����-0x43
								return FACE_OTHER_ERROR_ACK;
								break;

								case LMS_GET_FIRMWARE_INF_CMD://��ȡ�̼���Ϣ
								//�汾 7B
								PCTL_MemCopy(pFaceObject->vertionFirware,&pCmdBuf[i],7);i+=7;
								//ϵ�к� 7B
								PCTL_MemCopy(pFaceObject->serialNumber,&pCmdBuf[i],7);i+=7;
								break;
								case LMS_MEASUREMENT_MODE_CMD://����ģʽ
								
								break;
								case LMS_VOICE_MODE_CMD://����ģʽ
								break;
								case LMS_EVENT_INF_CMD://�¼���Ϣ
								break;
								case LMS_MENU_OPERATION_CMD://�˵�����
								
								if(pFaceObject->menuDataType!=pCmdBuf[i++])
								return  FACE_OTHER_ERROR_ACK;
	 							switch(pFaceObject->menuDataType)
								{
	 								case LMS_RFID_SET://��ӵ��û�id
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
								case LMS_RECOVER_MODE_CMD://�ָ���������
								
								break;
								case LMS_SLEEP_MODE_CMD://����
								
								break;
						    }
	 
	                    ack=FACE_OPERATE_SUCCEED_ACK;
						break;
						case LMS_OPERATE_CRC_ERROR_ACK://У��ʹ���
						ack=FACE_OPERATE_CRC_ERROR_ACK;
						break;
						
						case LMS_THEPOS_IS_EXPAND_USER_ACK://ָ��λ���Ѿ�����չ�û�
		                ack=FACE_THEPOS_IS_EXPAND_USER_ACK;
						break;
						
						case LMS_EXPAND_USER_EXIST_AXK://��չ�û��Ѿ�����
			            ack=FACE_EXPAND_USER_EXIST_AXK;
						break;
						
						case LMS_EXPAND_USER_NOSPACE_ACK://��չ�û��洢��
			            ack=FACE_EXPAND_USER_NOSPACE_ACK;
						break;
						
						case LMS_OPERATE_TIMEOUT_ACK://��ʱ�˳�
			            ack=FACE_OPERATE_TIMEOUT_ACK;
						break;
					
						default: 
							ack=FACE_OTHER_ERROR_ACK;
							break;   
					}
				}
				else if(pCmdBuf[i]==LMSCMDFLAG&&gPtclPro.recType==PTCL_PASSIVE_REC)//��������
				{
				  i++;
				  switch(cmd)
				  {
						case LMS_UPLOAD_EVENT_INF_CMD://�ϴ��¼���Ϣ
						pFaceObject->wakeUpType=pCmdBuf[i++];//��������-0x01  �Զ�����-0x02 ң�ػ���-0x03  �ǽӴ���Ƶ��-0x04 ����-0x05
						switch(pFaceObject->wakeUpType)
						{
                         	case LMS_KEY_VALUE_WUP_TYPE:
							   
                               pFaceObject->wUpTypeKeyValue=pCmdBuf[i++];//����-0x42 ����-0x43 ����-0x45
								
								break;
							case LMS_AUTO_LOCK_WUP_TYPE:
								pFaceObject->wUpTypeAutoCloseDoor=pCmdBuf[i++];//�Զ�����
								
								break;
							case LMS_REMOTE_WUP_TYPE:
								pFaceObject->wUpTypeRemoteControl=pCmdBuf[i++];//ң��ֵ
								break;
							case LMS_RFID_WUP_TYPE:
								//�ߵ�λ
								pFaceObject->wUpTypeRFID[0]=pCmdBuf[i++];//RFID 4Byte ���� ���ֽ���ǰ ��174C526D
								pFaceObject->wUpTypeRFID[1]=pCmdBuf[i++];
								pFaceObject->wUpTypeRFID[2]=pCmdBuf[i++];
								pFaceObject->wUpTypeRFID[3]=pCmdBuf[i++];
								break;
							case LMS_TRMPER_WUP_TYPE:
								pFaceObject->wUpTypeAntiTamper=pCmdBuf[i++];//����//1��Ч 0��Ч
								break;
							default: 
							ack=FACE_OTHER_ERROR_ACK;
							break;	
                        
						}
						//����
				        pFaceObject->batteryPos=pCmdBuf[i++];////���λ��
                        pFaceObject->powerVolumeUp=pCmdBuf[i++];//�ϵ�ص���
						pFaceObject->powerVolumeDown=pCmdBuf[i++];//�µ�ص���
						ack=FACE_UNLOAD_EVENT_INF_ACK;	
						break;
					  default:
					  	ack=FACE_OTHER_ERROR_ACK;
					  	break;
				  }
				}
				else//����ʶ����
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
����:����DMA�������� 
Э���ڽ�������ʱ����DMA��ʽ
*/

static void PTCL_UsartDmaConfigure(void)
{
    unsigned int  status;
    sXdmadCfg xdmadCfg;
	XDMAD_Initialize(&gPtclPro.dmad,0);
	gPtclPro.dmaChannel=XDMAD_AllocateChannel( &gPtclPro.dmad, USART_ID_PTCL, XDMAD_TRANSFER_MEMORY);//
	XDMAD_PrepareChannel( &gPtclPro.dmad,gPtclPro.dmaChannel);


		xdmadCfg.mbr_ubc = PTCL_RECDATALEN;  //���峤��
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
����:���ڳ�ʼ��

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
����:�򴮿ڷ�����
����:
dat:���� len:����
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
����:PTCL_CMDSEND ���������
����:
ptclCmd:��Ӧ������  
FACE_PROTOCOL_ENUM
value:ҪӦ��ʱ���õ�Ӧ���� 
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
����:����DMA�������ݵķ�ʽ����
����ֵ: FACE_PROTOICOL_ACK_CODE_ENUM
FACE_OPERATE_FAILED_ACK=0x00,//����ʧ��
FACE_OPERATE_SUCCEED_ACK=0x01,//�����ɹ�	
FACE_OPERATE_CRC_ERROR_ACK=0x02,//У��ʹ���
FACE_THEPOS_IS_EXPAND_USER_ACK=0x03,//ָ��λ���Ѿ�����չ�û�
FACE_EXPAND_USER_EXIST_AXK=0x04,//��չ�û��Ѿ�����
FACE_EXPAND_USER_NOSPACE_ACK=0x05,//��չ�û��洢��
FACE_OPERATE_TIMEOUT_ACK=0x06,//��ʱ�˳�

FACE_OTHER_ERROR_ACK=0xFD,//��������(���ݽ�������)
FACE_TIMEOUT_NOACK=0xFE,//û��Ӧ��ʱ
FACE_WAIT_PRO=0xFF,//�ȴ�����
FACE_UNLOAD_EVENT_INF_ACK=0x50,//�ϴ��¼���Ϣ

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
		  
		  if(t1+10<PCTL_GET_TICKTIME)//10msʱ��Ƭ�ȴ�ʱ��
		  {
		      t1=PCTL_GET_TICKTIME;
			  gPtclPro.framTimeout++;
			  if(gPtclPro.framTimeout>PTCL_TIMEOUT)//�������ݽ��ճ�ʱ
			  {
			     ret=FACE_TIMEOUT_NOACK;
			     gPtclPro.framTimeout=0;
				 gPtclPro.recType=PTCL_PASSIVE_REC;
				 PTCL_UsartDmaConfigure();//�������ý���DMA
				 //gPtclPro.rxLen=0;
				 //PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
				 return ret;
			  }
		  }
		  if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//������
	      {
	        t1=PCTL_GET_TICKTIME;
	        while(t1+10*PTCL_DATATIMEOUT>PCTL_GET_TICKTIME);	
			//gPtclPro.timeoutCnt++;
			gPtclPro.framTimeout=0;
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//ˢ��FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//����
			//gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//�������ý���DMA
			gPtclPro.recType=PTCL_PASSIVE_REC;
			return ret;	   
	      }
	 }
	}
   else
   {
        
		if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//������
		{
		    t1=PCTL_GET_TICKTIME;
	        while(t1+10*PTCL_DATATIMEOUT>PCTL_GET_TICKTIME);	
			//gPtclPro.timeoutCnt++;
			gPtclPro.framTimeout=0;//
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//ˢ��FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//����
			//gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//�������ý���DMA
			gPtclPro.recType=PTCL_PASSIVE_REC;
			return ret;
		}
		
   }
   return  FACE_WAIT_PRO;//�ȴ�����
 /*  
   if(gPtclPro.recType==PTCL_PASSIVE_REC)//����
   {
      gPtclPro.framTimeout++;
	  if(gPtclPro.framTimeout>PTCL_TIMEOUT)//�������ݽ��ճ�ʱ
	  {
	     ret=FACE_TIMEOUT_NOACK;
	     gPtclPro.framTimeout=0;
		 gPtclPro.recType=PTCL_INITISTIVE_REC;
		 return ret;
	  } 
	}
   
   if(gPtclPro.rxBuf[0]!='\0'&&gPtclPro.rxBuf[1]!='\0')//������
   {
		gPtclPro.timeoutCnt++;
		gPtclPro.framTimeout=0;//
		if(gPtclPro.timeoutCnt>=PTCL_DATATIMEOUT)//�ȴ�һ��ʱ��Ƭ
		{
			XDMAD_StopTransfer(&gPtclPro.dmad,gPtclPro.dmaChannel);//ˢ��FIFO 
			ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,0);//����
			gPtclPro.timeoutCnt=0; 
			gPtclPro.rxLen=0;
			PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
			PTCL_UsartDmaConfigure();//�������ý���DMA
			gPtclPro.recType=PTCL_INITISTIVE_REC;
			return ret;
		}
   
   }
   return  FACE_WAIT_PRO;//�ȴ�����
   */

}	

/*
����:��Э��ṹ���ʼ��
����Э��ʱ�����ȳ�ʼ��
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
	gPtclPro.timeoutCnt=0;//ʱ���ʱ0
	gPtclPro.recType=PTCL_PASSIVE_REC;//������ʽ
}






///////////////////////////////////////////////////////////////
/*
����:��ѯ��������
�г�ʱ����
*/
//void PTCL_UsartRec(void)
//{
//     unsigned char value;
//     if(USART_IsRxReady(USART_PTCL)&&gPtclPro.step==PTCL_STEP2)//�Ƿ�������
//	 {
//        value=USART_GetChar(USART_PTCL);
//		if(value==0x00&&gPtclPro.rxBuf[0]!=0xAA)
//        return ;
//		gPtclPro.rxBuf[gPtclPro.rxLen++]=value;
//	 }	
//	 if(PCTL_GET_TICKTIME>gPtclPro.framTimeout+PTCL_TIMEOUT&&gPtclPro.step==PTCL_STEP2)//�жϳ�ʱʱ��
//	 {
//	    gPtclPro.step=PTCL_STEP1;
//        gPtclPro.rxFlag=ptcl_ture;//ʱ�䵽
//	 }
//}
/*
���ݽ��մ�����
����ֵ: FACE_PROTOICOL_ACK_CODE_ENUM
FACE_OPERATE_FAILED_ACK=0x00,//����ʧ��
FACE_OPERATE_SUCCEED_ACK=0x01,//�����ɹ�	
FACE_OPERATE_CRC_ERROR_ACK=0x02,//У��ʹ���
FACE_THEPOS_IS_EXPAND_USER_ACK=0x03,//ָ��λ���Ѿ�����չ�û�
FACE_EXPAND_USER_EXIST_AXK=0x04,//��չ�û��Ѿ�����
FACE_EXPAND_USER_NOSPACE_ACK=0x05,//��չ�û��洢��
FACE_OPERATE_TIMEOUT_ACK=0x06,//��ʱ�˳�

FACE_OTHER_ERROR_ACK=0xFD,//��������(���ݽ�������)
FACE_TIMEOUT_NOACK=0xFE,//û��Ӧ��ʱ
FACE_WAIT_PRO=0xFF,//�ȴ�����
*/
//unsigned char PTCL_UsartPro(void)
//{
//   unsigned char ret;
//   //����Э��ʱ�ȴ�����
//   if(gPtclPro.rxFlag==ptcl_ture)
//   {
//     gPtclPro.rxFlag=ptcl_false;
//	 if(gPtclPro.rxLen<=0)//û�н��յ����ݳ�ʱ
//	 {
//       ret=FACE_TIMEOUT_NOACK;
//	 }
//	 else //���յ����ݽ��д���
//	 {
//      ret=gPtclPro.pFramCb(USART_TO_PROTOCOL,0,gPtclPro.rxBuf,gPtclPro.rxLen);//����
//      gPtclPro.framTimeout=0;
//	  gPtclPro.rxLen=0;
//	  PCTL_MemSet(gPtclPro.rxBuf, 0, 256);
//	 }	
//	 return ret;
//  }  
//  return FACE_WAIT_PRO;//�ȴ�����
//}	
////////////////////////////////////////////////////////////////////////////////



#endif





