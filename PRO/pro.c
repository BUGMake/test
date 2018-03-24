#include "pro.h"

//
S1_PRO_STRUCT S1_pro;//

//A83T zh:console mm:brave
//echo test2.winning8.com > /data/improcess/server && reboot

//��ͷ			��λ	CMD		����	����	��β
//0xff 0xaa		0-4		0-7		len		data	CRC16
//��λ0-4����������S1
//
/*
��λ0-4����������S1
CMD 0-7��
0=�÷�λ���Ƶ�������			A83T-->S1
1=�����ط��Ƶ�������			S1-->A83T
2=�������ƣ�					A83T-->S1
3=�������·������ƣ�			S1-->A83T
4=������λ���Ƶ����ݣ�			S1-->A83T������
5=�����ط�������λ���Ƶ����ݣ�	A83T-->S1
6=������						A83T-->S1
7=�����ط�������				S1-->A83T
typedef enum
{
  CMD_CARD_NUM =0,//            0=�÷�λ���Ƶ�������			A83T-->S1
  CMD_RESEND_CARD_NUM =1,//		S1-->A83T
  CMD_SINGLE_CARD=2,//			2=�������ƣ�					A83T-->S1
  CMD_RESEND_SINGLE_CARD=3,//	3=�������·������ƣ�			S1-->A83T
  CMD_ALL_DIR_CARD=4,//         4=������λ���Ƶ����ݣ�			S1-->A83T������
  CMD_RESEND_ALL_DIR_CARD=5,//  5=�����ط�������λ���Ƶ����ݣ�	A83T-->S1
  CMD_DICE_NUM=6,//				6=������						A83T-->S1
  CMD_RESEND_DICE_NUM=7,//		7=�����ط�������				S1-->A83T
}PCLT_ENUM;//Э��ö��
*/

//////////////////////////
//��ȡ16λУ����

//У�������������
//crc = GetCrc((const u8 *)pData,5+pData[4]);
//by[0] = crc;
//by[1] = crc >> 8;
//if((by[0]==pData[4+pData[4]+1]) && (by[1]==pData[4+pData[4]+2]))/*CRC16У����ȷ*/


static  void PCTL_MemCopy(unsigned char *dest, unsigned char *src, unsigned short size)
{
    unsigned char *pdest;
	unsigned char *psrc;
	pdest=dest;
	psrc=src;
    while(size--) *pdest++ = *src++;
}
static void PCTL_MemSet(unsigned char *dest, unsigned char value, unsigned short size)
{
    unsigned char *pdest=dest;
	while(size--) *pdest++ = value;
}


u16 PTLC_GetCrc(const u8 *pSendBuf, u8 nEnd)
{
	u16 wCrc;
	u8 i,j;
	//u8 by[2];
	wCrc = 0xFFFF;
	for (i = 0; i<nEnd; i++)
	{
		wCrc ^= pSendBuf[i];
		for (j = 0; j<8; j++)
		{
			if (wCrc & 1)
			{
				wCrc >>= 1;
				wCrc ^= 0xA001;
			}
			else
			{
				wCrc >>= 1;
			}
		}
	}
	return wCrc;

}

void sendData_usart(u8 *data,u8 len)
{
   u8 t=0,lenSend=len,i=0;   
   u8 USART_TX_BUF[200];
   PCTL_MemCopy(USART_TX_BUF,data,lenSend);
   
   //for(i=0;i<lenSend;i++)
   //printf("%x ",USART_TX_BUF[i]);	 
   //printf("\r\n");
   switch(S1_pro.send_usart)
   {
		case USART1_TYPE:																			
				for(t=0;t<len;t++)
				{
					USART1->DR=USART_TX_BUF[t];
					while((USART1->SR&0X40)==0);//�ȴ����ͽ���
				}				
			break;
		case USART2_TYPE:
			   printf("USART2_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					USART2->DR=USART_TX_BUF[t];
					while((USART2->SR&0X40)==0);//�ȴ����ͽ���
				}
			break;
		
		case USART3_TYPE:
			printf("USART3_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					USART3->DR=USART_TX_BUF[t];
					while((USART3->SR&0X40)==0);//�ȴ����ͽ���
				}
			break;
		
		case USART4_TYPE:
			printf("USART4_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					UART4->DR=USART_TX_BUF[t];
					while((UART4->SR&0X40)==0);//�ȴ����ͽ���
				}
			break;
		
		case USART5_TYPE:
			printf("USART5_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					UART5->DR=USART_TX_BUF[t];
					while((UART5->SR&0X40)==0);//�ȴ����ͽ���
				}
			break;			
		default:break;/*error*/	    
   }
   
}


//��ͷ			��λ   	CMD		����  	����  	��β
//0xff 0xaa		0-4		0-7		len		data	CRC16
//crc = GetCrc((const u8 *)pData,5+pData[4]);
//by[0] = crc;
//by[1] = crc >> 8;

//	u8 t=0;
//	u16 crc;
//	u8 by[2];		
	//У�������������
	//crc = GetCrc((const u8 *)pData,5+pData[4]);
	//by[0] = crc;
	//by[1] = crc >> 8;
	//if((by[0]==pData[4+pData[4]+1]) && (by[1]==pData[4+pData[4]+2]))/*CRC16У����ȷ*/
