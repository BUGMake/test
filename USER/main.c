#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "pwm.h"
#include "adc.h"
#include "stmflash.h"   
#include "usmart.h"

#include "CMT211xA.h"
#include "CC110x.h"
#include "encode_decode.h"
#include "pro.h"
#include "sz_libc.h"
#include "terminal.h"
#include "stm_console.h"
#if 0
//普通打色
static void normal_work(void)
{	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  	
	//USART_printf = 1;/*选择串口1作为printf输出*/
	printf("\n\rno command!\n\r");
	
	/*开转盘马达*/
	MOTOR_OUT_ON ();
	
	//延时3秒		
	delay_ms(1000);/*常规转盘*/	
	delay_ms(1000);/*常规转盘*/	
	delay_ms(1000);/*常规转盘*/	
	//注意：STM32对72M条件下,nms<=1864 
	
	/*关转盘马达*/
	MOTOR_OUT_OFF ();	
	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  	
	printf("\n\rnormal_work = no command!\n\r");
}

//控制打色
static void power_work(u8 touzi_num)
{
  static u16 crc;
  static u8 by[2];
  static u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  
	printf("\n\rtest num = %2X\n\r",touzi_num);/*输出2位16进制数*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*关无线充电*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  				

	/*发送点数命令*/
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = touzi_num;//骰子A：低4位；骰子B：高4位
	//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
	
	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//校验码低8位在前
	str[CMT211xA_DATALength - 1] = by[1];

	//连续发3次，无应答
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//发射前编码
	CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
	delay_ms(500);
//			delay_ms(200);
//			CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
//			delay_ms(200);
//			CMT211xA_TxPacket();//发射编码数据包 ≈ 70ms
	/*注意：必须先在CMT211xA_TxPacket();测试vDelayUs(1000);的实际延时*/
	
				
	/*开转盘马达*/
	MOTOR_OUT_ON ();
	delay_ms(1000);/*常规转盘*/	
//DC280V_Control_ON ();//异常测试	
	delay_ms(1000);/*常规转盘*/	
	//注意：STM32对72M条件下,nms<=1864 
	
//	/*开大线圈*/
//	DC280V_Control_ON ();
	delay_ms(100);/*常规转盘*/	
	/*注意：验证 = S1转动太快的情况下，可以接收到无线命令吗？*/
	
	/*关转盘马达*/
	MOTOR_OUT_OFF ();
	
	/*关闭微型马达后的延时*/
	delay_ms(500);/*常规转盘*/

	/*开大线圈*/
	DC280V_Control_ON ();

	delay_ms(500);/*常规转盘*/
	//delay_ms(800);/*常规转盘*/
	//delay_ms(1000);/*常规转盘*/
//delay_ms(1000);//异常测试		
	DC280V_Control_OFF ();//关闭大线圈控制
	//注意：要调试好转盘停止和线圈磁场通断的时序逻辑关系 -- 很关键！
				
	/*开无线充电*/
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能
}

int read_sys_mode(void)
{
	int mode=0;
//	int res;
//	
//	res = (int)GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2);
//	if(res == 1)
//	{
//		mode |= (0x01<<2);
//	}

//	res = (int)GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12);
//	if(res == 1)
//	{
//		mode |= (0x01<<1);
//	}

//	res = (int)GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11);
//	if(res == 1)
//	{
//		mode |= (0x01);
//	}

	mode=1;
	
	printf("system mode : %d\n",mode);
	return mode;
}

char test_buf[64];
static void console_entery(void)
{
	int ret = 0;
	char *temp;
	char *start;	
	ret = usart1_read(test_buf,64);//一次性读取64个字节出来
	if(ret>0)
	{
		start = test_buf;
		temp = start+ret;
		while(start<temp)
		{
			console_proc(*start++);//一个一个字节处理USART接收到的数据
		}
	}
}
#endif

extern S1_PRO_STRUCT S1_pro;

