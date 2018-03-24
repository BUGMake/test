
#include "terminal.h"
#include "console_cmd.h"

//��У��
#include "string.h"
//#include "math.h"
#include "usart.h"	  
#include "pwm.h"
#include "CMT211xA.h"



/*
˵������Tab�����԰����е������ӡ������Ҳ���Բ�ȫ����
*/
static const char *str_reboot = "reboot";
static const char str_echo[]="echo";
static const char str_cat[]="cat";


//��У��
static const char str_adc_open[]="adc_open";
static const char str_adc_close[]="adc_close";
static const char str_set_pwm[]="set_pwm";
static const char str_set_touzi[]="set_touzi";
static const char str_set_on_off_test[]="set_on_off_test";


//��У��
static char adc_test=0;
char console_cmd_adc(void)
{
	static char cmd_adc=0;
	cmd_adc = adc_test;
	return cmd_adc;
}


//static int strcmp_t(char *str1,char *str2)
//{
//	char *end;
//	int len1;
//	int len2;
//	len1 = sz_strlen(str1);
//	len2 = sz_strlen(str2);
//	if( len1 != len2)
//		return -1;
//	end = str1 + len1;
//	while(str1 < end)
//	{
//		if(*str1 != *str2)
//			break;
//		str1++;
//		str2++;
//	}
//	if(str1 == end)
//		return 0;
//	else
//		return -1;
//}

