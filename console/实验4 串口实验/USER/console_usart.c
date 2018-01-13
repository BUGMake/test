#include "console_usart.h"
#include "Queue.h"
#include "usart.h"
/*************************************************/
//             �ն˹���
//����ʹ��: CMD  argv1 argv2 ...  �س�
//          ADD
//          DEL
//          LS
//ע��:��ʱֻ����ӵĺ��� name(argv1,argv2,...)
//
/************************************************/


 void consoleFuncExe(void);

 
 CONSOLE_REC_TYPE_ENUM consoleCmdProFunc(unsigned char *pStr,unsigned short nlen);

/*
ϵͳ����
ls   //�г�����
list //�г�������
runtime on//��ʱ��
runtime off//��ʱ��

//
��������
show ,void show(void);
setled 1,void setled(int len);
*/
void show1__(void)
	{
		printf("show1__\r\n");
	}
unsigned char led_set__(unsigned char sta)
{
	
	printf("led_set__:%c\r\n",sta);
	return 0;
}
unsigned char play__(unsigned char sta1,unsigned char sta2)
{
	printf("play__:%c,%c\r\n",sta1,sta2);
  return 0;
}

unsigned char consoleRecBuf[CONSOLE_REC_BUF_MAX];

//���й���
QueueStruct stconsoleQueue;
//�������
CONSOLE_USART_STRUCT stConsoleDev;//�ն��豸������
//�����б�
CONSOLE_NAMETAB_STRUCT stConsoleNameTbl[]=
{
	{(void*)show1__,"show1__"},
	{(void*)led_set__,"led_set__"},
	{(void*)play__,"paly__"},
	//		{(void*)led_set__,"led_set__"},
	
};

//ϵͳ�����б�
const char*consoleSysCmeTbl[]
={
   "ls", //1cmd
   "list",//2cmd
   "runtime",//3cmd runtime on runtime off
};
/*
����:�ַ�������
����:
     pScr      Դ�ַ���
     pDest     Ŀ���ַ���
     size      ����
*/

static  void consoleMemCopy(unsigned char *pDest, unsigned char *pScr, unsigned short size)
{
    unsigned char *pD=pDest;
	unsigned char *pS=pScr;
    while(size--) *pD++ = *pS++;
}

/*********************************
����:�ַ�����ֵ
*********************************/

static void consoleMemSet(unsigned char *pDest, unsigned char value, unsigned short size)
{
    unsigned char *pD=pDest;
	while(size--) *pD++ = value;
}
/*************************************
����:�ַ����Ƚ�
����:
     pScr      Դ�ַ���
     pDest     Ŀ���ַ���
����ֵ:1,����;0,���
*************************************/

static unsigned char consoleStrCmp(unsigned char *pScr,unsigned char *pDest)
{
   unsigned char *pD=pDest;
   unsigned char *pS=pScr;
   while(*pS==*pD)
   {
      if(*pS=='\0') return 0; 
	  pS++;
	  pD++;
	  	
   }
   return 1;
}
/****************************************
����:��ȡϵͳָ����
����:
     src      Դ�ַ���
     cmdName ��ȡ������
     nlen    ����������
     maxLen  ��󳤶�
����ֵ:0,�ɹ�;����,ʧ��
*****************************************/
static unsigned char consoleGetSysCmdName(unsigned char *src,unsigned char*cmdName,unsigned char 
*nlen,unsigned char maxLen)
{
	*nlen=0;
	while(*src!=' '&&*src!='\0')//�����ڿո��'\0'
	{
		*cmdName++=*src++;
		(*nlen)++;
		if(*nlen>=maxLen) return 1;//����ָ��		
	}
	*cmdName='\0';
	return 0;
}
/****************************************
����:��ȡ�����Ĳ���
����:
     pSrc      Դ�ַ���
     nlen      ����  ����'\0'
     pDest      ��Ų�������
     pnum     �����ĸ���
     parmLenMax   ����������󳤶�
     parmMax   �����������ֵ
����ֵ:0,�ɹ�;����,ʧ��         ��ʽ(argvq,argv2,67868,76)
*******************************************/
unsigned char consoleGetFuncNameParm(unsigned char *pScr, unsigned char nlen, unsigned char *pDest, unsigned char *pnum, unsigned char parmLenMax,unsigned char parmMax)
{
	unsigned char len = 0, i = 0;
	unsigned char num = 0;//��������
	unsigned char *pBuf = pDest;
	unsigned char time=0;//'('���ֵĴ���
	for (len = 0; len<nlen; len++)
	{
		if (*pScr != ','&&*pScr != ')'&&*pScr !='\0')
		{
			pBuf[num*CONSOLE_PARM_LEN_MAX+i] = *pScr++;
			i++;

		}
		else if (*pScr == ',' || *pScr == ')')
		{
			if (*pScr == ')')
			{  
				if (time == 0)
					time = 1;
				else if (time == 1) //��������'(' ���ش���
					return 1;//
			}

			if (i >= parmLenMax) return 1;//��������̫��
			else
			{
				 
				pBuf[num*CONSOLE_PARM_LEN_MAX + i] ='\0';
				i = 0;
				num++;
				if(num>parmMax) return 1;//����̫��
				pScr++;
			}
		}
		else if (*pScr == '\0')
		{
			if (num >= 1) { ; *pnum = num; return 0; }//������һ������
			else return 1;//û��һ������
		}
	}
	return 0;
}