int main(void)
{

#if 0
	static u8 first3min=1;//开机高压充电3分钟的标志
	u8 key;
	u8 t=0;
	u16 pwmtimes=0;
	u32 playtimes=0;
	u8 play_flag=0;
	u16 adcx;
	float temp;
	u8 adc_flag=0;
	u8 adc_last_flag=0;
	u8 first_play_flag = 1;//默认是第一次控制打色
  unsigned char cardFinshed=0;
	unsigned char cardFinshedFlag=1;
	unsigned long cut_t = 0;
	static u8 touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数
	u8 get_key = 0;
	u8 last_key = 0;
	u8 key_flag = 0;
	static u16 times=0; 
	
	/*CC110x*/
	int get_cc110x_module = 0;
	u8 TxBuf[8]={0};	 //8字节，如果需要更长的数据包,请正确设置
//***************更多功率参数设置可详细参考DATACC1100英文文档中第48-49页的参数表******************
//u8 PaTabel[8] = {0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04};  //-30dBm   功率最小
u8 PaTabel[8] = {0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60};  //0dBm
//u8 PaTabel[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0};   //10dBm     功率最大
//************************************************************************************************	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组2
	delay_init();	    	//延时函数初始化	  
	/*PWM test*/ 
	/*100K ~ 120K*/ //C = 68pF/2 //40mm充电线圈（含铝散热片）
	//TIM1_PWM_Init(649,0);//PWM输出：PA8，不分频，PWM频率=72000/(649+1)=110Khz，PWM的占空比 = 50% VPP=170V递增（中心频率右边）大于180V时电机会发热
	TIM1_PWM_Init(711,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
	/*这个测试OK！*/ //骰子 = 6.8nF-50V-X7R谐振电容、6Ω小线圈、1mA功耗

//	TIM1_PWM_Init(711,0);
//	TIM_SetCompare1(TIM1,(711+1)/2);//调整PWM的占空比 = 50%

//	/*100K ~ 120K*/ //C = 68pF/2 //40mm充电线圈（不含铝散热片）
//	TIM1_PWM_Init(661,0);//PWM输出：PA8，不分频，PWM频率=72000/(661+1)=108Khz，PWM的占空比 = 50% VPP=160V递增（中心频率右边）


//	//C = 68pF /*80K*/ //40mm充电线圈（含铝散热片）
//	//TIM1_PWM_Init(903,0);//PWM输出：PA8，不分频，PWM频率=72000/(903+1)=80Khz，PWM的占空比 = 50% VPP=160V递增（中心频率右边）
//	TIM1_PWM_Init(999,0);//PWM输出：PA8，不分频，PWM频率=72000/(995+1)=72Khz，PWM的占空比 = 50% VPP=180V递减（中心频率左边）
//	/*这个测试OK！*/ //骰子 = 10nF-25V-X7R谐振电容、6Ω小线圈、4.8mA功耗


//	//C = 68pF/2 /*80K*/ //50mm充电线圈
//	TIM1_PWM_Init(965,0);//PWM输出：PA8，不分频，PWM频率=72000/(965+1)=75Khz，PWM的占空比 = 50%

	//战神充电线圈 /*100K ~ 120K*/ //C = 68pF
//	TIM1_PWM_Init(665,0);//PWM输出：PA8，不分频，PWM频率=72000/(665+1)=108Khz，PWM的占空比 = 50%

	//战神充电线圈 /*70K ~ 90K*/ //C = 68pF*2
//	TIM1_PWM_Init(935,0);//PWM输出：PA8，不分频，PWM频率=72000/(935+1)=77Khz，PWM的占空比 = 50%

	/*无线充电test*/
	/*注意：USART1和PA8-PWM输出有冲突，PA9-TX发送数据前必须要关闭PA8-PWM输出*/
	/*注意：连接线材必须要质量好的！不然会很影响VPP值！（注意发热也会影响无线充电的VPP值，要验证！）== VPP > 300V*/
/*
注意：必须先初始化PWM输出再初始化串口！！！否则PWM不能输出！！！
原因：PWM输出和串口都是PA口，如果先初始化串口的话，PWM输出的PA8口配置就没用了
*/
/*
注意：初始化完了全部硬件之后，不能再继续调用硬件初始化的函数了，如：TIM1_PWM_Init(649,0);
原因：重新配置PWM输出的PA8口，会影响PA口的串口通信（PA9-TX和PA10-RX），硬件的初始化尽量不要重复调用！！！
*/


	keyValueInit();/*6个按键和洗牌结束信号解码*/
	//配置中断初始化函数
	//STCP     PB0
	//clk      PC4
	//key_in   PB12
	//DS_OUT   PB13

	//uart1_init(9600);	 	//串口1初始化为9600（PA9-U1-TX;PA10-U1-RX）
	uart1_init(115200);
	LED_Init();		  		//初始化与LED连接的硬件接口：PC13
 	KEY_Init();				  //按键初始化：PA15 	

	/*DC280V通断控制test*/
	/*CMT211xA无线通信发射 test：马达输入检测；马达输出控制；DC280V通断控制*/
  CMT211xA_PORT_Init();	//PC3
  MOTOR_OUT_Init();			//PC1
  MOTOR_IN_Init();			//PC2	 
	DC280V_Control_Init();//PC0
	
//	/*测试vDelayUs(416);的实际延时*/
//	/*注意这里的vDelayUs(416)很关键，直接影响编码发射和接收解码的成功率*/
//	CMT211xA_vDelay416Us_test();/*测试vDelayUs(416);的实际延时*/ 	
	/*4个串口test*/
	all_uart_init();
	/*CC2540蓝牙模块（HC-08）test = UART1收发通信*/
	/*CC1101模块test*/
  SpiInit(); 								// 初始化spi硬件接口	
	delay_ms(100); 					
	POWER_UP_RESET_CC1100(); 	// 上电复位CC110x
	halRfWriteRfSettings();		// 配置CC1100的寄存器
	halSpiWriteBurstReg(CCxxx0_PATABLE, PaTabel, 8); // SPI连续写配置寄存器

	//TxBuf[0] = 8 ;
	TxBuf[1] = 1 ;
	TxBuf[2] = 1 ;
	halRfSendPacket(TxBuf,8);	// Transmit Tx buffer data
	delay_ms(100);	
	/*注意：初始化PC3输出的时候只能配置输出低电平，否则CC1101模块会初始化失败 = 卡死在 halRfSendPacket(TxBuf,8); */
	/*ADC test*/
	Adc_Init();//ADC初始化：PA0
	
	/*覃先维：串口终端接收处理初始化*/
	//init_console();
	//register_console_write((int (*)(char *,int))usart1_write);//注册发送函数：usart1_write
	//register_cmds();//注册命令和回调处理：在这里添加自定义命令，如reboot
//	switch(read_sys_mode())
//	{
//		case 1:
//			break;
//		case 2:
//			break;
//		case 3:
//		  break;
//		
//		default:
//			set_login_info("kitty","qwer1314");//登录名和密码
//			break;
//	}
//	//注意：要初始化完硬件之后再初始化这里，不然会影响PWM的输出

//#if 0
//	//开机高压充电3分钟
//	TIM1_PWM_Init(701,0);//PWM输出：PA8，不分频，PWM频率=72000/(701+1)=102Khz，PWM的占空比 = 50% VPP=190V递减（中心频率左边）
//	printf("\r\nfirst work 3min High voltage charging = VPP = 190V!\r\n");  
//	//开机高压充电3分钟
//	if(first3min)
//	{
//		for(t = 0; t < 3*60; t++) delay_ms(1000);//3分钟延时
//		first3min = 0;
//		TIM1_PWM_Init(711,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
//		printf("\r\nfirst work 3min High voltage charging ok!\r\n");  
//	}
//	TIM1_PWM_Init(711,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
//	printf("\r\nrun to while(1)\r\n"); 
//#endif		
	while(1)
	{		
		
		/*覃先维：串口终端接收处理入口*/
		//console_entery();
		systick_delay(1);//1ms心跳
		times++;	
		/*led test*/
		if(times%300==0)LED=!LED;//LED 300ms闪烁一次，提示系统正在运行
		/*cc110x test*/
		get_cc110x_module = cc110x_module_entery();
		
		/*无线充电test：开8秒，关2秒，如此反复（同时检测温度）*/
		pwmtimes++;//delay_ms(1);
		if(pwmtimes&0x8000)/*使能*/
		{
			if((pwmtimes&0X3FFF)%9000==0)//开8s之后/9
			{
				pwmtimes=0;/*数据清零*/
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭 
				set_pwm_flag(1);//关闭 
			}
			else 
			{
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能
				set_pwm_flag(0);//使能
			}
		}
		else/*关闭*/
		{
			if(pwmtimes%500==0)//关闭2s之后/0.5
			{
				pwmtimes=0;/*数据清零*/
				pwmtimes|=0x8000;
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能
				set_pwm_flag(0);//使能
			}
			else 
			{
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭 
				set_pwm_flag(1);//关闭 
			}	
		}	
		/*ADC test：温度采集，温度过高就不控色打色、关闭无线充电*/
		/*NTC热敏电阻的温度特性：
			25? = 10K (1.50V adcx=?)
			58? = 3K  (0.76V adcx=?)
			72? = 2K  (0.55V adcx=?)
		*/
		if(times%5000==0)//delay_ms(1);
		{
			adcx=Get_Adc_Average(ADC_Channel_0,10);//PA0
			temp=(float)adcx*(3.3/4096);
			
			if(temp < 0.7)//温度太高
			{
				adc_flag = 1;

				printf("\r\nadc_flag = 1 = 温度过高！\r\n");  
			}
			else 
			{
				adc_flag = 0;
				//if(console_cmd_adc())//串口终端控制是否打开ADC打印，默认关闭
				//	printf("\r\nadc_flag = 0 = 温度正常！\r\n");  
			}
		}	
		//开机充电5分钟（300s）后，才能控色打色
		playtimes++;//delay_ms(1);
		//if(playtimes == 300*1000)
		if(playtimes == 180*1000)//3分钟
		{
			play_flag = 1;
			printf("\n\r\n\r充电已满3分钟 ready to play!\n\r\n\r");
		}
		//play_flag = 1;
		
		
		if(MOTOR_IN_Check()==0)//获取微型马达状态：微型马达转动，开始打色
		{
			if(key_flag == 1)
			{
				get_key = last_key;//获取东南西北按键：1234		
			}			
			//判断东南西北按键
			if(get_key>=1 && get_key<=4)//庄家方位
			{				
				//touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数
				if(touzi_recv[get_key-1][0]>=1 && touzi_recv[get_key-1][0]<=3)//判断是否接收到了点数命令：打色次数
				{					
					if(adc_flag==1 || play_flag==0)//需要控制打色的时候，温度太高或者充电时间不够			
					{
						/*说明：如果第一次正常控制打色，则第二次也要正常控制；如果第一次不控制打色，则第二次也要不控制打色*/
						if(first_play_flag)//判断是不是第一次控制打色
						{
							first_play_flag=0;
							adc_last_flag=0;//第一次不正常控制	
							printf("\n\r\n\r first time error! \n\r\n\r");

							/*不控制打色*/
							normal_work();//普通打色
						}
						else//不是第一次打色
						{
							if(adc_last_flag==1)//如果上一次正常控制，这一次也要正常控制
							{
								/*控制打色*/
								power_work(touzi_recv[get_key-1][1]);//控制打色
							}
							else
							{
								/*不控制打色*/
								normal_work();//普通打色
							}
						}					
					}
					else//正常控制打色
					{
						if(first_play_flag==1)//判断是不是第一次控制打色
						{ 
							first_play_flag=0;
							adc_last_flag=1;//第一次正常控制	
							printf("\n\r\n\r first time normal! \n\r\n\r");							
					
							/*控制打色*/
							power_work(touzi_recv[get_key-1][1]);//控制打色
						}
						else//不是第一次打色
						{
							if(adc_last_flag==1)//如果上一次正常控制，这一次也要正常控制
							{
								/*控制打色*/
								power_work(touzi_recv[get_key-1][1]);//控制打色
							}
							else
							{
								/*不控制打色*/
								normal_work();//普通打色
							}
						}					
					}
					
					touzi_recv[get_key-1][0]--;
					if(touzi_recv[get_key-1][0] == 0)//只打1次色
					{
						//打完一次控色之后，要继续充电5分钟后才能控色打色
						if(play_flag)
						{
							play_flag=0;
							playtimes=0;
							printf("\n\r\n\r需要充电3分钟 not ready to play!\n\r\n\r");
						}

						//所有点数清零
						for (t = 0; t < 4; t++)
							sz_memset((char*)&touzi_recv[t][0],0,4);						
						key_flag = 0;
						first_play_flag=1;//标志下一次就是第一次控制打色
						
						//发送打色结束命令
						all_uart_entry(get_cc110x_module,touzi_recv[0],touzi_recv[1],touzi_recv[2],touzi_recv[3],0,1);//touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数						
					}
					else//2-3次
					{
						touzi_recv[get_key-1][1] = touzi_recv[get_key-1][2];//移位
						touzi_recv[get_key-1][2] = touzi_recv[get_key-1][3];//移位
						last_key=get_key;	
						key_flag = 1;
					}	
				}
				else//没有接收到点数命令
				{
					normal_work();//普通打色
				}
			}
			else//error：还是要判断点数命令
			{
				normal_work();//普通打色
			}
		}
		for (t = 0; t < 6; t++)
		{
			if(stPwm.keyValue[t]) 
			{  
				/**/
				if(key_flag == 0)
				{
					//if(t>=0 && t<=3)//庄家方位
					if(t<=3)//庄家方位
					{
						get_key = t+1;//获取东南西北按键：1234								
						//东和西互换，南和北互换
						if(get_key==1)get_key=3;
						//else if(get_key==2)get_key=4;//南和北不换
						else if(get_key==3)get_key=1;
						//else if(get_key==4)get_key=2;//南和北不换
/*
	西
南  北
	东
*/																		
					}
				}
				else
				{
					get_key=last_key;	
				}
				
				//printf("name[%d]:%s\r\n",t,&name[t][0]);
				stPwm.keyValue[t]=0;//清除
			}
		}
		//获取洗牌信号
		if(cut_t+1000<get_systick())
		{
			cut_t=get_systick();
			if(stPwm.cardWashFlag==KEY_TRUE)
			{
				if(cardFinshedFlag==1)
				{					
					cardFinshed++;
					if(cardFinshed==2)//5个
					{
						cardFinshed=0;
						cardFinshedFlag=0;
					}	
				}					
				//printf("洗牌亮\r\n");
				stPwm.cardWashFlag=KEY_FALSE;
			}
			else//洗牌结束判断
			{
				//初始化cardFinshedFlag = 1；
				if(cardFinshedFlag==0)//开机就检测到洗牌结束信号怎么处理？开机正在洗牌？
				{					
					cardFinshed++;
					if(cardFinshed==2)//5个
					{
						cardFinshed=0;
						cardFinshedFlag=1;
						
						printf("\r\n\r\n send cardFinshedFlag==1 \r\n\r\n");
						//发送洗牌结束信号
						all_uart_entry(get_cc110x_module,touzi_recv[0],touzi_recv[1],touzi_recv[2],touzi_recv[3],cardFinshedFlag,0);//touzi_recv[4][4] = {0};//点数（4个方位）= 方位+次数+点数
					}	
				}					
				//printf("洗牌灭\r\n");
			}
		}

		
////遥控测试		
//switch(get_cc110x_module)//判断接收到的数据：TxBuf[1] = 1~9; //9个按键
//{
//		case 1://VOL- = 发送洗牌结束命令和打色结束命令测试
//				
//			break;

//		case 2://上 = 玩家 = 西
//			
//			break;

//		case 3://VOL+
//				power_work(0xFF);//FPC板卡测试
//			break;

//		case 4://右 = 玩家 = 南
//			
//			break;

//		case 5://下 = 玩家 = 东

//			break;

//		case 6://左 = 玩家 = 北

//			break;
//		
//		case 7://右下
//				power_work(0x33);//控制打色测试：33
//			break;	

//		case 8://中下
//				power_work(0x22);//控制打色测试：22
//			break;

//		case 9://左下
//				power_work(0x11);//控制打色测试：11
//			break;	

//	
//	default:break;/*error*/	
//}
		
		
	}
#else

		
		/*CC110x*/
		int get_cc110x_module = 0;
		u8 TxBuf[8]={0};	 //8字节，如果需要更长的数据包,请正确设置
	//***************更多功率参数设置可详细参考DATACC1100英文文档中第48-49页的参数表******************
	//u8 PaTabel[8] = {0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04};  //-30dBm	功率最小
	u8 PaTabel[8] = {0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60};  //0dBm
	//u8 PaTabel[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0};   //10dBm	  功率最大
	//************************************************************************************************	
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组2
		delay_init();			//延时函数初始化   
		/*PWM test*/ 
		/*100K ~ 120K*/ //C = 68pF/2 //40mm充电线圈（含铝散热片）
		//TIM1_PWM_Init(649,0);//PWM输出：PA8，不分频，PWM频率=72000/(649+1)=110Khz，PWM的占空比 = 50% VPP=170V递增（中心频率右边）大于180V时电机会发热
		TIM1_PWM_Init(711,0);//PWM输出：PA8，不分频，PWM频率=72000/(711+1)=101Khz，PWM的占空比 = 50% VPP=170V递减（中心频率左边）
		/*这个测试OK！*/ //骰子 = 6.8nF-50V-X7R谐振电容、6Ω小线圈、1mA功耗
		keyValueInit();/*6个按键和洗牌结束信号解码*/
		//配置中断初始化函数
		//STCP	   PB0
		//clk	   PC4
		//key_in   PB12
		//DS_OUT   PB13
	
		//uart1_init(9600); 	//串口1初始化为9600（PA9-U1-TX;PA10-U1-RX）
		uart1_init(115200);
		LED_Init(); 			//初始化与LED连接的硬件接口：PC13
		KEY_Init(); 			  //按键初始化：PA15	
	
		/*DC280V通断控制test*/
		/*CMT211xA无线通信发射 test：马达输入检测；马达输出控制；DC280V通断控制*/
	   CMT211xA_PORT_Init(); //PC3
	   MOTOR_OUT_Init(); 		//PC1
	   MOTOR_IN_Init();			//PC2	 
		DC280V_Control_Init();//PC0
		
	//	/*测试vDelayUs(416);的实际延时*/
	//	/*注意这里的vDelayUs(416)很关键，直接影响编码发射和接收解码的成功率*/
	//	CMT211xA_vDelay416Us_test();/*测试vDelayUs(416);的实际延时*/ 	
		/*4个串口test*/
		all_uart_init();
		/*CC2540蓝牙模块（HC-08）test = UART1收发通信*/
		/*CC1101模块test*/
	    SpiInit();								// 初始化spi硬件接口 
		delay_ms(100);					
		POWER_UP_RESET_CC1100();	// 上电复位CC110x
		halRfWriteRfSettings(); 	// 配置CC1100的寄存器
		halSpiWriteBurstReg(CCxxx0_PATABLE, PaTabel, 8); // SPI连续写配置寄存器
	
		//TxBuf[0] = 8 ;
		TxBuf[1] = 1 ;
		TxBuf[2] = 1 ;
		halRfSendPacket(TxBuf,8);	// Transmit Tx buffer data
		delay_ms(100);	
		/*注意：初始化PC3输出的时候只能配置输出低电平，否则CC1101模块会初始化失败 = 卡死在 halRfSendPacket(TxBuf,8); */
		/*ADC test*/
		Adc_Init();//ADC初始化：PA0
		
		S1_Init();
/*
		{
		   u16 wCrc=0;
		   u8 by[2];
           u8 b=0,ii=0,data[10]={0xff,0xaa,0x00,0x02,0x02,0x01,0x03,0x00,0x00};
		   for(ii=0;ii<4;ii++)
		   {
              data[5]=ii;
			  wCrc=PTLC_GetCrc((const u8 *)data,7);
				 printf("wCrc:%x \r\n",wCrc);
			  data[8]=wCrc;
			  data[7] = wCrc >> 8;
				 printf("data[7]:%x data[6]:%x\r\n",data[7],data[6]);
			  for(b=0;b<9;b++) printf("%x ",data[b]);printf("\r\n");
		   }
		}
		 ////点数（4个方位）= 方位+次数+个数+点数 
		{
           u16 wCrc=0;
		   u8 by[2];
           u8 b=0,ii=0,data[20]={0xff,0xaa,0x00,0x06,0x07,0x00,0x02,0x02,0x02,0x02,0x03,0x03,0x00,0x00};

              //data[5]=ii;
			  wCrc=PTLC_GetCrc((const u8 *)data,12);
				 printf("wCrc:%x \r\n",wCrc);
			  data[13]=wCrc;
			  data[12] = wCrc >> 8;
				// printf("data[7]:%x data[6]:%x\r\n",data[11],data[12]);
			  for(b=0;b<14;b++) printf("%x ",data[b]);printf("\r\n");
        
		}
		{
		u16 wCrc=0;
		u8 by[2];
		u8 b=0,data[20]={ 0xaa ,0x00 ,0x02 ,0x02,0x03,0x03,0xec,0x8c};
		wCrc=PTLC_GetCrc((const u8 *)data,6);
		   printf("wCrc:%x \r\n",wCrc);
		data[7]=wCrc;
		data[6] = wCrc >> 8;
		printf("%x ",data[b]);printf("\r\n");

		}
	*/	
		while(1)
		{
		  #if 1
		    if(S1_pro.timeoutCnt%300==0)LED=!LED;//LED 300ms闪烁一次，提示系统正在运行
		    #if PCTL_PRO_ONLY
            systick_delay(1);
			#else
			S1_HARDWARE_Pro();
			#endif
			usart_pro();
            S1_CTRL_PrO();
		  #else
		  S1_HARDWARE_Pro();       
		  #endif
         
		}


#endif
}


