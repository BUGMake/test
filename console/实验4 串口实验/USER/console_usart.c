#include "console_usart.h"
#include "Queue.h"
#include "usart.h"
/*************************************************/
//             终端管理
//命令使用: CMD  argv1 argv2 ...  回车
//          ADD
//          DEL
//          LS
//注意:暂时只能添加的函数 name(argv1,argv2,...)
//
/************************************************/


 void consoleFuncExe(void);

 
 CONSOLE_REC_TYPE_ENUM consoleCmdProFunc(unsigned char *pStr,unsigned short nlen);

/*
系统命令
ls   //列出命令
list //列出函数名
runtime on//开时间
runtime off//关时间

//
函数命令
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

//队列管理
QueueStruct stconsoleQueue;
//定义变量
CONSOLE_USART_STRUCT stConsoleDev;//终端设备管理器
//函数列表
CONSOLE_NAMETAB_STRUCT stConsoleNameTbl[]=
{
	{(void*)show1__,"show1__"},
	{(void*)led_set__,"led_set__"},
	{(void*)play__,"paly__"},
	//		{(void*)led_set__,"led_set__"},
	
};

//系统命令列表
const char*consoleSysCmeTbl[]
={
   "ls", //1cmd
   "list",//2cmd
   "runtime",//3cmd runtime on runtime off
};
/*
函数:字符串复制
参数:
     pScr      源字符串
     pDest     目标字符串
     size      长度
*/

static  void consoleMemCopy(unsigned char *pDest, unsigned char *pScr, unsigned short size)
{
    unsigned char *pD=pDest;
	unsigned char *pS=pScr;
    while(size--) *pD++ = *pS++;
}

/*********************************
函数:字符串赋值
*********************************/

static void consoleMemSet(unsigned char *pDest, unsigned char value, unsigned short size)
{
    unsigned char *pD=pDest;
	while(size--) *pD++ = value;
}
/*************************************
函数:字符串比较
参数:
     pScr      源字符串
     pDest     目标字符串
返回值:1,不等;0,相等
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
函数:获取系统指令名
参数:
     src      源字符串
     cmdName 获取命令名
     nlen    命令名长度
     maxLen  最大长度
返回值:0,成功;其他,失败
*****************************************/
static unsigned char consoleGetSysCmdName(unsigned char *src,unsigned char*cmdName,unsigned char 
*nlen,unsigned char maxLen)
{
	*nlen=0;
	while(*src!=' '&&*src!='\0')//不等于空格和'\0'
	{
		*cmdName++=*src++;
		(*nlen)++;
		if(*nlen>=maxLen) return 1;//错误指令		
	}
	*cmdName='\0';
	return 0;
}
/****************************************
函数:获取函数的参数
参数:
     pSrc      源字符串
     nlen      长度  包含'\0'
     pDest      存放参数函数
     pnum     参数的个数
     parmLenMax   单个参数最大长度
     parmMax   参数个数最大值
返回值:0,成功;其他,失败         形式(argvq,argv2,67868,76)
*******************************************/
unsigned char consoleGetFuncNameParm(unsigned char *pScr, unsigned char nlen, unsigned char *pDest, unsigned char *pnum, unsigned char parmLenMax,unsigned char parmMax)
{
	unsigned char len = 0, i = 0;
	unsigned char num = 0;//参数个数
	unsigned char *pBuf = pDest;
	unsigned char time=0;//'('出现的次数
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
				else if (time == 1) //出现两次'(' 返回错误
					return 1;//
			}

			if (i >= parmLenMax) return 1;//参数长度太长
			else
			{
				 
				pBuf[num*CONSOLE_PARM_LEN_MAX + i] ='\0';
				i = 0;
				num++;
				if(num>parmMax) return 1;//参数太多
				pScr++;
			}
		}
		else if (*pScr == '\0')
		{
			if (num >= 1) { ; *pnum = num; return 0; }//至少有一个参数
			else return 1;//没有一个参数
		}
	}
	return 0;
}


/****************************************
函数:获取系统命令参数 cmdsys:arg1 argv2 argv3 ...  
参数:
     pSrc      源字符串
     nlen      长度  包含'\0'
     parmLenMax   单个参数最大长度
     parmMax   参数个数最大值
返回值:0,成功;其他,失败   
注意:参数以字符串的形式保存
*******************************************/
unsigned char consoleGetSysCmdNameParm(unsigned char *pScr, unsigned char nlen,unsigned char parmLenMax,unsigned char parmMax)
{
   unsigned char len=0,i=0;
   unsigned char *pBuf=pScr;
   stConsoleDev.funcParmn=1;//参数个数
   for(len=0;len<nlen;len++)
   {  
        if(pBuf[len]==' '&&pBuf[len+1]!=' '&&pBuf[len+1]!='\0')//增加参数个数
        {
          stConsoleDev.funcParmTbl[stConsoleDev.funcParmn-1][i++]='\0';
          i=0;		  
          stConsoleDev.funcParmn++;//一个参数          
          if(stConsoleDev.funcParmn>parmMax)//参数个数过多
           return 1;
		}
		if(pBuf[len]!=' '&&pBuf[len]!='\0')
		{
	       stConsoleDev.funcParmTbl[stConsoleDev.funcParmn-1][i++]=pBuf[len];
		   if(i>parmLenMax) 
		   	return 1;//大于最大长度
		}
   }
   return 0;
}