/****************************************
����:��ȡϵͳ������� cmdsys:arg1 argv2 argv3 ...  
����:
     pSrc      Դ�ַ���
     nlen      ����  ����'\0'
     parmLenMax   ����������󳤶�
     parmMax   �����������ֵ
����ֵ:0,�ɹ�;����,ʧ��   
ע��:�������ַ�������ʽ����
*******************************************/
unsigned char consoleGetSysCmdNameParm(unsigned char *pScr, unsigned char nlen,unsigned char parmLenMax,unsigned char parmMax)
{
   unsigned char len=0,i=0;
   unsigned char *pBuf=pScr;
   stConsoleDev.funcParmn=1;//��������
   for(len=0;len<nlen;len++)
   {  
        if(pBuf[len]==' '&&pBuf[len+1]!=' '&&pBuf[len+1]!='\0')//���Ӳ�������
        {
          stConsoleDev.funcParmTbl[stConsoleDev.funcParmn-1][i++]='\0';
          i=0;		  
          stConsoleDev.funcParmn++;//һ������          
          if(stConsoleDev.funcParmn>parmMax)//������������
           return 1;
		}
		if(pBuf[len]!=' '&&pBuf[len]!='\0')
		{
	       stConsoleDev.funcParmTbl[stConsoleDev.funcParmn-1][i++]=pBuf[len];
		   if(i>parmLenMax) 
		   	return 1;//������󳤶�
		}
   }
   return 0;
}

/**********************************************
	����:ִ��ϵͳ����ĺ��� 
	����:��
	����ֵ:��
**********************************************/

static void consoleSysCmdExe(void)
{
	//
	
}

//ϵͳls����
static void consoleSysCmdLs(void)
{
	int i;
	int size=sizeof(consoleSysCmeTbl)/4;
	for(i=0;i<size;i++)
	{
	  CONSOLE_PRINT("%s\r\n",consoleSysCmeTbl[i]);
	}

}
//ϵͳlist����
static void consoleSysCmdList(void)
{
   int i;
   int size=sizeof(stConsoleNameTbl)/sizeof(stConsoleNameTbl[0]);
   for(i=0;i<size;i++)
   {
     CONSOLE_PRINT("%s\r\n",stConsoleNameTbl[i].name);
   }

}
//ϵͳruntime����
static void consoleSysCmdRunTime(void)
{
   unsigned char i;
   for(i=0;i<stConsoleDev.funcParmn;i++)
   {
      printf("%d:%s\r\n",i+1,stConsoleDev.funcParmTbl[i]);
   }
   stConsoleDev.funcParmn=0;
   printf("time..\r\n");
} 



/********************************
����:�ն˳�ʼ��
����:��
����ֵ:��
*********************************/
void consoleInit(void)
{
    stConsoleDev.cmdRecPro=consoleCmdProFunc;
    stConsoleDev.funcExe=consoleFuncExe;
	stConsoleDev.funcNum=sizeof(stConsoleNameTbl)/sizeof(stConsoleNameTbl[0]);
	stConsoleDev.funs=stConsoleNameTbl;
	Queue_Init(&stconsoleQueue,consoleRecBuf,CONSOLE_REC_BUF_MAX);//��ʼ�����ն���
	  
}

