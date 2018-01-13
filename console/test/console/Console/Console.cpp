// Console.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
static unsigned char consoleStrCmp(const  char *pScr, const  char *pDest)
{
	while (*pScr == *pDest)
	{
		if (*pScr == '\0') return 0;
		pScr++;
		pDest++;

	}
	return 1;
}
void con(int i)
{
	printf("con:%d\r\n",i);

}
void show1(void)
{
	printf("hhh");
}
void show2(void)
{
	printf("hhh");
}
char  led_set(char sta)
{
	return  0;
}
#define CONSOLE_PARM_LEN_MAX  15 //参数的长度  
unsigned char consoleGetFuncNameParm(unsigned char *pScr, unsigned char nlen, unsigned char *pDest, unsigned char *pnum, unsigned char parmLenMax, unsigned char parmMax)
{
	unsigned char len = 0, i = 0;
	unsigned char num = 0;//²ÎÊý¸öÊý
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
				else if (time == 1)
					return 1;//多次出现'('
			}

			if (i >= parmLenMax) return 1;//²ÎÊý³¤¶ÈÌ«³¤
			else
			{
				 
				pBuf[num*CONSOLE_PARM_LEN_MAX + i] ='\0';
				i = 0;
				num++;
				if (num>parmMax) return 1;
				pScr++;
			}
		}
		else if (*pScr == '\0')
		{
			if (num >= 1) { ; *pnum = num; return 0; }//ÖÁÉÙÓÐÒ»¸ö²ÎÊý
			else return 1;//Ã»ÓÐÒ»¸ö²ÎÊý
		}
	}
	return 0;
}

//函数成员 
typedef struct{
	void *func;//函数指针
	const  char *name;//函数名(查找字符串执行相应得函数功能)
}CONSOLE_NAMETAB_STRUCT;

CONSOLE_NAMETAB_STRUCT stConsoleNameTbl[] = 
	{
		{ (void*)show1, "void show1(void)" },
		{ (void*)led_set, "u8 void show1(u8 sta)", }

	};
const char *consoleSysCmeTbl[]
= {
	"ls1",
	"list",
	"runtime",//runtime on runtime off
};
const char *name = "1)";
typedef int(*type1)();
const  char *pp = "con";
unsigned char buf[256];
unsigned char num;
int _tmain(int argc, _TCHAR* argv[])
{
	unsigned char i;
	unsigned char ret;
	//void *temp1 = (void *)con;
	//type1 temp2;
	//(*(char(*)(char))temp1)(12);
	//temp2=(type1 )temp1;
	///temp2 = con;
    // (*temp2)(12);
	//printf("dddd %d\r\n", temp2(12));

	for (i = 0; i < sizeof(consoleSysCmeTbl) / sizeof(consoleSysCmeTbl[0]);i++)
		printf("len:%s\r\n", consoleSysCmeTbl[i]);
	ret = consoleStrCmp("12343","1234");
	printf("ret:%d\r\n",ret);


	if (!consoleGetFuncNameParm((unsigned char *)name, strlen(name) + 1, buf, &num, 10,5))
	{
		printf("num:%d\r\n", num);
		for (i = 0; i < num; i++)
		{
			printf("i:%d\r\n%s\r\n", i,&buf[i*CONSOLE_PARM_LEN_MAX]);
		}
		//printf("strlen(name):%d\r\n", strlen(name)+1);
		//printf("num:%d\r\n", num);
		//printf("argv:\r\n%s", buf);
	}
	return 0;
}

