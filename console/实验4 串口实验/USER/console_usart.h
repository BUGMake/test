
#ifndef CONSOLE_USART_
#define CONSOLE_USART_
#include "queue.h"


//��ʱֻ֧��char
#define CONSOLE_PARM_LEN_MAX  15 //�����ĳ���  
#define CONSOLE_PARM_NUM_MAX  5 //�����ĸ���
#define CONSOLE_FUNCNAME_LEN_MAX  25 //����������
#define CONSOLE_FRAME_DATA_LEN_MAX   100 //һ֡���ݵĳ��� ������+����
//�Ƿ�֧��ʱ��  
#define CONSOLE_TIME_GET  0
//#define CONSOLE_TIME_GET  getSystickCount()

//////
#define CONSOLE_REC_BUF_MAX 128

///////
#define CONSOLE_PRINT printf
#define CONSOLE_PRINTK
#define CONSOLE_PRINTTIME

typedef enum{
CONSOLE_LS_CMD=0,//ls �г����������
CONSOLE_ADD_CMD=1,//������� add ������=����
CONSOLE_DEL_CMD=2,//ɾ������ del ����
CONSOLE_EXE_CMD=3,//ִ������
}CONSOLE_CMD_ENUM;

//���������������ֵ
typedef enum{
CONSOLE_REC_PRO_OK=0,//�ɹ�
CONSOLE_REC_PRO_ERROR=1,//ʧ��
}CONSOLE_REC_TYPE_ENUM;
//��������
typedef enum{
CONSOLE_FUNC_CMD=0,//��������
CONSOLE_SYS_CMD=1,//ϵͳ����
}CONSOLE_CMD_TYPE_ENUM;

//������Ա 
typedef struct{
  void *func;//����ָ��
  const  char *name;//������(�����ַ���ִ����Ӧ�ú�������)
}CONSOLE_NAMETAB_STRUCT;

///ϵͳ����
typedef enum {
CONSOLE_SYS_LS_CMD=0,
CONSOLE_SYS_LIST_CMD=1,
CONSOLE_SYS_RUNTIME_CMD=2,
}CONSOLE_SYS_CMD_ENUM;

//console ������
typedef struct{


unsigned char funcNum;//�������ܸ���  ��ʼ��ʱ�������
unsigned char funcId;//��Ӧ������id   id����С�ں������ܸ���

unsigned char frameDataBuf[CONSOLE_FRAME_DATA_LEN_MAX];//һ֡����
unsigned char frameDataBuflen;//һ֡���ݵĳ���


unsigned char funcName[CONSOLE_FUNCNAME_LEN_MAX];//�������ַ������ 
unsigned char funcNameLen;//����

unsigned char funcParmn;//�������������� 
unsigned char funcParmTbl[CONSOLE_PARM_NUM_MAX][CONSOLE_PARM_LEN_MAX];//���溯���Ĳ���

unsigned char runTimeFlag;//ʱ�俪����ʶ
unsigned int  runTinme;//����ʱ�� ����ϵͳ�ĵδ�ʱ��


CONSOLE_CMD_TYPE_ENUM cmdType;//�������� ϵͳor��������
CONSOLE_SYS_CMD_ENUM sysCmd;  //ϵͳ����

CONSOLE_NAMETAB_STRUCT *funs;//������ָ��

void (*funcEntry)(void);//�ն˳������ 
CONSOLE_REC_TYPE_ENUM (*cmdRecPro)(unsigned char *,unsigned short);//�������
void (*funcExe)(void);//��������ִ��

}CONSOLE_USART_STRUCT;

extern void consoleInit(void);
extern void consoleProEntry(void);
extern unsigned char consoleRecBuf[CONSOLE_REC_BUF_MAX];
//���й���
extern QueueStruct stconsoleQueue;
//�������
extern CONSOLE_USART_STRUCT stConsoleDev;//�ն��豸������


#endif /*CONSOLE_USART_*/

