/*********************************
����:ִ����Ӧ����ĺ��� 
����:��
����ֵ:��
**********************************/
void consoleFuncExe(void)
{
    unsigned int ret;//����ֵ    
    unsigned char id=stConsoleDev.funcId;//ִ�к�����id

    switch(stConsoleDev.cmdType)
   {
	case CONSOLE_FUNC_CMD://��������
		{
			
			printf("stConsoleDev.funcNum:%d\r\n",stConsoleDev.funcNum);
		    if(id>stConsoleDev.funcNum-1) return;//����id
		    //ִ�к����Ĳ�������
		    if(stConsoleDev.runTimeFlag==1) 
			stConsoleDev.runTinme=CONSOLE_TIME_GET;//����ʱ��	
			switch(stConsoleDev.funcParmn)
		    {
				case 0://�޲���
					printf("0\r\n");
					ret=(*(unsigned int (*)())stConsoleDev.funs[id].func)();
					break;
				case 1://һ������
						printf("1\r\n");
					ret=(*(unsigned int(*)())stConsoleDev.funs[id].func)(stConsoleDev.funcParmTbl[0][0]);
					break;
				case 2://��������
				    ret=(*(unsigned  int(*)())stConsoleDev.funs[id].func)(stConsoleDev.funcParmTbl[0][0],stConsoleDev.funcParmTbl[1][0]);
					break;
				case 3:
					ret=(*(unsigned int(*)())stConsoleDev.funs[id].func)(stConsoleDev.funcParmTbl[0],stConsoleDev.funcParmTbl[1],stConsoleDev.funcParmTbl[2]);
					break;
				case 4:
					ret=(*(unsigned int(*)())stConsoleDev.funs[id].func)(stConsoleDev.funcParmTbl[0],stConsoleDev.funcParmTbl[1],stConsoleDev.funcParmTbl[2],stConsoleDev.funcParmTbl[3]);
					break;
				default :break;		
			}
		    if(stConsoleDev.runTimeFlag==1) 
		    stConsoleDev.runTinme=CONSOLE_TIME_GET-stConsoleDev.runTinme;//����ʱ��	
		  
        } 
	    break;
	case CONSOLE_SYS_CMD://ϵͳ����
		{
		  switch(stConsoleDev.sysCmd)
			{
				case CONSOLE_SYS_LS_CMD:	
					consoleSysCmdLs();//ls����
					break;
				case CONSOLE_SYS_LIST_CMD:
					consoleSysCmdList();//list����
					break;
				case 	CONSOLE_SYS_RUNTIME_CMD:
					consoleSysCmdRunTime();//runtime ����
					break;
				default:break;		
		   }			
		}
		break;
	default:break;//��������
    }
}