//0 s1-->a83t
//1 a83t-->s1
static u8 PTLC_PRO(u8 uart,u8 *data,u8 plen,u8 mode)
{
  
  u16 wCrc;
  u8 by[2];
  u8 len=0,i=0;
  u8 dataLen=0;
  u8 dir=0,CMD=0;//��λ
  u8 dataSend[200]={0};
  if(mode)//a83t-->s1
  {
		  if(data[len]==0xff&&data[len+1]==0xaa) {len+=2;}
		  else {printf("head error\r\n");return PTLC_FALSE;}
		  dir=data[len++];//�Զ�������ĸ�����
		  if(dir>=4) {printf("dir error\r\n");return PTLC_FALSE;}
		  else {
              S1_pro.uasrt_dir[dir][0]=uart;
		  }
		  
		  //У��
 #if REC_DATA_PRINTF
		  {
		  u8 b=0;
		  switch(dir)
	      {
	       case 0:
		   printf("R��:");break;
		   case 1:
		   printf("R��:");break;
		   case 2:
		   printf("R��:");break;
		   case 3:
		   printf("R��:");break;
	      }
		    for(b=0;b<plen;b++) printf("%02x ",data[b]);printf("\r\n");
		  }
 #endif 
		  wCrc=PTLC_GetCrc((const u8 *)data,5+data[4]);
		  //printf("wCrc:%x \r\n",wCrc);
		  by[0]=wCrc;
          by[1] = wCrc >> 8;
		  //printf("data[4]:%x by[0]:%x by[1]:%x \r\n",data[4],by[0],by[1]);
		  //printf("2----\r\n");
	      if((by[0]==data[4+data[4]+1]) && (by[1]==data[4+data[4]+2])){}//����ɹ�
		  else {printf("crc error--\r\n"); return PTLC_FALSE;}//У�����
		  //if(PTLC_GetCrc((const u8 *)data,5+data[4])==PTLC_FALSE) return PTLC_FALSE;//�������  
		  CMD=data[len++];
		  switch(CMD)
		  {
		    case CMD_CARD_NUM://   0=�÷�λ���Ƶ�������			A83T-->S1
		    {
		        //if(S1_pro.PRO_step!=PRO_CARD_NUM_TO_A83T) return PTLC_FALSE;
				if(data[len++]!=1) return PTLC_FALSE;//�ж�����
		        S1_pro.card_num[dir][0]=data[len++];//�Ƶ�����
		        
		        S1_pro.card_recv[dir][CAER_REC_NUM-2]=0; 
		        //printf("====dir:%x num:%x===\r\n",dir,S1_pro.card_num[dir][0]);
		        //S1_pro.card_num_cnt++;
				break;
			}
			case CMD_SINGLE_CARD://2=�������ƣ�					A83T-->S1
			{
			   // printf("1--\r\n");
			    
				//if(S1_pro.PRO_step!=PRO_CARD_ALL_CARD_TO_A83T&&S1_pro.PRO_step!=PRO_CARD_SEND_TO_A83T_DONG) return PTLC_FALSE;	
			    if(data[len++]!=2) return PTLC_FALSE;//�ж�����			    
			    S1_pro.card_recv[dir][data[len]]=data[len+1];len+=2;//��������
			    if(S1_pro.card_recv[dir][CAER_REC_NUM-2]!=CAER_REC_FLAG)
			    {
                   S1_pro.card_recv[dir][CAER_REC_NUM-2]=CAER_REC_FLAG; 
				   printf("dir:%x rq card flag\r\n",dir);
				}
			    
				break;
			}
			case CMD_RESEND_ALL_DIR_CARD://5=�����ط�������λ���Ƶ����ݣ�	A83T-->S1
			{
				if(S1_pro.PRO_step<=PRO_CARD_SEND_TO_A83T_DONG) 
				{
                    printf("���󵥸���λ��������Ч\r\n");
					return PTLC_FALSE;
				}
				//
				
                if(data[len++]!=1) return PTLC_FALSE;//���ݳ��Ȳ���                
			    printf("����������λ������\r\n");
				//for(i=0;i<S1_pro.card_num[data[len]];i++)					
				S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
				S1_pro.send_dir=data[len];//��λ
				S1_pro.send_data_len=S1_pro.card_num[S1_pro.send_dir][0]+1;//��һ����λ	
				S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
				S1_pro.send_dir=0;
				
				for(i=1;i<S1_pro.send_data_len;i++)
				{
				  S1_pro.send_data[i]=S1_pro.card_recv[data[len]][i];
				}
				
				//delay_ms(500);
				S1_TO_A83T();
                
				
				break;
			}
			case CMD_DICE_NUM: //  6=������						A83T-->S1
			{  
				printf("8888\r\n");
			    ////������4����λ��= ��λ+����+����+���� 
		        //if(S1_pro.PRO_step!=PRO_DICE_TO_A83T) return PTLC_FALSE;
                //data[len]����
                if(data[len]>20) 
					{printf("1111\r\n");return PTLC_FALSE;}//���ȹ��� 
                if(data[len+1]<0||data[len+1]>=4) {printf("2222\r\n");return PTLC_FALSE;}//��λ����
                dir=data[len+1];
                if(data[len+2]<=0||data[len+2]>=4) {printf("3333\r\n");return PTLC_FALSE;}//��ɫ��������4�β���
                if(data[len+3]<=0||data[len+3]>=4) {printf("4444\r\n");return PTLC_FALSE;}//ɫ�Ӹ�������3������
				PCTL_MemCopy(&S1_pro.card_dice[dir][0],&data[len+1],data[len]);//
				//���õ���������־
				S1_pro.card_dice[dir][CARD_DICE_REC_DIR]=CARD_DICE_REC_FLAG;
				printf("dir:%x rq dice flag\r\n",dir);
				break;
			}
			default :return PTLC_FALSE;

		  }
  	}
    else
  	{
	  //��ͷ	  
	  dataSend[len++]=0xFF;
	  dataSend[len++]=0xAA;
	  //��λ
	  dataSend[len++]=s_DIR;//
	  //CMD
      switch(S1_pro.PCLT_enum)
      {
     	case CMD_RESEND_CARD_NUM://1=�����ط��Ƶ�������			S1-->A83T
		{

			break;
			
		}
		case CMD_RESEND_SINGLE_CARD://	3=�������·������ƣ�			S1-->A83T
		{
			break;
		}
		case CMD_ALL_DIR_CARD://4=������λ���Ƶ����ݣ�			S1-->A83T������
		{
		    //dataSend[len++]=S1_pro.send_dir; 
			break;
		}
		case CMD_RESEND_DICE_NUM://7=�����ط�������				S1-->A83T
		{
			break;
		}
		case CMD_SHUFFLE_CARD_END://     8=ϴ�ƽ���:             S1-->A83T
		{
			break;
		}
		case CMD_CLEAR_PLAY_DICE:
		{
			break;
		}
		default :return PTLC_FALSE;
	  }
      dataSend[len++]=S1_pro.PCLT_enum;
	  //data len
      dataSend[len++]=S1_pro.send_data_len;
	  //data
	  PCTL_MemCopy(&dataSend[len],S1_pro.send_data,S1_pro.send_data_len);len+=S1_pro.send_data_len;
      //crc
      
     wCrc = GetCrc((const u8 *)dataSend,len);
     dataSend[len]= wCrc;
     dataSend[len+1] = wCrc >> 8;
	 len+=2;
#if SEND_DATA_PRINTF	
     if(S1_pro.PCLT_enum==CMD_ALL_DIR_CARD)
     {
       	 switch(S1_pro.send_data[0])
	     {
	       case 0:
		   printf("S��:");break;
		   case 1:
		   printf("S��:");break;
		   case 2:
		   printf("S��:");break;
		   case 3:
		   printf("S��:");break;
	     }
	 } 	
	 else
	 {
	     switch(S1_pro.send_dir)
	     {
	       case 0:
		   printf("S��:");break;
		   case 1:
		   printf("S��:");break;
		   case 2:
		   printf("S��:");break;
		   case 3:
		   printf("S��:");break;
	     }
	 	}
     for(i=0;i<len;i++) 
	 printf("%02x ",dataSend[i]);	 
	 printf("\r\n");
#endif   
	 
	 //�������� 1.�ĸ�����
/////////////////////////////
//usart2--dong 0  usart3--nan 1   usart4--xi 2  usart5--bei 3
//////////////////////////
#if 0
   S1_pro.send_usart=S1_pro.send_dir+2;//�涨�������
 #else  
   if(S1_pro.PCLT_enum==CMD_RESEND_CARD_NUM)
   	{
   	    for(i=0;i<4;i++)
   	    {
     		S1_pro.send_usart =i+2;S1_pro.uasrt_dir[S1_pro.send_dir][0];		
			sendData_usart(dataSend,len);
   	    }
   	}
	 // S1_pro.send_usart=S1_pro.send_dir+2;//�涨�������
	 else
	{
	    S1_pro.send_usart =S1_pro.uasrt_dir[S1_pro.send_dir][0];
   		sendData_usart(dataSend,len);
	}
	 #endif
   }
  return PTLC_TRUE;


}