//�ο���x2_Transponder��
static int console_do_cmd(int argc,char *argv[])//�������
{
//	char ch;
//	int len;
	int i;
	
	if(argc==0)//argc >= 1
		return -1;
	
	for(i=0;i<argc;i++)
	{
		printf("argc%d=\"%s\"\r\n",i,argv[i]);
	}

	if(strcmp((char*)str_echo,argv[0])==0)
	{
		console_cmd_echo(argc-1,&argv[1]);//дһ����������Ӧ�Ľڵ�
		printf("console do cmd = echo\r\n");
	}
	else if(strcmp((char*)str_cat,argv[0])==0)
	{
		console_cmd_cat(argc-1,&argv[1]);//��һ���ڵ������
		printf("console do cmd = cat\r\n");
	}
	else if(strcmp((char*)str_reboot,argv[0])==0)
	{
		printf("system reboot is Success!!!\r\n");
		//systick_delay(100);
		//sys_reboot();
	}
	
	
	//��У��
	else if(strcmp((char*)str_adc_open,argv[0])==0)
	{
		printf("adc open!!!\r\n");
		adc_test=1;
	}
	else if(strcmp((char*)str_adc_close,argv[0])==0)
	{
		printf("adc close!!!\r\n");
		adc_test=0;
	}
	else if(strcmp((char*)str_set_pwm,argv[0])==0)//����PWMƵ�ʣ�ռ�ձȹ̶�50%��= set_pwm -f 500
	{
		if(argc<2)//argc >= 1
		{
			printf("error -> str_set_pwm -> argc = 1 !!!\r\n");
			printf("correct input -> set_pwm -f 500 !!!\r\n");
		}
		else if(strcmp("-f",argv[1])==0)//argc >= 2
		{
			int fre_pwm = sz_strtoul(argv[2],0,10);//ASCLL��ת10����
			printf("set_pwm fre = 72000/(%d+1) = %dKhz\r\n",fre_pwm,72000/(fre_pwm+1));
			if(fre_pwm>0 && fre_pwm<30000) TIM1_SET_PWM(fre_pwm,0);			//PWM�����PA8������Ƶ��PWMƵ��=72000/(665+1)=108Khz��PWM��ռ�ձ� = 50%
			else 
			{
				printf("error -> str_set_pwm -> argv[2]=%d overflow(0-30000)!!!\r\n",fre_pwm);
				printf("correct input -> set_pwm -f 500 !!!\r\n");
			}
		}
		else//argc >= 3
		{
			printf("error -> str_set_pwm -> argv[2]!='-f' !!!\r\n");
			printf("correct input -> set_pwm -f 500 !!!\r\n");
		}
	}
	else if(strcmp((char*)str_set_touzi,argv[0])==0)//���ô�ɫ���� = set_touzi -t 22
	{
		if(argc<2)//argc >= 1
		{
			printf("error -> str_set_touzi -> argc = 1 !!!\r\n");
			printf("correct input -> set_touzi -t 22 !!!\r\n");
		}
		else if(strcmp("-t",argv[1])==0)//argc >= 2
		{
			int touzi_num = sz_strtoul(argv[2],0,10);//ASCLL��ת10����
			printf("set_touzi test number = %d\r\n",touzi_num);
			if(touzi_num>=0x11 && touzi_num<=0x66) control_work(touzi_num);//��ɫ���ԣ�17-102��0x11-0x66��
			else 
			{
				printf("error -> str_set_touzi -> argv[2]=%d overflow(17-102)(0x11-0x66)!!!\r\n",touzi_num);
				printf("correct input -> set_touzi -t 22 !!!\r\n");
			}
		}
		else//argc >= 3
		{
			printf("error -> str_set_touzi -> argv[2]!='-t' !!!\r\n");
			printf("correct input -> set_touzi -t 22 !!!\r\n");
		}
	}	
	//str_set_on_off_test
	else if(strcmp((char*)str_set_on_off_test,argv[0])==0)//���ÿ���PWM��ʱ�䳤�ȣ�ռ�ձȹ̶�50%��= set_on_off_test -f 50 50 50 50
	{
		if(argc<5)//argc >= 4
		{
			printf("error -> set_on_off_test -> argc < 5 !!!\r\n");
			printf("correct input -> set_on_off_test -f 50 50 50 50 !!!\r\n");
		}
		else if(strcmp("-f",argv[1])==0)//argc >= 5
		{
			int set_time_on = sz_strtoul(argv[2],0,10);//ASCLL��ת10����
			int set_time_off = sz_strtoul(argv[3],0,10);//ASCLL��ת10����
			int set_time_on2 = sz_strtoul(argv[4],0,10);//ASCLL��ת10����
			int set_time_off2 = sz_strtoul(argv[5],0,10);//ASCLL��ת10����
			
			printf("set_time_on = %dus\r\n",set_time_on);
			printf("set_time_off = %dus\r\n",set_time_off);
			printf("set_time_on2 = %dus\r\n",set_time_on2);
			printf("set_time_off2 = %dus\r\n",set_time_off2);
			
			if(set_time_on>0 && set_time_on<30000 && set_time_off>0 && set_time_off<30000 && set_time_on2>0 && set_time_on2<30000 && set_time_off2>0 && set_time_off2<30000) 
				TIM1_SET_on_off(set_time_on,set_time_off,set_time_on2,set_time_off2);	
			else 
			{
				printf("error -> set_on_off_test -> argv[2]=%d overflow(0-30000) or argv[3]=%d overflow(0-30000) or argv[4]=%d overflow(0-30000) or argv[5]=%d overflow(0-30000)!!!\r\n",set_time_on,set_time_off,set_time_on2,set_time_off2);
				printf("correct input -> set_on_off_test -f 50 50 50 50 !!!\r\n");
			}
		}
		else//argc >= 5
		{
			printf("error -> set_on_off_test -> argv[2]!='-f' !!!\r\n");
			printf("correct input -> set_on_off_test -f 50 50 50 50 !!!\r\n");
		}
	}
	
	
	return 0;
}

int register_cmds(void)
{
	register_cmd((char*)str_echo,strlen(str_echo));
	register_cmd((char*)str_cat,strlen(str_cat));

	register_console_handl_data(console_do_cmd);//����������Ļص�����

	register_cmd((char*)str_reboot,strlen(str_reboot));//reboot
	
	
	//��У��
	register_cmd((char*)str_adc_open,strlen(str_adc_open));
	register_cmd((char*)str_adc_close,strlen(str_adc_close));
	register_cmd((char*)str_set_pwm,strlen(str_set_pwm));
	register_cmd((char*)str_set_touzi,strlen(str_set_touzi));
	register_cmd((char*)str_set_on_off_test,strlen(str_set_on_off_test));
	
	
	return 0;
}
