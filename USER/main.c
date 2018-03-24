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
//��ͨ��ɫ
static void normal_work(void)
{	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  	
	//USART_printf = 1;/*ѡ�񴮿�1��Ϊprintf���*/
	printf("\n\rno command!\n\r");
	
	/*��ת�����*/
	MOTOR_OUT_ON ();
	
	//��ʱ3��		
	delay_ms(1000);/*����ת��*/	
	delay_ms(1000);/*����ת��*/	
	delay_ms(1000);/*����ת��*/	
	//ע�⣺STM32��72M������,nms<=1864 
	
	/*��ת�����*/
	MOTOR_OUT_OFF ();	
	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  	
	printf("\n\rnormal_work = no command!\n\r");
}

//���ƴ�ɫ
static void power_work(u8 touzi_num)
{
  static u16 crc;
  static u8 by[2];
  static u8 str[7] = {'C', 'M', 'O', 'S', 'T', 'E', 'K'};
	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  
	printf("\n\rtest num = %2X\n\r",touzi_num);/*���2λ16������*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*�����߳��*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  				

	/*���͵�������*/
	str[0] = 0x55;
	str[1] = 0xab;
	str[2] = touzi_num;//����A����4λ������B����4λ
	//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
	
	crc = GetCrc((const u8 *)str, CMT211xA_DATALength - 2);
	by[0] = crc;
	by[1] = crc >> 8;
	str[CMT211xA_DATALength - 2] = by[0];//У�����8λ��ǰ
	str[CMT211xA_DATALength - 1] = by[1];

	//������3�Σ���Ӧ��
	CMT211xA_Encode(str, CMT211xA_DATALength, 0);//����ǰ����
	CMT211xA_TxPacket();//����������ݰ� �� 70ms
	delay_ms(500);
//			delay_ms(200);
//			CMT211xA_TxPacket();//����������ݰ� �� 70ms
//			delay_ms(200);
//			CMT211xA_TxPacket();//����������ݰ� �� 70ms
	/*ע�⣺��������CMT211xA_TxPacket();����vDelayUs(1000);��ʵ����ʱ*/
	
				
	/*��ת�����*/
	MOTOR_OUT_ON ();
	delay_ms(1000);/*����ת��*/	
//DC280V_Control_ON ();//�쳣����	
	delay_ms(1000);/*����ת��*/	
	//ע�⣺STM32��72M������,nms<=1864 
	
//	/*������Ȧ*/
//	DC280V_Control_ON ();
	delay_ms(100);/*����ת��*/	
	/*ע�⣺��֤ = S1ת��̫�������£����Խ��յ�����������*/
	
	/*��ת�����*/
	MOTOR_OUT_OFF ();
	
	/*�ر�΢���������ʱ*/
	delay_ms(500);/*����ת��*/

	/*������Ȧ*/
	DC280V_Control_ON ();

	delay_ms(500);/*����ת��*/
	//delay_ms(800);/*����ת��*/
	//delay_ms(1000);/*����ת��*/
//delay_ms(1000);//�쳣����		
	DC280V_Control_OFF ();//�رմ���Ȧ����
	//ע�⣺Ҫ���Ժ�ת��ֹͣ����Ȧ�ų�ͨ�ϵ�ʱ���߼���ϵ -- �ܹؼ���
				
	/*�����߳��*/
	TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��
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
	ret = usart1_read(test_buf,64);//һ���Զ�ȡ64���ֽڳ���
	if(ret>0)
	{
		start = test_buf;
		temp = start+ret;
		while(start<temp)
		{
			console_proc(*start++);//һ��һ���ֽڴ���USART���յ�������
		}
	}
}
#endif

extern S1_PRO_STRUCT S1_pro;