//ϴ�ƽ����ź�
void washFinshSendCmd(void)
{
   u8 i=0;
   for(i=0;i<4;i++)
   {
    S1_pro.PCLT_enum=CMD_RESEND_CARD_NUM;
	S1_pro.send_dir=i;
	S1_pro.send_data_len=1;
	S1_pro.send_data[0]=0;
	S1_TO_A83T();
   }
}

//��������Ƿ���ȫ���գ�û��©�ƣ�������Ƶ������Ƿ���ȷ
//0.��ȷ
//1.��©������
//u8 card_recv[4][CAER_REC_NUM];//���ܵ��Ƶ����� �ĸ���λ
//u8 card_dice[4][4];//������4����λ��= ��λ+����+����
//u8 card_num[4][1];//4����λ�Ƶ�����


//0:OK
//1:NO


u8 checkCard(void)
{
  u8 i=0,j=0;
  switch(S1_pro.PRO_step)
  {
     case PRO_CARD_NUM_TO_A83T://����ĸ���λ������
     {
			for(i=0;i<CARD_DIRTION_NUM;i++)
			{
				 if(S1_pro.card_num[i][0]==0) 
				 {
						//�����ط�����
					S1_pro.PCLT_enum=CMD_RESEND_CARD_NUM;
					S1_pro.send_dir=i;
					S1_pro.send_data_len=1;
					S1_pro.send_data[0]=0;
					S1_TO_A83T();
				    return 1;
				 }				
		    }
			return 0;
			break;
	 }
	 case PRO_CARD_ALL_CARD_TO_A83T://
	 {
        //����ϴ�ƽ����ź�
    
		for(i=0;i<CARD_DIRTION_NUM;i++)
		{  
		    delay_ms(50);
		    if(S1_pro.card_recv[i][CAER_REC_NUM-2]!=CAER_REC_FLAG)
		    {
		     //������
			 //S1_pro.PCLT_enum=CMD_RESEND_SINGLE_CARD;
			 S1_pro.PCLT_enum=CMD_SHUFFLE_CARD_END;//
			 S1_pro.send_dir=i;
			 S1_pro.send_data_len=1;
			 S1_pro.send_data[0]=0;
			 S1_TO_A83T();
			 return 1;
			}
		}
	    return 0;
        break;
	 }
     case PRO_CARD_SEND_TO_A83T_DONG:
     {    
  	 	//��ѯ�����Ƿ�����
		for(i=0;i<CARD_DIRTION_NUM;i++)//����ĸ���λ����
		{
			for(j=1;j<=S1_pro.card_num[i][0];j++)//������Ƿ�Ϊ0
	        {
				if(S1_pro.card_recv[i][j]==0) //�鵽�����λ����û�н��յ�
				{
				  //���󷢵�����
				  S1_pro.PCLT_enum=CMD_RESEND_SINGLE_CARD;
				  S1_pro.send_dir=i;//��λ
				  S1_pro.send_data_len=1;
				  S1_pro.send_data[0]=j;
				  S1_TO_A83T();
				  return 1;//
				}
			}
		}
		return 0;
        break;
	 }
		 case PRO_DICE_TO_A83T:
	 {

       for(i=0;i<CARD_DIRTION_NUM;i++)
       {
          delay_ms(1000);
          if(S1_pro.card_dice[i][CARD_DICE_REC_DIR]!=CARD_DICE_REC_FLAG)
          {
                //�����ط�
              	S1_pro.PCLT_enum=CMD_RESEND_DICE_NUM;
				S1_pro.send_dir=0;//��λ
				S1_pro.send_data_len=1;
				S1_pro.send_data[0]=0;
				printf("=======\r\n");
				S1_TO_A83T(); 
				return 1;//
		  }
		  
	   }
	   return 0;
       break;
	 }
     case PRO_PLAY_DICE:
     {
          if(S1_pro.diceFishedFlag==1)//��ɫ���
          {
                //��ɫ���
              	S1_pro.PCLT_enum=CMD_CLEAR_PLAY_DICE;
				S1_pro.send_dir=i;//��λ
				S1_pro.send_data_len=1;
				S1_pro.send_data[0]=0;
				S1_TO_A83T(); 
				return 0;//
		  }
	   return 1;
       break;
	 }
	 case PRO_END:
	 {
       break; 
	 }
	 default: break;
	 
  }
  
   return 1;

}
void sendAllData(void)
{

switch(CARD_DIRTION_NUM)
{
	case 1://��
	{

		 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //���󷢵�����
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//��λ
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//��һ����λ	  
		  S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
		  S1_pro.send_dir=0;
		  
		  for(j=1;j<S1_pro.send_data_len;j++)
		  {
			S1_pro.send_data[j]=S1_pro.card_recv[0][j];
		  }

		  //delay_ms(500);
		  S1_TO_A83T();
		 
		 }
     break;
	}
	case 2://����
	{
	 	 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //���󷢵�����
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//��λ
		  if(S1_pro.send_dir==1)
		  S1_pro.send_data_len=S1_pro.card_num[1][0]+1;//��һ����λ	 
		  else
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//��һ����λ	
		  
		  S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
		  S1_pro.send_dir=0;
		  
		  if(S1_pro.send_data[0]==1)  
		  {
			  for(j=1;j<S1_pro.send_data_len;j++)
			  {
				S1_pro.send_data[j]=S1_pro.card_recv[1][j];
			  }
		  }
		  else
		  {
			  for(j=1;j<S1_pro.send_data_len;j++)
			  {
				S1_pro.send_data[j]=S1_pro.card_recv[0][j];
			  }
		  }

		  //delay_ms(500);
		  S1_TO_A83T();
		 
		 }

     break;
	}
	case 3://������
	{
		 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //���󷢵�����
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//��λ
		  if(S1_pro.send_dir==3)
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//��һ����λ	 
		  else
		  S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//��һ����λ	
		  
		  S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
		  S1_pro.send_dir=0;
		  
		  if(S1_pro.send_data[0]==3)  
		  {
			  for(j=1;j<S1_pro.send_data_len;j++)
			  {
				S1_pro.send_data[j]=S1_pro.card_recv[0][j];
			  }
		  }
		  else
		  {
			  for(j=1;j<S1_pro.send_data_len;j++)
			  {
				S1_pro.send_data[j]=S1_pro.card_recv[i][j];
			  }
		  }

		  //delay_ms(500);
		  S1_TO_A83T();
		 
		}
	 break;
    }
	case 4://��������
	{
		   u8 i=0,j=0;
		   for(i=0;i<4;i++)
		   {
		    //���󷢵�����
		    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		    S1_pro.send_dir=i;//��λ
		    S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//��һ����λ
			
			S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
			S1_pro.send_dir=0;//����ֻ��������λ
			for(j=1;j<S1_pro.send_data_len;j++)
			{
			  S1_pro.send_data[j]=S1_pro.card_recv[i][j];
			}
		    S1_TO_A83T();
		   
	    	}
		break;
	}		 
	defalut:
	{
    printf("��λ��������\r\n");
		break;
	}
	//�����ɫ����
	{
		u8 i=0,j=0;
		for(i=0;i<4;i++)
		{   
		    for(j=0;j<25;j++)
			S1_pro.card_dice[i][j]=0;
		}
	}
		

}
/*
   u8 i=0,j=0;
   for(i=0;i<4;i++)
   {
    //���󷢵�����
    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
    S1_pro.send_dir=i;//��λ
    S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//��һ����λ
	
	S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
	S1_pro.send_dir=0;//����ֻ��������λ
	for(j=1;j<S1_pro.send_data_len;j++)
	{

	  S1_pro.send_data[j]=S1_pro.card_recv[i][j];
	}
    S1_TO_A83T();

}

   u8 i=0,j=0,l=0;
   for(i=0;i<4;i++)
   {

    //���󷢵�����
    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
    S1_pro.send_dir=i;//��λ
    S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//��һ����λ
	
	S1_pro.send_data[0]=S1_pro.send_dir;//���ݵĵ�һ���Ƿ�λ
	S1_pro.send_dir=0;
	if(S1_pro.send_data[0]==1) 	
	{
		for(j=1;j<S1_pro.send_data_len;j++)
		{
		  S1_pro.send_data[j]=S1_pro.card_recv[1][j];
		}
	}
	else
	{
		for(j=1;j<S1_pro.send_data_len;j++)
		{
		  S1_pro.send_data[j]=S1_pro.card_recv[0][j];
		}
	}
	//delay_ms(500);
    S1_TO_A83T();

   }
   */
}