/**********************************************
	函数:执行系统命令的函数 
	参数:无
	返回值:无
**********************************************/

static void consoleSysCmdExe(void)
{
	//
	
}

//系统ls命令
static void consoleSysCmdLs(void)
{
	int i;
	int size=sizeof(consoleSysCmeTbl)/4;
	for(i=0;i<size;i++)
	{
	  CONSOLE_PRINT("%s\r\n",consoleSysCmeTbl[i]);
	}

}
//系统list命令
static void consoleSysCmdList(void)
{
   int i;
   int size=sizeof(stConsoleNameTbl)/sizeof(stConsoleNameTbl[0]);
   for(i=0;i<size;i++)
   {
     CONSOLE_PRINT("%s\r\n",stConsoleNameTbl[i].name);
   }

}
//系统runtime命令
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
函数:终端初始化
参数:无
返回值:无
*********************************/
void consoleInit(void)
{
    stConsoleDev.cmdRecPro=consoleCmdProFunc;
    stConsoleDev.funcExe=consoleFuncExe;
	stConsoleDev.funcNum=sizeof(stConsoleNameTbl)/sizeof(stConsoleNameTbl[0]);
	stConsoleDev.funs=stConsoleNameTbl;
	Queue_Init(&stconsoleQueue,consoleRecBuf,CONSOLE_REC_BUF_MAX);//初始化接收队列
	  
}

