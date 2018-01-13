
#ifndef FACE_PROTOCOL
#define FACE_PROTOCOL
#include "board.h"


/////////////////////Э�鷽��
typedef enum{
PROTOCOL_TO_USART=1,
USART_TO_PROTOCOL=2,
//PROTOCOL_TO_IIC=3,
}PROTOCOL_DIRECTIOPN_ENUM;


/////////////////////����Э��
typedef enum{
FACE_FRONT_WAKEUP_CMD=0x00000001,//ǰ�˻���
FACE_READ_ELECTRIC_CMD=0x00000002,//��ȡ����
FACE_PARAMETER_SET_CMD=0x00000003,//��������
FACE_OPEN_CLOSE_DOOR_CMD=0x00000004,//������
FACE_GET_FIRMWARE_INF_CMD=0x00000005,//��ȡ�̼���Ϣ
FACE_MEASUREMENT_MODE_CMD=0x00000006,//����ģʽ
FACE_VOICE_MODE_CMD=0x00000007,//����ģʽ
FACE_EVENT_INF_CMD=0x00000008,//�¼���Ϣ
FACE_MENU_OPERATION_CMD=0x00000009,//�˵�����
FACE_RECOVER_MODE_CMD=0x0000000A,//�ָ���������
FACE_SLEEP_MODE_CMD=0x0000000B,//����

FACE_UPLOAD_EVENT_INF_CMD=0x0000000C,////�ϴ��¼���Ϣ

FACE_ALL_CMD=0xFFFFFFFF,//���
}FACE_PROTOCOL_ENUM;


////////////////////Ӧ��Э��
typedef enum{	
    //////////////////����Ӧ������֡
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

	////////////////������������֡��־
    FACE_UNLOAD_EVENT_INF_ACK=0x50,//�ϴ��¼���Ϣ
	
	/////////////////
}FACE_PROTOICOL_ACK_CODE_ENUM;



///////////////////
typedef enum{
PTCL_STEP1=0,
PTCL_STEP2=1,
PTCL_STEP3=2,
}PTCL_SERVER_STEP;
//Э����շ�ʽ �����ͱ���
typedef enum{
PTCL_INITISTIVE_REC=0,//����  �������ȷ������ݣ��Ŷ�Ӧ������
PTCL_PASSIVE_REC=1,//���� �Ŷ��ȷ������ݣ�������Ӧ��
}PTCL_REC_TYPE_ENYM;


//�������ݵı�ʶ����
#define ptcl_ture 1
#define ptcl_false 0

typedef unsigned char (*FramDataPro)(PROTOCOL_DIRECTIOPN_ENUM ProtocolDir,unsigned int ptclCmd,unsigned char *pBuf,unsigned short bufLen);
/////////////
typedef struct
{
	PTCL_SERVER_STEP step;		        //��ǰЭ������״̬ Ӧ����ѯ��������
	unsigned int framTimeout;			//֡����ʱʱ�� �������ݺ�ȴ��������ݵĳ�ʱʱ��
	FramDataPro pFramCb;                //֡���ݻص�����  
	unsigned char rxBuf[256];           //�������ݻ���ͳ���
	unsigned char rxLen;
	unsigned char rxFlag;               //�������ݱ�ʶ
	unsigned char txBuf[256];           //�������ݻ���ͳ���
	unsigned char txLen;	
	unsigned char Cmd;                  //��������


	/////////////DMA driver 
	sXdmad dmad;                        //
	unsigned int dmaChannel;            //DMA driver instance
	unsigned short timeoutCnt;          //��⵽���ݺ�ĵȴ�ʱ��
	PTCL_REC_TYPE_ENYM recType;        //���ݽ��շ�ʽ ����(��ʱ���Խ��յ�����)�ͱ���(һӦһ��)
	
	
}PTCL_SERVER_STRUCT;





#define LMS_PROTOICOL






#ifdef LMS_PROTOICOL//����˹

//���������ֵ
enum{

LMS_CMDANALYSIS_ERROR=0,
LMS_CMDANALYSIS_OK=1,
};



typedef enum{
LMS_FRONT_WAKEUP_CMD=0xA0,//ǰ�˻���
LMS_READ_ELECTRIC_CMD=0xA1,//��ȡ����
LMS_PARAMETER_SET_CMD=0xA2,//��������
LMS_OPEN_CLOSE_DOOR_CMD=0xA3,//������
LMS_GET_FIRMWARE_INF_CMD=0xA4,//��ȡ�̼���Ϣ
LMS_MEASUREMENT_MODE_CMD=0xA5,//����ģʽ
LMS_VOICE_MODE_CMD=0xA6,//����ģʽ
LMS_EVENT_INF_CMD=0xA7,//�¼���Ϣ
LMS_MENU_OPERATION_CMD=0xA8,//�˵�����
LMS_RECOVER_MODE_CMD=0xA9,//�ָ���������
LMS_SLEEP_MODE_CMD=0xAF,//����

LMS_UPLOAD_EVENT_INF_CMD=0xB0,//�ϴ��¼���Ϣ
}LMS_PROTOICOL_CMD_ENUM;



typedef enum{
LMS_OPERATE_FAILED_ACK=0x00,//����ʧ��
LMS_OPERATE_SUCCEED_ACK=0x01,//�����ɹ�
LMS_OPERATE_CRC_ERROR_ACK=0xe0,//У��ʹ���
LMS_THEPOS_IS_EXPAND_USER_ACK=0xe1,//ָ��λ���Ѿ�����չ�û�
LMS_EXPAND_USER_EXIST_AXK=0xe2,//��չ�û��Ѿ�����
LMS_EXPAND_USER_NOSPACE_ACK=0xe3,//��չ�û��洢��
LMS_OPERATE_TIMEOUT_ACK=0xe4,//��ʱ�˳�
}LMS_PROTOICOL_ACK_CODE_ENUM;
//�����
typedef struct
{
   unsigned short cmd_pack_wakeup_len;//�����ֽڳ���
   unsigned char*cmd_pack_wakeup_data;
   unsigned char cmd_pack_start[2];//��ͷ 0xAA75
   unsigned char cmd_pack_len;
   LMS_PROTOICOL_CMD_ENUM cmd_pack_enum;
   unsigned char cmd_pack_sign;//��ʶ 0x01
   unsigned short cmd_pack_data_len;
   unsigned char *cmd_pack_data;
   unsigned char cmd_pack_crc_sum;
}FACE_PROTOCOL_PACK_CMD_STRUCT;
//Ӧ�����ʽ
typedef struct
{
   unsigned short ack_pack_wakeup_len;//�����ֽڳ���
   unsigned char*ack_pack_wakeup_data;
   unsigned char ack_pack_start[2];//��ͷ 0xAA75
   unsigned char ack_pack_len;
   LMS_PROTOICOL_ACK_CODE_ENUM ack_pack_enum;
   unsigned char ack_pack_sign;//��ʶ 0x01
   unsigned short ack_pack_data_len;
   unsigned char *ack_pack_data;
   unsigned char ack_pack_crc_sum;
}FACE_PROTOCOL_PACK_ACK_STRUCT;



extern void PTCL_UsartInit(void);//���ڳ�ʼ������
extern void PTCL_StructInit(void);//��ʼ������

extern void PTCL_CmdSend(unsigned int ptclCmd,unsigned char value);//�����
extern unsigned char PTCL_UsartProDma(void);//�������ݺ���

//extern void PTCL_UsartRec(void);//��ѯ���մ������ݺ���
//extern unsigned char PTCL_UsartPro(void);//�����ݴ�����












#endif













#endif /* FACE_PROTOCOL */