u8 S1_TO_A83T(void)
{
	return PTLC_PRO(0,NULL,0,0);
}
u8 A83T_TO_S1(u8 usart,u8 *data,u8 len)
{
  //return PTLC_PRO()
  return PTLC_PRO(usart,data,len,1);
}

static void S1_clearData(void)
{
   u8 i=0,j=0;
   for(i=0;i<4;i++)//ÿ����λ������
   {
   		S1_pro.card_num[i][0]=0;
   }
   
   for(i=0;i<4;i++)//ÿ����λ������
   {
        for(j=0;j<CAER_REC_NUM;j++)
	 	S1_pro.card_recv[i][j]=0;
   }
   for(i=0;i<4;i++)//���ܵĵ���
   {
        for(j=0;j<25;j++)
	 	S1_pro.card_dice[i][j]=0;
   }
   S1_pro.diceFishedFlag=0;
  // PCTL_MemSet(S1_pro,0,sizeof(S1_pro))
   printf("clear data\r\n");

}
void S1_Init(void)
{
  S1_pro.PRO_step=PRO_CARD_NUM_TO_A83T;
  S1_clearData();
}
//
//���ƴ�ɫ
static void power_work_pro_(u8 touzi_num)
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


//���ƴ�ɫ
//��λ1 2 3����ͬ��ɫ��ɫ��
//��λ������Ӧ��ֵ 1��6
static void power_work_pro(u8 *touzi_num,u8 len)
{
  static u16 crc;
  static u8 by[2];
  u8 i=0;
	
	/*ע�⣺ʹ��USART1ʱ����Ҫ�ر�PA8-PWM���*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  
	printf("num:%d\r\n",len);
    for(i=0;i<len;i++)
    {
       printf(" %2X ",touzi_num[i]);  
	}
	printf("\r\n");
	//rintf("\n\rtest num = %2X\n\r",touzi_num);/*���2λ16������*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*�����߳��*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//�ر�  				

	/*���͵�������*/
	str[0] = 0x55;
	str[1] = 0xab;
//
   switch(len)
   	{
   	 case 1://һ��ɫ��
   	 {
   	 		str[2] = touzi_num[0];//����A����4λ������B����4λ
			//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
			str[3]=0;
			srt[4]=0;
	 }
	 case 2://����ɫ��
	 {
	 		str[2] = touzi_num[0];//����A����4λ������B����4λ
			//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
			str[3]=touzi_num[1];
			srt[4]=0;
	 }
	 case 3://����ɫ��		
	 	{
			str[2] = touzi_num[0];//����A����4λ������B����4λ
			//ͨ��Э�飺0x55 0xAB [����] [У�����8λ] [У�����8λ]
			str[3]=touzi_num[1];
			srt[4]=touzi_num[2];
	 	}
   	}
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


//��ͨ��ɫ
static void normal_work_Pro(void)
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



//Ӳ�����
////////////////////Ӳ����־
//1.���߳��
//2.�¶Ȳɼ�
//3.���״̬
//4.������ȡ
//5.ϴ�ƽ����źŻ�ȡ

static void temp_pro(void)
{
	float temp=0;
    if(S1_pro.st_hardWare.tempCnt++>TEMP_TIMER_TICK)
    {
       S1_pro.st_hardWare.tempCnt=0;
       S1_pro.st_hardWare.tempValue=Get_Adc_Average(ADC_Channel_0,10);
	   temp=(float)S1_pro.st_hardWare.tempValue*(3.3/4096);
       if(temp<0.7)
       {
            S1_pro.st_hardWare.tempFlag=1;
			printf("�¶ȹ���\r\n");
			
	   }
	   else
	   {
            S1_pro.st_hardWare.tempFlag=0;
	   }
	}
}
/*
///����ʱ���ܻ����ϴ�ƽ���
//����ϴ�ƽ���
*/
static void washFinsh_pro(void)
{
    if(S1_pro.st_hardWare.washCnt++>WASH_TIMER_TICK)
    {
       S1_pro.st_hardWare.washCnt=0;
	   
	   if(stPwm.cardWashFlag==KEY_TRUE)
	   {
	       S1_pro.st_hardWare.washTimesCnt++;
		   if(S1_pro.st_hardWare.washTimesCnt==4)
		   	{
		   	 	S1_pro.st_hardWare.washTimes=0;
		   	   S1_pro.st_hardWare.washTimesCnt=0;
	      	 	S1_pro.st_hardWare.washedFalg=0;
           		printf("ϴ����\r\n");
		   	}
		   stPwm.cardWashFlag=KEY_FALSE;
		   
	   }
	   else
	   {
	   
	       S1_pro.st_hardWare.washTimes++;
		   if(S1_pro.st_hardWare.washTimes==2)
		   {
		      S1_pro.st_hardWare.washTimes=0;
		      S1_pro.st_hardWare.washTimesCnt=0;
	          S1_pro.st_hardWare.washedFalg=1;
			  printf("ϴ����\r\n");
		   }

         //printf("ϴ����\r\n");
	   }
	}

}
static void charge_pro(void)
{
	//if(S1_pro.st_hardWare.washCnt++&0x8000)/*��1?��*/
	/*���߳��test����8�룬��2�룬��˷�����ͬʱ����¶ȣ�*/
	if((S1_pro.st_hardWare.washCnt++)&0x8000)/*ʹ��*/
	{
		if((S1_pro.st_hardWare.washCnt&0X3FFF)%9000==0)//��8s֮��/9
		{
			S1_pro.st_hardWare.washCnt=0;/*��������*/
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
		if(S1_pro.st_hardWare.washCnt%500==0)//�ر�2s֮��/0.5
		{
			S1_pro.st_hardWare.washCnt=0;/*��������*/
			S1_pro.st_hardWare.washCnt|=0x8000;
			TIM_CtrlPWMOutputs(TIM1,ENABLE);//ʹ��
			set_pwm_flag(0);//ʹ��
		}
		else 
		{
			TIM_CtrlPWMOutputs(TIM1, DISABLE);//�ر� 
			set_pwm_flag(1);//�ر� 
		}	
	}
    //�������3����
    if((S1_pro.st_hardWare.chargeCnt++)==DICE_TIMER_TICK)
	{
	   S1_pro.st_hardWare.chargeFlag=1;//ÿ�δ���ɫҪ�����
	}

}

static void control_dice_pro(void)
{
    if(MOTOR_IN_Check()==0)//��ȡ΢�����״̬��΢�����ת������ʼ��ɫ
    {
        if(S1_pro.st_hardWare.keyGetFlag==1&&S1_pro.diceTimes!=S1_pro.diceTimesBack)//��������
        {
			S1_pro.st_hardWare.keyGetValue=S1_pro.st_hardWare.keylastValue;//ֱ�ӻ�ȡ�ϴε�ֵ
		}
		if(S1_pro.st_hardWare.keyGetValue>=1&&S1_pro.st_hardWare.keyGetValue<=4)
		{
			//�ж��Ƿ��һ�δ�ɫ�¶�������
			if(S1_pro.diceTimes==S1_pro.diceTimesBack)//��һ�δ�ɫ		
			{
			    S1_pro.diceTimes--;
				S1_pro.st_hardWare.keylastValue=S1_pro.st_hardWare.keyGetValue ;
				if(S1_pro.st_hardWare.tempFlag==1||S1_pro.st_hardWare.chargeFlag==1)//�¶ȹ��ߣ���粻����ͨ��ɫ
				{
				   S1_pro.diceContolFlag=0;
				   normal_work_Pro();

				}
				else//�¶����������ƴ�ɫ
				{
					S1_pro.diceContolFlag=1;
					//����A����4λ������B����4λ
					//power_work_pro();
				}
			}
			else//���ǵ�һ�δ�ɫ
			{
			    S1_pro.diceTimes--;
				//�жϵ�һ���Ƿ�������ɫ
				if(S1_pro.diceContolFlag==0)
				{
					normal_work_Pro();
				}
				else//
				{   
				    //����A����4λ������B����4λ
					//power_work_pro();
				}
				
			}
			if(S1_pro.diceTimes==0) //��ɫ���
			{
				  S1_pro.diceFishedFlag=1;

				  S1_pro.diceTimesBack=0;//���

		      }
		}
	}
   
}


void clearDirData(void)//����ĸ���λ������
{
   u8 i=0,j=0;
   for(i=0;i<4;i++)//ÿ����λ������
   {
        for(j=0;j<CAER_REC_NUM;j++)
	 	S1_pro.card_recv[i][j]=0;
   }
}
/***
//��ɫ
//�����ɫ��������2�Σ�һ�δ�ɫҪ���ݰ���
//�´δ�ɫ��ֻ������ĵ�ƽ���жϴ�ɫ
***/

static void keyGet_pro(void)
{
   u8 i=0;
   #if 0
    u8 i=0;
	for(i=0;i<6;i++)
	{
		if(stPwm.keyValue[i])
		{

		   if(S1_pro.st_hardWare.keyGetFlag==0) 
		   	{

			   if(i<=3)
			   	{
			  		S1_pro.st_hardWare.keyGetValue=i+1;//
			  		//���������� �Ϻͱ�����
					if(S1_pro.st_hardWare.keyGetValue==1) S1_pro.st_hardWare.keyGetValue=3;
					//else if(S1_pro.st_hardWare.keyGetValue==2)S1_pro.st_hardWare.keyGetValue=4;//�Ϻͱ�����
					else if(S1_pro.st_hardWare.keyGetValue==3) S1_pro.st_hardWare.keyGetValue=1;
					//else if(S1_pro.st_hardWare.keyGetValue==4)S1_pro.st_hardWare.keyGetValue=2;//�Ϻͱ�����
			   	}
		   }
		   //se
		   //
           //1_pro.st_hardWare.keyGetValue=S1_pro.st_hardWare.keylastValue;  
		   //
		    printf("��������:%d\r\n",stPwm.keyValue[i]);
		    stPwm.keyValue[i]=0;
		}


      
	}
	#else
	//ֻ���ڴ�ɫ��ʱ���°���������Ч��
	//
	if()
	i=KEY_Read();
	if(i==0) 
	{
		
	}
	else
	{
		printf("��������:%d\r\n",i);
	}
	

	#endif
   
}
//������״̬����
void S1_HARDWARE_Pro(void)
{
    
	systick_delay(1);
    //1.���߳��
    charge_pro();
	//2.�¶Ȳɼ�
	temp_pro();
  
	//3.���״̬
	control_dice_pro();
	//4.������ȡ
	keyGet_pro();
	//5.ϴ�ƽ����źŻ�ȡ
	washFinsh_pro();
	

}
//��ʾ���ܵ���ɫ��
static void displayDice(void)
{
   u8 i=0,j=0;
   u8 str[4][5]={"��","��","��","��"};
   printf("��ɫ:\r\n");
   for(i=0;i<CARD_DIRTION_NUM;i++)
   {
      printf("%s:",str[i]);
      for(j=0;j<25;j++)
	  	printf("%x ",S1_pro.card_dice[i][j]);
	  	printf("\r\n");
   }

}

//Э�鴦��
void S1_CTRL_PrO(void)
{

#if PCTL_PRO_ONLY//
   static u32 time=0;
   switch(S1_pro.PRO_step)
   {
        case PRO_CARD_NUM_TO_A83T://�����Ƶ�����
        {

		          if(S1_pro.timeoutCnt%500==0)//500ms
		         {
		              printf("PRO_CARD_NUM_TO_A83T\r\n");
					  if(checkCard()==1)//
					  {

					  }
					  else//
					  {
					      //S1_pro.card_recv[i][CAER_REC_NUM-2]
			              S1_pro.PRO_step=PRO_CARD_ALL_CARD_TO_A83T; 
						  //�������
						  clearDirData();
						  time=S1_pro.timeoutCnt;
					  }
		          }
          
          break;
		}

        case PRO_CARD_ALL_CARD_TO_A83T://�����ĸ���λ����
       	{
			  
       	      if((S1_pro.timeoutCnt-time)>2000) 
			  {
			      time=S1_pro.timeoutCnt;
				  printf("PRO_CARD_ALL_CARD_TO_A83T\r\n");
				  if(checkCard()==1)//
				  {

				  }
				  else//
				  {
		              S1_pro.PRO_step=PRO_CARD_SEND_TO_A83T_DONG;   
				  }
       	      }
          break;
	    }
  	    case PRO_CARD_SEND_TO_A83T_DONG://��������λ
  	   	{
  	   	  if(S1_pro.timeoutCnt%20==0)//1s
  	   	  	{
			  printf("PRO_CARD_SEND_TO_A83T_DONG\r\n");
	          //��������Ƿ�����         
			  if(checkCard()==1)//������©��
			  {
					
			  }
			  else//����û��©��
			  {
			    //����ȫ������		 
			    sendAllData();
				S1_pro.PRO_step=PRO_DICE_TO_A83T;	
				//
			  }

  	   	  	}
          break;
	    }
  	    case PRO_DICE_TO_A83T://�������(ң�ؿ���֪��ׯ��)
  	   	{

		 if(S1_pro.timeoutCnt%1000==0) 
		 {
				  printf("PRO_DICE_TO_A83T\r\n");
				  if(checkCard()==1)
				  {

				  }
				  else
				  {
						S1_pro.diceFishedFlag=0;
		            S1_pro.PRO_step=PRO_PLAY_DICE;     
				  }
		 }
          break;
		  
	    }
  	   case PRO_PLAY_DICE://���ܵ��������ɫ ��ɫ������Ҫ���������A83T
  	   {
		 if(S1_pro.timeoutCnt%1000==0) 
		 {
		   printf("PRO_PLAY_DICE\r\n");
			 if(checkCard()==1)
			 {
			   displayDice();//��ʾ���
			   S1_pro.diceFishedFlag=1;
			 
			 }
			 else
			 {
			    
		     	S1_pro.PRO_step=PRO_END;
				time=S1_pro.timeoutCnt;
			 }
		 	}
         break;
	   }
  	   case PRO_END:
  	   {
  	   
		if((S1_pro.timeoutCnt-time)%5000==0) 
		{
			   time=S1_pro.timeoutCnt;
		       printf("PRO_END\r\n");
                //�������
		       S1_clearData();
			  // S1_pro.PRO_step=PRO_CARD_NUM_TO_A83T;
		}
		

	  
         break;
       }
	   default:
	   {
	   	 break;
	   }
   }
#else
	static u8 time=0;
	switch(S1_pro.PRO_step)
	{
		 case PRO_CARD_NUM_TO_A83T://�����Ƶ�����
		 {//////
		
		   if(S1_pro.timeoutCnt%500==0)//500ms
		  {
			   printf("PRO_CARD_NUM_TO_A83T\r\n");

			    if(S1_pro.st_hardWare.washedFalg==0)//ϴ�Ƶ���ʱ������ϴ��
			   	{
				   if(checkCard()==1)//
				   {
		
				   }
				   else//
				   {
					   //S1_pro.card_recv[i][CAER_REC_NUM-2]
					   S1_pro.PRO_step=PRO_CARD_ALL_CARD_TO_A83T;
					   S1_pro.st_hardWare.washedFalg=0;
					   	//�������
					   clearDirData();
					   time=S1_pro.timeoutCnt;
				   }
		   		}
		}
		
		   break;
		 }
	     
		 case PRO_CARD_ALL_CARD_TO_A83T://�����ĸ���λ����
		 {
		      // printf("PRO_CARD_ALL_CARD_TO_A83T\r\n");
			  // time=S1_pro.timeoutCnt;
		       if(S1_pro.st_hardWare.washedFalg==1)//�ж��Ƿ�ϴ�ƽ���
		       {
		           
				   if(S1_pro.timeoutCnt-time>2000) 
				   {
				       time=S1_pro.timeoutCnt;
					   printf("PRO_CARD_ALL_CARD_TO_A83T\r\n");
					   if(checkCard()==1)//
					   {
		
					   }
					   else//
					   {
						   S1_pro.PRO_step=PRO_CARD_SEND_TO_A83T_DONG;
						   S1_pro.st_hardWare.washedFalg=0;//�����־
					   }
				   }
		       }
			break;
		 }
		 case PRO_CARD_SEND_TO_A83T_DONG://��������λ
		 {
		   if(S1_pro.timeoutCnt%30==0)//1s
			 {
			   printf("PRO_CARD_SEND_TO_A83T_DONG\r\n");
			   //��������Ƿ�����		  
			   if(checkCard()==1)//������©��
			   {
					 
			   }
			   else//����û��©��
			   {
			     //������ڵ�����
			     init_rgb();
				 
				 //����ȫ������		  
				 sendAllData();
				 //
				 S1_pro.PRO_step=PRO_DICE_TO_A83T;			 
			   }
	
			 }
		   break;
		 }
		 case PRO_DICE_TO_A83T://�������(ң�ؿ���֪��ׯ�� ����λ��ɫ)
		 {
	
		  if(S1_pro.timeoutCnt%1000==0) 
		  {
				   printf("PRO_DICE_TO_A83T\r\n");
				   if(checkCard()==1)
				   {
	
				   }
				   else
				   {
				    S1_pro.diceFishedFlag=0;
					 S1_pro.PRO_step=PRO_PLAY_DICE; 	
				   }
		  }
		   break;
		   
		 }
		case PRO_PLAY_DICE://���ܵ��������ɫ ��ɫ������Ҫ���������A83T
		{
		  
		  if(S1_pro.timeoutCnt%1000==0) 
		  {
			  printf("PRO_PLAY_DICE\r\n");
			  if(checkCard()==1)
			  {
				displayDice();//��ʾ���
				S1_pro.diceFishedFlag=1;
			  
			  }
			  else
			  {
				 //S1_pro.diceFishedFlag=0;
				 S1_pro.PRO_step=PRO_END;
				 
			  }
			 }
		  break;
		}
		case PRO_END:
		{
		
		 if(S1_pro.timeoutCnt%5000==0) 
		 {
			
	
				printf("PRO_END\r\n");
				 //�������
				S1_clearData();
				 if(S1_pro.st_hardWare.washedFalg==0)//ϴ�Ƶ�������һ������
				 {
				      S1_pro.PRO_step=PRO_CARD_NUM_TO_A83T;
				 }
		 }
		 
	
	   
		  break;
		}
		default:
		{
		  break;
		}
	}


#endif
}


