/*********************************
函数:执行相应命令的函数 
参数:无
返回值:无
**********************************/
void consoleFuncExe(void)
{
    unsigned int ret;//返回值    
    unsigned char id=stConsoleDev.funcId;//执行函数的id

    switch(stConsoleDev.cmdType)
   {
	case CONSOLE_FUNC_CMD://函数命令
		{
			
			printf("stConsoleDev.funcNum:%d\r\n",stConsoleDev.funcNum);
		    if(id>stConsoleDev.funcNum-1) return;//参数id
		    //执行函数的参数个数
		    if(stConsoleDev.runTimeFlag==1) 
			stConsoleDev.runTinme=CONSOLE_TIME_GET;//开启时间	
			switch(stConsoleDev.funcParmn)
		    {
				case 0://无参数
					printf("0\r\n");
					ret=(*(unsigned int (*)())stConsoleDev.funs[id].func)();
					break;
				case 1://一个参数
						printf("1\r\n");
					ret=(*(unsigned int(*)())stConsoleDev.funs[id].func)(stConsoleDev.funcParmTbl[0][0]);
					break;
				case 2://两个参数
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
		    stConsoleDev.runTinme=CONSOLE_TIME_GET-stConsoleDev.runTinme;//计算时间	
		  
        } 
	    break;
	case CONSOLE_SYS_CMD://系统命令
		{
		  switch(stConsoleDev.sysCmd)
			{
				case CONSOLE_SYS_LS_CMD:	
					consoleSysCmdLs();//ls命令
					break;
				case CONSOLE_SYS_LIST_CMD:
					consoleSysCmdList();//list命令
					break;
				case 	CONSOLE_SYS_RUNTIME_CMD:
					consoleSysCmdRunTime();//runtime 命令
					break;
				default:break;		
		   }			
		}
		break;
	default:break;//其他命令
    }
}

/*********************************
函数:串口命令解析函数
参数:pStr 输入的解析数据数据指针
     nlen 数据的长度 包含'\0'
返回值:CONSOLE_REC_PRO_OK 
       CONSOLE_REC_PRO_ERROR
***********************************/
CONSOLE_REC_TYPE_ENUM consoleCmdProFunc(unsigned char *pStr,unsigned short nlen)
{
   //1.系统命令 2.函数命令  3.错误
   unsigned char cmdtype;//检测到字符类型1':', 2' '
   unsigned short i;
   unsigned short len=0;//记录长度
   unsigned char *pBuf;
   //系统命令
     pBuf=pStr;
     for(i=0;i<nlen-1;i++)
     {
		if(*pBuf!='('&&*pBuf!=' '&&*pBuf!=':')//
		{ 
		   	stConsoleDev.funcName[len++]=*pBuf++;//取数据
		}
		else 
		{
	        if(*pBuf=='(')
			{//函数命令
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
    if(i==nlen-1) //没有找到 '(' ' ' ':' //视为系统命令
    {
        for(i=0;i<(sizeof(consoleSysCmeTbl)/4);i++)
		{
		   stConsoleDev.funcName[len++]='\0';
           if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)consoleSysCmeTbl[i])==0)//系统命令查到
           {
              stConsoleDev.cmdType=CONSOLE_SYS_CMD;//系统命令
              stConsoleDev.sysCmd=(unsigned char)i;//保存系统命令id
              return CONSOLE_REC_PRO_OK;//返回ok
		   }
 
		}
	 }
	else//找到'(' ':' ' '找到系统命令或者函数命令
	{  
	      stConsoleDev.funcName[len++]='\0';
	      stConsoleDev.funcNameLen=len;//函数名或系统命令长度 包含'\0'
          switch(stConsoleDev.cmdType)
          {
            case CONSOLE_SYS_CMD://' ' ':'
            if(cmdtype==1) //':' 带参数的系统命令 解析参数
            {
               pBuf++;//跳过':'  参数是字符串的而形式
               if(consoleGetSysCmdNameParm(pBuf,nlen-len,CONSOLE_PARM_LEN_MAX,CONSOLE_PARM_NUM_MAX))
               { 
                   stConsoleDev.funcParmn=0;//参数错误
				   return CONSOLE_REC_PRO_ERROR;
			   }
			}
            else if(cmdtype==2) //' ' 判断是否一直都是' '
			{
			  for(i=len;i<nlen;i++) 
			  {
				  if(*pBuf!=' ') break;
		          pBuf++;len++;									
			   }
			  if(len!=nlen)//不是一直都是' '
			  {  
				  return CONSOLE_REC_PRO_ERROR;
			  }									  								  
			}
		    for(i=0;i<(sizeof(consoleSysCmeTbl)/4);i++)
		    {
               if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)consoleSysCmeTbl[i])==0)//系统命令查到
	           {
	              stConsoleDev.cmdType=CONSOLE_SYS_CMD;//系统命令
	              stConsoleDev.sysCmd=(unsigned char)i;//保存系统命令id
							   
	              return CONSOLE_REC_PRO_OK;//返回ok
			   }
	        }  
			break;
			case CONSOLE_FUNC_CMD:
			//查找对应函数命令
			for(i=0;i<(sizeof(stConsoleNameTbl)/8);i++) //循环查询函数名
			{
				   
				if(consoleStrCmp(stConsoleDev.funcName,(unsigned char *)stConsoleNameTbl[i].name)==0)//函数命令查到
				{
					stConsoleDev.funcId=(unsigned char)i;//保存系统命令id
				    pBuf++;len++;
				    if(*pBuf==')')  //无参数  func()
					{
				      pBuf++;len++;
					  for(i=len;i<nlen;i++)
					  {
					    if(*pBuf!=' '&&*pBuf!='\0') break;//查找 func()  中的')'后面是否是空格号或者结束符
						   pBuf++;len++;
					  }
					  if(len!=nlen)//
					  {
                         return CONSOLE_REC_PRO_ERROR;//
					  }
					  else
					  {
                           stConsoleDev.funcParmn=0;//无参数
						   return CONSOLE_REC_PRO_OK;
					  }
					 }
					//至少有一个参数
				    else  if(0==consoleGetFuncNameParm(pBuf,nlen-len,(unsigned char*)stConsoleDev.funcParmTbl,&stConsoleDev.funcParmn,CONSOLE_PARM_LEN_MAX,CONSOLE_PARM_NUM_MAX)) //成功
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
函数:终端处理入口
参数:无
返回值:无
*************************************/
void consoleProEntry(void)
{   
    CONSOLE_REC_TYPE_ENUM ret;
    unsigned short len;
    len=Queue_Out(&stconsoleQueue,&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen],CONSOLE_FRAME_DATA_LEN_MAX-stConsoleDev.frameDataBuflen);
	  if(len)//有数据
	  {   
			
       stConsoleDev.frameDataBuflen+=len;//保存数据的长度
       if(stConsoleDev.frameDataBuflen==CONSOLE_FRAME_DATA_LEN_MAX)//达到最大长度
       {
  		  
	   }
       else if(stConsoleDev.frameDataBuflen>=2&&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-2]=='\r'&&stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-1]=='\n')//接收到一帧数据
       {
       
			 stConsoleDev.frameDataBuflen-=1;  //
			 stConsoleDev.frameDataBuf[stConsoleDev.frameDataBuflen-1]='\0';//结束符
			 ret=stConsoleDev.cmdRecPro(stConsoleDev.frameDataBuf,stConsoleDev.frameDataBuflen);//解析一个数据帧
			 if(CONSOLE_REC_PRO_OK==ret)//解析成功
			 {
				stConsoleDev.funcExe();//执行命令
			 }
			 stConsoleDev.frameDataBuflen=0;//清除接收缓存
	   }
	}
	//1.获取一帧数据
	   //命令 or 函数
	//2.调用串口处理函数
}