/*********************************
����:���������������
����:pStr ����Ľ�����������ָ��
     nlen ���ݵĳ��� ����'\0'
����ֵ:CONSOLE_REC_PRO_OK 
       CONSOLE_REC_PRO_ERROR
***********************************/
CONSOLE_REC_TYPE_ENUM consoleCmdProFunc(unsigned char *pStr,unsigned short nlen)
{
   //1.ϵͳ���� 2.��������  3.����
   unsigned char cmdtype;//��⵽�ַ�����1':', 2' '
   unsigned short i;
   unsigned short len=0;//��¼����
   unsigned char *pBuf;
   //ϵͳ����
     pBuf=pStr;
     for(i=0;i<nlen-1;i++)
     {
		if(*pBuf!='('&&*pBuf!=' '&&*pBuf!=':')//
		{ 
		   	stConsoleDev.funcName[len++]=*pBuf++;//ȡ����
		}
		else 
		{
	        if(*pBuf=='(')
			{//��������
			   stConsoleDev.cmdType=CONSOLE_FUNC_CMD;
	           break;
				 
			}
			else if(*pBuf==':')
			{
			   cmdtype=1;
			   stConsoleDev.cmdType=CONSOLE_SYS_CMD;
	           break;
			}
			else if(*pBuf==' ')
			{
			  cmdtype=2;
			  stConsoleDev.cmdType=CONSOLE_SYS_CMD;
	          break;
			}
	   }
    }
    if(i==nlen-1) //û���ҵ� '(' ' ' ':' //��Ϊϵͳ����
    {
        for(i=0;i<(sizeof(consoleSysCmeTbl)/4);i++)
		{
		   stConsoleDev.funcName[len++]='\0';
           if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)consoleSysCmeTbl[i])==0)//ϵͳ����鵽
           {
              stConsoleDev.cmdType=CONSOLE_SYS_CMD;//ϵͳ����
              stConsoleDev.sysCmd=(unsigned char)i;//����ϵͳ����id
              return CONSOLE_REC_PRO_OK;//����ok
		   }
 
		}
	 }
	else//�ҵ�'(' ':' ' '�ҵ�ϵͳ������ߺ�������
	{  
	      stConsoleDev.funcName[len++]='\0';
	      stConsoleDev.funcNameLen=len;//��������ϵͳ����� ����'\0'
          switch(stConsoleDev.cmdType)
          {
            case CONSOLE_SYS_CMD://' ' ':'
            if(cmdtype==1) //':' ��������ϵͳ���� ��������
            {
               pBuf++;//����':'  �������ַ����Ķ���ʽ
               if(consoleGetSysCmdNameParm(pBuf,nlen-len,CONSOLE_PARM_LEN_MAX,CONSOLE_PARM_NUM_MAX))
               { 
                   stConsoleDev.funcParmn=0;//��������
				   return CONSOLE_REC_PRO_ERROR;
			   }
			}
            else if(cmdtype==2) //' ' �ж��Ƿ�һֱ����' '
			{
			  for(i=len;i<nlen;i++) 
			  {
				  if(*pBuf!=' ') break;
		          pBuf++;len++;									
			   }
			  if(len!=nlen)//����һֱ����' '
			  {  
				  return CONSOLE_REC_PRO_ERROR;
			  }									  								  
			}
		    for(i=0;i<(sizeof(consoleSysCmeTbl)/4);i++)
		    {
               if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)consoleSysCmeTbl[i])==0)//ϵͳ����鵽
	           {
	              stConsoleDev.cmdType=CONSOLE_SYS_CMD;//ϵͳ����
	              stConsoleDev.sysCmd=(unsigned char)i;//����ϵͳ����id
							   
	              return CONSOLE_REC_PRO_OK;//����ok
			   }
	        }  
			break;
			case CONSOLE_FUNC_CMD:
			//���Ҷ�Ӧ��������
			for(i=0;i<(sizeof(stConsoleNameTbl)/8);i++) //ѭ����ѯ������
			{
				   
				if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)stConsoleNameTbl[i].name)==0)//��������鵽
				{
					stConsoleDev.funcId=(unsigned char)i;//����ϵͳ����id
				    pBuf++;len++;
				    if(*pBuf==')')  //�޲���  func()
					{
				      pBuf++;len++;
					  for(i=len;i<nlen;i++)
					  {
					    if(*pBuf!=' '&&*pBuf!='\0') break;//���� func()  �е�')'�����Ƿ��ǿո�Ż��߽�����
						   pBuf++;len++;
					  }
					  if(len!=nlen)//
					  {
                         return CONSOLE_REC_PRO_ERROR;//
					  }
					  else
					  {
                           stConsoleDev.funcParmn=0;//�޲���
						   return CONSOLE_REC_PRO_OK;
					  }
					 }
					//������һ������
				    else  if(0==consoleGetFuncNameParm(pBuf,nlen-len,(unsigned char*)stConsoleDev.funcParmTbl,&stConsoleDev.funcParmn,CONSOLE_PARM_LEN_MAX,CONSOLE_PARM_NUM_MAX)) //�ɹ�
				    {
					 
					   return CONSOLE_REC_PRO_OK;
				    }		 	 
				  }
			 }
	 
			 break;
			 default: 
			   return CONSOLE_REC_PRO_ERROR;
			   break;	

		  }
    
	}
	return CONSOLE_REC_PRO_ERROR;
   
}
/************************************
����:�ն˴������
����:��
����ֵ:��
*************************************/
void consoleProEntry(void)
{   
    CONSOLE_REC_TYPE_ENUM ret;
    unsigned short len;
    len=Queue_Out(&stconsoleQueue,&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen],CONSOLE_FRAME_DATA_LEN_MAX-stConsoleDev.frameDataBuflen);
	  if(len)//������
	  {   
			
       stConsoleDev.frameDataBuflen+=len;//�������ݵĳ���
       if(stConsoleDev.frameDataBuflen==CONSOLE_FRAME_DATA_LEN_MAX)//�ﵽ��󳤶�
       {
  		  
	   }
       else if(stConsoleDev.frameDataBuflen>=2&&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-2]=='\r'&&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-1]=='\n')//���յ�һ֡����
       {
       
			 stConsoleDev.frameDataBuflen-=1;  //
			 stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-1]='\0';//������
			 ret=stConsoleDev.cmdRecPro(stConsoleDev.frameDataBuf,stConsoleDev.frameDataBuflen);//����һ������֡
			 if(CONSOLE_REC_PRO_OK==ret)//�����ɹ�
			 {
				stConsoleDev.funcExe();//ִ������
			 }
			 stConsoleDev.frameDataBuflen=0;//������ջ���
	   }
	}
	//1.��ȡһ֡����
	   //���� or ����
	//2.���ô��ڴ�����
}