int main(void)
{

#if 0
	static u8 first3min=1;//������ѹ���3���ӵı�־
	u8 key;
	u8 t=0;
	u16 pwmtimes=0;
	u32 playtimes=0;
	u8 play_flag=0;
	u16 adcx;
	float temp;
	u8 adc_flag=0;
	u8 adc_last_flag=0;
	u8 first_play_flag = 1;//Ĭ���ǵ�һ�ο��ƴ�ɫ
  unsigned char cardFinshed=0;
	unsigned char cardFinshedFlag=1;
	unsigned long cut_t = 0;
	static u8 touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����
	u8 get_key = 0;
	u8 last_key = 0;
	u8 key_flag = 0;
	static u16 times=0; 
	
	/*CC110x*/
	int get_cc110x_module = 0;
	u8 TxBuf[8]={0};	 //8�ֽڣ������Ҫ���������ݰ�,����ȷ����
//***************���๦�ʲ������ÿ���ϸ�ο�DATACC1100Ӣ���ĵ��е�48-49ҳ�Ĳ�����******************
//u8 PaTabel[8] = {0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04};  //-30dBm   ������С
u8 PaTabel[8] = {0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60};  //0dBm
//u8 PaTabel[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0};   //10dBm     �������
//************************************************************************************************	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����2
	delay_init();	    	//��ʱ������ʼ��	  
	/*PWM test*/ 
	/*100K ~ 120K*/ //C = 68pF/2 //40mm�����Ȧ������ɢ��Ƭ��
	//TIM1_PWM_Init(649,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(649+1)=110Khz��PWM��ռ�ձ� = 50% VPP=170V����������Ƶ���ұߣ�����180Vʱ����ᷢ��
	TIM1_PWM_Init(711,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
	/*�������OK��*/ //���� = 6.8nF-50V-X7Rг����ݡ�6��С��Ȧ��1mA����

//	TIM1_PWM_Init(711,0);
//	TIM_SetCompare1(TIM1,(711+1)/2);//����PWM��ռ�ձ� = 50%

//	/*100K ~ 120K*/ //C = 68pF/2 //40mm�����Ȧ��������ɢ��Ƭ��
//	TIM1_PWM_Init(661,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(661+1)=108Khz��PWM��ռ�ձ� = 50% VPP=160V����������Ƶ���ұߣ�


//	//C = 68pF /*80K*/ //40mm�����Ȧ������ɢ��Ƭ��
//	//TIM1_PWM_Init(903,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(903+1)=80Khz��PWM��ռ�ձ� = 50% VPP=160V����������Ƶ���ұߣ�
//	TIM1_PWM_Init(999,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(995+1)=72Khz��PWM��ռ�ձ� = 50% VPP=180V�ݼ�������Ƶ����ߣ�
//	/*�������OK��*/ //���� = 10nF-25V-X7Rг����ݡ�6��С��Ȧ��4.8mA����


//	//C = 68pF/2 /*80K*/ //50mm�����Ȧ
//	TIM1_PWM_Init(965,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(965+1)=75Khz��PWM��ռ�ձ� = 50%

	//ս������Ȧ /*100K ~ 120K*/ //C = 68pF
//	TIM1_PWM_Init(665,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(665+1)=108Khz��PWM��ռ�ձ� = 50%

	//ս������Ȧ /*70K ~ 90K*/ //C = 68pF*2
//	TIM1_PWM_Init(935,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(935+1)=77Khz��PWM��ռ�ձ� = 50%

	/*���߳��test*/
	/*ע�⣺USART1��PA8-PWM����г�ͻ��PA9-TX��������ǰ����Ҫ�ر�PA8-PWM���*/
	/*ע�⣺�����߲ı���Ҫ�����õģ���Ȼ���Ӱ��VPPֵ����ע�ⷢ��Ҳ��Ӱ�����߳���VPPֵ��Ҫ��֤����== VPP > 300V*/
/*
ע�⣺�����ȳ�ʼ��PWM����ٳ�ʼ�����ڣ���������PWM�������������
ԭ��PWM����ʹ��ڶ���PA�ڣ�����ȳ�ʼ�����ڵĻ���PWM�����PA8�����þ�û����
*/
/*
ע�⣺��ʼ������ȫ��Ӳ��֮�󣬲����ټ�������Ӳ����ʼ���ĺ����ˣ��磺TIM1_PWM_Init(649,0);
ԭ����������PWM�����PA8�ڣ���Ӱ��PA�ڵĴ���ͨ�ţ�PA9-TX��PA10-RX����Ӳ���ĳ�ʼ��������Ҫ�ظ����ã�����
*/


	keyValueInit();/*6��������ϴ�ƽ����źŽ���*/
	//�����жϳ�ʼ������
	//STCP     PB0
	//clk      PC4
	//key_in   PB12
	//DS_OUT   PB13

	//uart1_init(9600);	 	//����1��ʼ��Ϊ9600��PA9-U1-TX;PA10-U1-RX��
	uart1_init(115200);
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿڣ�PC13
 	KEY_Init();				  //������ʼ����PA15 	

	/*DC280Vͨ�Ͽ���test*/
	/*CMT211xA����ͨ�ŷ��� test����������⣻���������ƣ�DC280Vͨ�Ͽ���*/
  CMT211xA_PORT_Init();	//PC3
  MOTOR_OUT_Init();			//PC1
  MOTOR_IN_Init();			//PC2	 
	DC280V_Control_Init();//PC0
	
//	/*����vDelayUs(416);��ʵ����ʱ*/
//	/*ע�������vDelayUs(416)�ܹؼ���ֱ��Ӱ����뷢��ͽ��ս���ĳɹ���*/
//	CMT211xA_vDelay416Us_test();/*����vDelayUs(416);��ʵ����ʱ*/ 	
	/*4������test*/
	all_uart_init();
	/*CC2540����ģ�飨HC-08��test = UART1�շ�ͨ��*/
	/*CC1101ģ��test*/
  SpiInit(); 								// ��ʼ��spiӲ���ӿ�	
	delay_ms(100); 					
	POWER_UP_RESET_CC1100(); 	// �ϵ縴λCC110x
	halRfWriteRfSettings();		// ����CC1100�ļĴ���
	halSpiWriteBurstReg(CCxxx0_PATABLE, PaTabel, 8); // SPI����д���üĴ���

	//TxBuf[0] = 8 ;
	TxBuf[1] = 1 ;
	TxBuf[2] = 1 ;
	halRfSendPacket(TxBuf,8);	// Transmit Tx buffer data
	delay_ms(100);	
	/*ע�⣺��ʼ��PC3�����ʱ��ֻ����������͵�ƽ������CC1101ģ����ʼ��ʧ�� = ������ halRfSendPacket(TxBuf,8); */
	/*ADC test*/
	Adc_Init();//ADC��ʼ����PA0
	
	/*����ά�������ն˽��մ����ʼ��*/
	//init_console();
	//register_console_write((int (*)(char *,int))usart1_write);//ע�ᷢ�ͺ�����usart1_write
	//register_cmds();//ע������ͻص���������������Զ��������reboot
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
//			set_login_info("kitty","qwer1314");//��¼��������
//			break;
//	}
//	//ע�⣺Ҫ��ʼ����Ӳ��֮���ٳ�ʼ�������Ȼ��Ӱ��PWM�����

//#if 0
//	//������ѹ���3����
//	TIM1_PWM_Init(701,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(701+1)=102Khz��PWM��ռ�ձ� = 50% VPP=190V�ݼ�������Ƶ����ߣ�
//	printf("\r\nfirst work 3min High voltage charging = VPP = 190V!\r\n");  
//	//������ѹ���3����
//	if(first3min)
//	{
//		for(t = 0; t < 3*60; t++) delay_ms(1000);//3������ʱ
//		first3min = 0;
//		TIM1_PWM_Init(711,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
//		printf("\r\nfirst work 3min High voltage charging ok!\r\n");  
//	}
//	TIM1_PWM_Init(711,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
//	printf("\r\nrun to while(1)\r\n"); 
//#endif		
	while(1)
	{		
		
		/*����ά�������ն˽��մ������*/
		//console_entery();
		systick_delay(1);//1ms����
		times++;	
		/*led test*/
		if(times%300==0)LED=!LED;//LED 300ms��˸һ�Σ���ʾϵͳ��������
		/*cc110x test*/
		get_cc110x_module = cc110x_module_entery();
		
		/*���߳��test����8�룬��2�룬��˷�����ͬʱ����¶ȣ�*/
		pwmtimes++;//delay_ms(1);
		if(pwmtimes&0x8000)/*ʹ��*/
		{
			if((pwmtimes&0X3FFF)%9000==0)//��8s֮��/9
			{
				pwmtimes=0;/*��������*/
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر� 
				set_pwm_flag(1);//�ر� 
			}
			else 
			{
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��
				set_pwm_flag(0);//ʹ��
			}
		}
		else/*�ر�*/
		{
			if(pwmtimes%500==0)//�ر�2s֮��/0.5
			{
				pwmtimes=0;/*��������*/
				pwmtimes|=0x8000;
				TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��
				set_pwm_flag(0);//ʹ��
			}
			else 
			{
				TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر� 
				set_pwm_flag(1);//�ر� 
			}	
		}	
		/*ADC test���¶Ȳɼ����¶ȹ��߾Ͳ���ɫ��ɫ���ر����߳��*/
		/*NTC����������¶����ԣ�
			25? = 10K (1.50V adcx=?)
			58? = 3K  (0.76V adcx=?)
			72? = 2K  (0.55V adcx=?)
		*/
		if(times%5000==0)//delay_ms(1);
		{
			adcx=Get_Adc_Average(ADC_Channel_0,10);//PA0
			temp=(float)adcx*(3.3/4096);
			
			if(temp < 0.7)//�¶�̫��
			{
				adc_flag = 1;

				printf("\r\nadc_flag = 1 = �¶ȹ��ߣ�\r\n");  
			}
			else 
			{
				adc_flag = 0;
				//if(console_cmd_adc())//�����ն˿����Ƿ��ADC��ӡ��Ĭ�Ϲر�
				//	printf("\r\nadc_flag = 0 = �¶�������\r\n");  
			}
		}	
		//�������5���ӣ�300s���󣬲��ܿ�ɫ��ɫ
		playtimes++;//delay_ms(1);
		//if(playtimes == 300*1000)
		if(playtimes == 180*1000)//3����
		{
			play_flag = 1;
			printf("\n\r\n\r�������3���� ready to play!\n\r\n\r");
		}
		//play_flag = 1;
		
		
		if(MOTOR_IN_Check()==0)//��ȡ΢�����״̬��΢�����ת������ʼ��ɫ
		{
			if(key_flag == 1)
			{
				get_key = last_key;//��ȡ��������������1234		
			}			
			//�ж϶�����������
			if(get_key>=1 && get_key<=4)//ׯ�ҷ�λ
			{				
				//touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����
				if(touzi_recv[get_key-1][0]>=1 && touzi_recv[get_key-1][0]<=3)//�ж��Ƿ���յ��˵��������ɫ����
				{					
					if(adc_flag==1 || play_flag==0)//��Ҫ���ƴ�ɫ��ʱ���¶�̫�߻��߳��ʱ�䲻��			
					{
						/*˵���������һ���������ƴ�ɫ����ڶ���ҲҪ�������ƣ������һ�β����ƴ�ɫ����ڶ���ҲҪ�����ƴ�ɫ*/
						if(first_play_flag)//�ж��ǲ��ǵ�һ�ο��ƴ�ɫ
						{
							first_play_flag=0;
							adc_last_flag=0;//��һ�β���������	
							printf("\n\r\n\r first time error! \n\r\n\r");

							/*�����ƴ�ɫ*/
							normal_work();//��ͨ��ɫ
						}
						else//���ǵ�һ�δ�ɫ
						{
							if(adc_last_flag==1)//�����һ���������ƣ���һ��ҲҪ��������
							{
								/*���ƴ�ɫ*/
								power_work(touzi_recv[get_key-1][1]);//���ƴ�ɫ
							}
							else
							{
								/*�����ƴ�ɫ*/
								normal_work();//��ͨ��ɫ
							}
						}					
					}
					else//�������ƴ�ɫ
					{
						if(first_play_flag==1)//�ж��ǲ��ǵ�һ�ο��ƴ�ɫ
						{ 
							first_play_flag=0;
							adc_last_flag=1;//��һ����������	
							printf("\n\r\n\r first time normal! \n\r\n\r");							
					
							/*���ƴ�ɫ*/
							power_work(touzi_recv[get_key-1][1]);//���ƴ�ɫ
						}
						else//���ǵ�һ�δ�ɫ
						{
							if(adc_last_flag==1)//�����һ���������ƣ���һ��ҲҪ��������
							{
								/*���ƴ�ɫ*/
								power_work(touzi_recv[get_key-1][1]);//���ƴ�ɫ
							}
							else
							{
								/*�����ƴ�ɫ*/
								normal_work();//��ͨ��ɫ
							}
						}					
					}
					
					touzi_recv[get_key-1][0]--;
					if(touzi_recv[get_key-1][0] == 0)//ֻ��1��ɫ
					{
						//����һ�ο�ɫ֮��Ҫ�������5���Ӻ���ܿ�ɫ��ɫ
						if(play_flag)
						{
							play_flag=0;
							playtimes=0;
							printf("\n\r\n\r��Ҫ���3���� not ready to play!\n\r\n\r");
						}

						//���е�������
						for (t = 0; t < 4; t++)
							sz_memset((char*)&touzi_recv[t][0],0,4);						
						key_flag = 0;
						first_play_flag=1;//��־��һ�ξ��ǵ�һ�ο��ƴ�ɫ
						
						//���ʹ�ɫ��������
						all_uart_entry(get_cc110x_module,touzi_recv[0],touzi_recv[1],touzi_recv[2],touzi_recv[3],0,1);//touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����						
					}
					else//2-3��
					{
						touzi_recv[get_key-1][1] = touzi_recv[get_key-1][2];//��λ
						touzi_recv[get_key-1][2] = touzi_recv[get_key-1][3];//��λ
						last_key=get_key;	
						key_flag = 1;
					}	
				}
				else//û�н��յ���������
				{
					normal_work();//��ͨ��ɫ
				}
			}
			else//error������Ҫ�жϵ�������
			{
				normal_work();//��ͨ��ɫ
			}
		}
		for (t = 0; t < 6; t++)
		{
			if(stPwm.keyValue[t]) 
			{  
				/**/
				if(key_flag == 0)
				{
					//if(t>=0 && t<=3)//ׯ�ҷ�λ
					if(t<=3)//ׯ�ҷ�λ
					{
						get_key = t+1;//��ȡ��������������1234								
						//�������������Ϻͱ�����
						if(get_key==1)get_key=3;
						//else if(get_key==2)get_key=4;//�Ϻͱ�����
						else if(get_key==3)get_key=1;
						//else if(get_key==4)get_key=2;//�Ϻͱ�����
/*
	��
��  ��
	��
*/																		
					}
				}
				else
				{
					get_key=last_key;	
				}
				
				//printf("name[%d]:%s\r\n",t,&name[t][0]);
				stPwm.keyValue[t]=0;//���
			}
		}
		//��ȡϴ���ź�
		if(cut_t+1000<get_systick())
		{
			cut_t=get_systick();
			if(stPwm.cardWashFlag==KEY_TRUE)
			{
				if(cardFinshedFlag==1)
				{					
					cardFinshed++;
					if(cardFinshed==2)//5��
					{
						cardFinshed=0;
						cardFinshedFlag=0;
					}	
				}					
				//printf("ϴ����\r\n");
				stPwm.cardWashFlag=KEY_FALSE;
			}
			else//ϴ�ƽ����ж�
			{
				//��ʼ��cardFinshedFlag = 1��
				if(cardFinshedFlag==0)//�����ͼ�⵽ϴ�ƽ����ź���ô������������ϴ�ƣ�
				{					
					cardFinshed++;
					if(cardFinshed==2)//5��
					{
						cardFinshed=0;
						cardFinshedFlag=1;
						
						printf("\r\n\r\n send cardFinshedFlag==1 \r\n\r\n");
						//����ϴ�ƽ����ź�
						all_uart_entry(get_cc110x_module,touzi_recv[0],touzi_recv[1],touzi_recv[2],touzi_recv[3],cardFinshedFlag,0);//touzi_recv[4][4] = {0};//������4����λ��= ��λ+����+����
					}	
				}					
				//printf("ϴ����\r\n");
			}
		}

		
////ң�ز���		
//switch(get_cc110x_module)//�жϽ��յ������ݣ�TxBuf[1] = 1~9; //9������
//{
//		case 1://VOL- = ����ϴ�ƽ�������ʹ�ɫ�����������
//				
//			break;

//		case 2://�� = ��� = ��
//			
//			break;

//		case 3://VOL+
//				power_work(0xFF);//FPC�忨����
//			break;

//		case 4://�� = ��� = ��
//			
//			break;

//		case 5://�� = ��� = ��

//			break;

//		case 6://�� = ��� = ��

//			break;
//		
//		case 7://����
//				power_work(0x33);//���ƴ�ɫ���ԣ�33
//			break;	

//		case 8://����
//				power_work(0x22);//���ƴ�ɫ���ԣ�22
//			break;

//		case 9://����
//				power_work(0x11);//���ƴ�ɫ���ԣ�11
//			break;	

//	
//	default:break;/*error*/	
//}
		
		
	}
#else

		
		/*CC110x*/
		int get_cc110x_module = 0;
		u8 TxBuf[8]={0};	 //8�ֽڣ������Ҫ���������ݰ�,����ȷ����
	//***************���๦�ʲ������ÿ���ϸ�ο�DATACC1100Ӣ���ĵ��е�48-49ҳ�Ĳ�����******************
	//u8 PaTabel[8] = {0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04};  //-30dBm	������С
	u8 PaTabel[8] = {0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60 ,0x60};  //0dBm
	//u8 PaTabel[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0};   //10dBm	  �������
	//************************************************************************************************	
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����2
		delay_init();			//��ʱ������ʼ��   
		/*PWM test*/ 
		/*100K ~ 120K*/ //C = 68pF/2 //40mm�����Ȧ������ɢ��Ƭ��
		//TIM1_PWM_Init(649,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(649+1)=110Khz��PWM��ռ�ձ� = 50% VPP=170V����������Ƶ���ұߣ�����180Vʱ����ᷢ��
		TIM1_PWM_Init(711,0);//PWM�����PA8������Ƶ��PWMƵ��=72000/(711+1)=101Khz��PWM��ռ�ձ� = 50% VPP=170V�ݼ�������Ƶ����ߣ�
		/*�������OK��*/ //���� = 6.8nF-50V-X7Rг����ݡ�6��С��Ȧ��1mA����
		keyValueInit();/*6��������ϴ�ƽ����źŽ���*/
		//�����жϳ�ʼ������
		//STCP	   PB0
		//clk	   PC4
		//key_in   PB12
		//DS_OUT   PB13
	
		//uart1_init(9600); 	//����1��ʼ��Ϊ9600��PA9-U1-TX;PA10-U1-RX��
		uart1_init(115200);
		LED_Init(); 			//��ʼ����LED���ӵ�Ӳ���ӿڣ�PC13
		KEY_Init(); 			  //������ʼ����PA15	
	
		/*DC280Vͨ�Ͽ���test*/
		/*CMT211xA����ͨ�ŷ��� test����������⣻���������ƣ�DC280Vͨ�Ͽ���*/
	   CMT211xA_PORT_Init(); //PC3
	   MOTOR_OUT_Init(); 		//PC1
	   MOTOR_IN_Init();			//PC2	 
		DC280V_Control_Init();//PC0
		
	//	/*����vDelayUs(416);��ʵ����ʱ*/
	//	/*ע�������vDelayUs(416)�ܹؼ���ֱ��Ӱ����뷢��ͽ��ս���ĳɹ���*/
	//	CMT211xA_vDelay416Us_test();/*����vDelayUs(416);��ʵ����ʱ*/ 	
		/*4������test*/
		all_uart_init();
		/*CC2540����ģ�飨HC-08��test = UART1�շ�ͨ��*/
		/*CC1101ģ��test*/
	    SpiInit();								// ��ʼ��spiӲ���ӿ� 
		delay_ms(100);					
		POWER_UP_RESET_CC1100();	// �ϵ縴λCC110x
		halRfWriteRfSettings(); 	// ����CC1100�ļĴ���
		halSpiWriteBurstReg(CCxxx0_PATABLE, PaTabel, 8); // SPI����д���üĴ���
	
		//TxBuf[0] = 8 ;
		TxBuf[1] = 1 ;
		TxBuf[2] = 1 ;
		halRfSendPacket(TxBuf,8);	// Transmit Tx buffer data
		delay_ms(100);	
		/*ע�⣺��ʼ��PC3�����ʱ��ֻ����������͵�ƽ������CC1101ģ����ʼ��ʧ�� = ������ halRfSendPacket(TxBuf,8); */
		/*ADC test*/
		Adc_Init();//ADC��ʼ����PA0
		
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
		 ////������4����λ��= ��λ+����+����+���� 
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
		    if(S1_pro.timeoutCnt%300==0)LED=!LED;//LED 300ms��˸һ�Σ���ʾϵͳ��������
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


