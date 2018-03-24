#include "pro.h"

//
S1_PRO_STRUCT S1_pro;//

//A83T zh:console mm:brave
//echo test2.winning8.com > /data/improcess/server && reboot

//包头			方位	CMD		长度	数据	结尾
//0xff 0xaa		0-4		0-7		len		data	CRC16
//方位0-4：东南西北S1
//
/*
方位0-4：东南西北S1
CMD 0-7：
0=该方位的牌的数量；			A83T-->S1
1=请求重发牌的数量；			S1-->A83T
2=发单个牌；					A83T-->S1
3=请求重新发单个牌；			S1-->A83T
4=整个方位的牌的数据；			S1-->A83T（东）
5=请求重发整个方位的牌的数据；	A83T-->S1
6=点数；						A83T-->S1
7=请求重发点数；				S1-->A83T
typedef enum
{
  CMD_CARD_NUM =0,//            0=该方位的牌的数量；			A83T-->S1
  CMD_RESEND_CARD_NUM =1,//		S1-->A83T
  CMD_SINGLE_CARD=2,//			2=发单个牌；					A83T-->S1
  CMD_RESEND_SINGLE_CARD=3,//	3=请求重新发单个牌；			S1-->A83T
  CMD_ALL_DIR_CARD=4,//         4=整个方位的牌的数据；			S1-->A83T（东）
  CMD_RESEND_ALL_DIR_CARD=5,//  5=请求重发整个方位的牌的数据；	A83T-->S1
  CMD_DICE_NUM=6,//				6=点数；						A83T-->S1
  CMD_RESEND_DICE_NUM=7,//		7=请求重发点数；				S1-->A83T
}PCLT_ENUM;//协议枚举
*/

//////////////////////////
//获取16位校验码

//校验命令的完整性
//crc = GetCrc((const u8 *)pData,5+pData[4]);
//by[0] = crc;
//by[1] = crc >> 8;
//if((by[0]==pData[4+pData[4]+1]) && (by[1]==pData[4+pData[4]+2]))/*CRC16校验正确*/


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
					while((USART1->SR&0X40)==0);//等待发送结束
				}				
			break;
		case USART2_TYPE:
			   printf("USART2_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					USART2->DR=USART_TX_BUF[t];
					while((USART2->SR&0X40)==0);//等待发送结束
				}
			break;
		
		case USART3_TYPE:
			printf("USART3_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					USART3->DR=USART_TX_BUF[t];
					while((USART3->SR&0X40)==0);//等待发送结束
				}
			break;
		
		case USART4_TYPE:
			printf("USART4_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					UART4->DR=USART_TX_BUF[t];
					while((UART4->SR&0X40)==0);//等待发送结束
				}
			break;
		
		case USART5_TYPE:
			printf("USART5_TYPE\r\n");
				for(t=0;t<len;t++)
				{
					UART5->DR=USART_TX_BUF[t];
					while((UART5->SR&0X40)==0);//等待发送结束
				}
			break;			
		default:break;/*error*/	    
   }
   
}


//包头			方位   	CMD		长度  	数据  	结尾
//0xff 0xaa		0-4		0-7		len		data	CRC16
//crc = GetCrc((const u8 *)pData,5+pData[4]);
//by[0] = crc;
//by[1] = crc >> 8;

//	u8 t=0;
//	u16 crc;
//	u8 by[2];		
	//校验命令的完整性
	//crc = GetCrc((const u8 *)pData,5+pData[4]);
	//by[0] = crc;
	//by[1] = crc >> 8;
	//if((by[0]==pData[4+pData[4]+1]) && (by[1]==pData[4+pData[4]+2]))/*CRC16校验正确*/
//0 s1-->a83t
//1 a83t-->s1
static u8 PTLC_PRO(u8 uart,u8 *data,u8 plen,u8 mode)
{
  
  u16 wCrc;
  u8 by[2];
  u8 len=0,i=0;
  u8 dataLen=0;
  u8 dir=0,CMD=0;//方位
  u8 dataSend[200]={0};
  if(mode)//a83t-->s1
  {
		  if(data[len]==0xff&&data[len+1]==0xaa) {len+=2;}
		  else {printf("head error\r\n");return PTLC_FALSE;}
		  dir=data[len++];//自动标记是哪个串口
		  if(dir>=4) {printf("dir error\r\n");return PTLC_FALSE;}
		  else {
              S1_pro.uasrt_dir[dir][0]=uart;
		  }
		  
		  //校验
 #if REC_DATA_PRINTF
		  {
		  u8 b=0;
		  switch(dir)
	      {
	       case 0:
		   printf("R东:");break;
		   case 1:
		   printf("R南:");break;
		   case 2:
		   printf("R西:");break;
		   case 3:
		   printf("R北:");break;
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
	      if((by[0]==data[4+data[4]+1]) && (by[1]==data[4+data[4]+2])){}//检验成功
		  else {printf("crc error--\r\n"); return PTLC_FALSE;}//校验错误
		  //if(PTLC_GetCrc((const u8 *)data,5+data[4])==PTLC_FALSE) return PTLC_FALSE;//检验错误  
		  CMD=data[len++];
		  switch(CMD)
		  {
		    case CMD_CARD_NUM://   0=该方位的牌的数量；			A83T-->S1
		    {
		        //if(S1_pro.PRO_step!=PRO_CARD_NUM_TO_A83T) return PTLC_FALSE;
				if(data[len++]!=1) return PTLC_FALSE;//判断数据
		        S1_pro.card_num[dir][0]=data[len++];//牌的数量
		        
		        S1_pro.card_recv[dir][CAER_REC_NUM-2]=0; 
		        //printf("====dir:%x num:%x===\r\n",dir,S1_pro.card_num[dir][0]);
		        //S1_pro.card_num_cnt++;
				break;
			}
			case CMD_SINGLE_CARD://2=发单个牌；					A83T-->S1
			{
			   // printf("1--\r\n");
			    
				//if(S1_pro.PRO_step!=PRO_CARD_ALL_CARD_TO_A83T&&S1_pro.PRO_step!=PRO_CARD_SEND_TO_A83T_DONG) return PTLC_FALSE;	
			    if(data[len++]!=2) return PTLC_FALSE;//判断数据			    
			    S1_pro.card_recv[dir][data[len]]=data[len+1];len+=2;//单个牌数
			    if(S1_pro.card_recv[dir][CAER_REC_NUM-2]!=CAER_REC_FLAG)
			    {
                   S1_pro.card_recv[dir][CAER_REC_NUM-2]=CAER_REC_FLAG; 
				   printf("dir:%x rq card flag\r\n",dir);
				}
			    
				break;
			}
			case CMD_RESEND_ALL_DIR_CARD://5=请求重发整个方位的牌的数据；	A83T-->S1
			{
				if(S1_pro.PRO_step<=PRO_CARD_SEND_TO_A83T_DONG) 
				{
                    printf("请求单个方位的数据无效\r\n");
					return PTLC_FALSE;
				}
				//
				
                if(data[len++]!=1) return PTLC_FALSE;//数据长度不对                
			    printf("请求整个方位的数据\r\n");
				//for(i=0;i<S1_pro.card_num[data[len]];i++)					
				S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
				S1_pro.send_dir=data[len];//方位
				S1_pro.send_data_len=S1_pro.card_num[S1_pro.send_dir][0]+1;//多一个方位	
				S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
				S1_pro.send_dir=0;
				
				for(i=1;i<S1_pro.send_data_len;i++)
				{
				  S1_pro.send_data[i]=S1_pro.card_recv[data[len]][i];
				}
				
				//delay_ms(500);
				S1_TO_A83T();
                
				
				break;
			}
			case CMD_DICE_NUM: //  6=点数；						A83T-->S1
			{  
				printf("8888\r\n");
			    ////点数（4个方位）= 方位+次数+个数+点数 
		        //if(S1_pro.PRO_step!=PRO_DICE_TO_A83T) return PTLC_FALSE;
                //data[len]长度
                if(data[len]>20) 
					{printf("1111\r\n");return PTLC_FALSE;}//长度过长 
                if(data[len+1]<0||data[len+1]>=4) {printf("2222\r\n");return PTLC_FALSE;}//方位不对
                dir=data[len+1];
                if(data[len+2]<=0||data[len+2]>=4) {printf("3333\r\n");return PTLC_FALSE;}//打色次数大于4次不对
                if(data[len+3]<=0||data[len+3]>=4) {printf("4444\r\n");return PTLC_FALSE;}//色子个数超过3个错误
				PCTL_MemCopy(&S1_pro.card_dice[dir][0],&data[len+1],data[len]);//
				//设置点数结束标志
				S1_pro.card_dice[dir][CARD_DICE_REC_DIR]=CARD_DICE_REC_FLAG;
				printf("dir:%x rq dice flag\r\n",dir);
				break;
			}
			default :return PTLC_FALSE;

		  }
  	}
    else
  	{
	  //包头	  
	  dataSend[len++]=0xFF;
	  dataSend[len++]=0xAA;
	  //方位
	  dataSend[len++]=s_DIR;//
	  //CMD
      switch(S1_pro.PCLT_enum)
      {
     	case CMD_RESEND_CARD_NUM://1=请求重发牌的数量；			S1-->A83T
		{

			break;
			
		}
		case CMD_RESEND_SINGLE_CARD://	3=请求重新发单个牌；			S1-->A83T
		{
			break;
		}
		case CMD_ALL_DIR_CARD://4=整个方位的牌的数据；			S1-->A83T（东）
		{
		    //dataSend[len++]=S1_pro.send_dir; 
			break;
		}
		case CMD_RESEND_DICE_NUM://7=请求重发点数；				S1-->A83T
		{
			break;
		}
		case CMD_SHUFFLE_CARD_END://     8=洗牌结束:             S1-->A83T
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
		   printf("S东:");break;
		   case 1:
		   printf("S南:");break;
		   case 2:
		   printf("S西:");break;
		   case 3:
		   printf("S北:");break;
	     }
	 } 	
	 else
	 {
	     switch(S1_pro.send_dir)
	     {
	       case 0:
		   printf("S东:");break;
		   case 1:
		   printf("S南:");break;
		   case 2:
		   printf("S西:");break;
		   case 3:
		   printf("S北:");break;
	     }
	 	}
     for(i=0;i<len;i++) 
	 printf("%02x ",dataSend[i]);	 
	 printf("\r\n");
#endif   
	 
	 //发送数据 1.哪个串口
/////////////////////////////
//usart2--dong 0  usart3--nan 1   usart4--xi 2  usart5--bei 3
//////////////////////////
#if 0
   S1_pro.send_usart=S1_pro.send_dir+2;//规定串口配对
 #else  
   if(S1_pro.PCLT_enum==CMD_RESEND_CARD_NUM)
   	{
   	    for(i=0;i<4;i++)
   	    {
     		S1_pro.send_usart =i+2;S1_pro.uasrt_dir[S1_pro.send_dir][0];		
			sendData_usart(dataSend,len);
   	    }
   	}
	 // S1_pro.send_usart=S1_pro.send_dir+2;//规定串口配对
	 else
	{
	    S1_pro.send_usart =S1_pro.uasrt_dir[S1_pro.send_dir][0];
   		sendData_usart(dataSend,len);
	}
	 #endif
   }
  return PTLC_TRUE;


}



//洗牌结束信号
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

//检查数据是否完全接收，没有漏牌，不检测牌的数据是否正确
//0.正确
//1.有漏牌现象
//u8 card_recv[4][CAER_REC_NUM];//接受到牌的数量 四个方位
//u8 card_dice[4][4];//点数（4个方位）= 方位+次数+点数
//u8 card_num[4][1];//4个方位牌的数量


//0:OK
//1:NO


u8 checkCard(void)
{
  u8 i=0,j=0;
  switch(S1_pro.PRO_step)
  {
     case PRO_CARD_NUM_TO_A83T://检测四个方位的牌数
     {
			for(i=0;i<CARD_DIRTION_NUM;i++)
			{
				 if(S1_pro.card_num[i][0]==0) 
				 {
						//请求重发牌数
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
        //发送洗牌结束信号
    
		for(i=0;i<CARD_DIRTION_NUM;i++)
		{  
		    delay_ms(50);
		    if(S1_pro.card_recv[i][CAER_REC_NUM-2]!=CAER_REC_FLAG)
		    {
		     //请求发牌
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
  	 	//查询数据是否完整
		for(i=0;i<CARD_DIRTION_NUM;i++)//检查四个方位的牌
		{
			for(j=1;j<=S1_pro.card_num[i][0];j++)//检测牌是否为0
	        {
				if(S1_pro.card_recv[i][j]==0) //查到这个方位的牌没有接收到
				{
				  //请求发单个牌
				  S1_pro.PCLT_enum=CMD_RESEND_SINGLE_CARD;
				  S1_pro.send_dir=i;//方位
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
                //请求重发
              	S1_pro.PCLT_enum=CMD_RESEND_DICE_NUM;
				S1_pro.send_dir=0;//方位
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
          if(S1_pro.diceFishedFlag==1)//打色完成
          {
                //打色完成
              	S1_pro.PCLT_enum=CMD_CLEAR_PLAY_DICE;
				S1_pro.send_dir=i;//方位
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
	case 1://东
	{

		 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //请求发单个牌
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//方位
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//多一个方位	  
		  S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
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
	case 2://东南
	{
	 	 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //请求发单个牌
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//方位
		  if(S1_pro.send_dir==1)
		  S1_pro.send_data_len=S1_pro.card_num[1][0]+1;//多一个方位	 
		  else
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//多一个方位	
		  
		  S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
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
	case 3://东南西
	{
		 u8 i=0,j=0;
		 for(i=0;i<4;i++)
		 {
		 
		  //请求发单个牌
		  S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		  S1_pro.send_dir=i;//方位
		  if(S1_pro.send_dir==3)
		  S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//多一个方位	 
		  else
		  S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//多一个方位	
		  
		  S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
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
	case 4://东南西北
	{
		   u8 i=0,j=0;
		   for(i=0;i<4;i++)
		   {
		    //请求发单个牌
		    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
		    S1_pro.send_dir=i;//方位
		    S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//多一个方位
			
			S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
			S1_pro.send_dir=0;//数据只发给东方位
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
    printf("方位个数错误\r\n");
		break;
	}
	//清除打色点数
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
    //请求发单个牌
    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
    S1_pro.send_dir=i;//方位
    S1_pro.send_data_len=S1_pro.card_num[i][0]+1;//多一个方位
	
	S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
	S1_pro.send_dir=0;//数据只发给东方位
	for(j=1;j<S1_pro.send_data_len;j++)
	{

	  S1_pro.send_data[j]=S1_pro.card_recv[i][j];
	}
    S1_TO_A83T();

}

   u8 i=0,j=0,l=0;
   for(i=0;i<4;i++)
   {

    //请求发单个牌
    S1_pro.PCLT_enum=CMD_ALL_DIR_CARD;
    S1_pro.send_dir=i;//方位
    S1_pro.send_data_len=S1_pro.card_num[0][0]+1;//多一个方位
	
	S1_pro.send_data[0]=S1_pro.send_dir;//数据的第一个是方位
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
   for(i=0;i<4;i++)//每个方位的牌数
   {
   		S1_pro.card_num[i][0]=0;
   }
   
   for(i=0;i<4;i++)//每个方位的数据
   {
        for(j=0;j<CAER_REC_NUM;j++)
	 	S1_pro.card_recv[i][j]=0;
   }
   for(i=0;i<4;i++)//接受的点数
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
//控制打色
static void power_work_pro_(u8 touzi_num)
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


//控制打色
//高位1 2 3代表不同的色的色子
//低位代表相应的值 1到6
static void power_work_pro(u8 *touzi_num,u8 len)
{
  static u16 crc;
  static u8 by[2];
  u8 i=0;
	
	/*注意：使用USART1时必须要关闭PA8-PWM输出*/
	//TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  
	printf("num:%d\r\n",len);
    for(i=0;i<len;i++)
    {
       printf(" %2X ",touzi_num[i]);  
	}
	printf("\r\n");
	//rintf("\n\rtest num = %2X\n\r",touzi_num);/*输出2位16进制数*/		
	printf("\n\rMOTOR_IN_Check ok!\n\r");
	
	/*关无线充电*/
	TIM_CtrlPWMOutputs(TIM1, DISABLE);  	//关闭  				

	/*发送点数命令*/
	str[0] = 0x55;
	str[1] = 0xab;
//
   switch(len)
   	{
   	 case 1://一个色子
   	 {
   	 		str[2] = touzi_num[0];//骰子A：低4位；骰子B：高4位
			//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
			str[3]=0;
			srt[4]=0;
	 }
	 case 2://两个色子
	 {
	 		str[2] = touzi_num[0];//骰子A：低4位；骰子B：高4位
			//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
			str[3]=touzi_num[1];
			srt[4]=0;
	 }
	 case 3://三个色子		
	 	{
			str[2] = touzi_num[0];//骰子A：低4位；骰子B：高4位
			//通信协议：0x55 0xAB [数字] [校验码低8位] [校验码高8位]
			str[3]=touzi_num[1];
			srt[4]=touzi_num[2];
	 	}
   	}
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


//普通打色
static void normal_work_Pro(void)
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



//硬件检测
////////////////////硬件标志
//1.无线充电
//2.温度采集
//3.马达状态
//4.按键获取
//5.洗牌结束信号获取

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
			printf("温度过高\r\n");
			
	   }
	   else
	   {
            S1_pro.st_hardWare.tempFlag=0;
	   }
	}
}
/*
///开机时可能会出现洗牌结束
//灯灭洗牌结束
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
           		printf("洗牌亮\r\n");
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
			  printf("洗牌灭\r\n");
		   }

         //printf("洗牌灭\r\n");
	   }
	}

}
static void charge_pro(void)
{
	//if(S1_pro.st_hardWare.washCnt++&0x8000)/*ê1?ü*/
	/*无线充电test：开8秒，关2秒，如此反复（同时检测温度）*/
	if((S1_pro.st_hardWare.washCnt++)&0x8000)/*使能*/
	{
		if((S1_pro.st_hardWare.washCnt&0X3FFF)%9000==0)//开8s之后/9
		{
			S1_pro.st_hardWare.washCnt=0;/*数据清零*/
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
		if(S1_pro.st_hardWare.washCnt%500==0)//关闭2s之后/0.5
		{
			S1_pro.st_hardWare.washCnt=0;/*数据清零*/
			S1_pro.st_hardWare.washCnt|=0x8000;
			TIM_CtrlPWMOutputs(TIM1,ENABLE);//使能
			set_pwm_flag(0);//使能
		}
		else 
		{
			TIM_CtrlPWMOutputs(TIM1, DISABLE);//关闭 
			set_pwm_flag(1);//关闭 
		}	
	}
    //开机充电3分钟
    if((S1_pro.st_hardWare.chargeCnt++)==DICE_TIMER_TICK)
	{
	   S1_pro.st_hardWare.chargeFlag=1;//每次打完色要清除掉
	}

}

static void control_dice_pro(void)
{
    if(MOTOR_IN_Check()==0)//获取微型马达状态：微型马达转动，开始打色
    {
        if(S1_pro.st_hardWare.keyGetFlag==1&&S1_pro.diceTimes!=S1_pro.diceTimesBack)//按键按下
        {
			S1_pro.st_hardWare.keyGetValue=S1_pro.st_hardWare.keylastValue;//直接获取上次的值
		}
		if(S1_pro.st_hardWare.keyGetValue>=1&&S1_pro.st_hardWare.keyGetValue<=4)
		{
			//判断是否第一次打色温度正常不
			if(S1_pro.diceTimes==S1_pro.diceTimesBack)//第一次打色		
			{
			    S1_pro.diceTimes--;
				S1_pro.st_hardWare.keylastValue=S1_pro.st_hardWare.keyGetValue ;
				if(S1_pro.st_hardWare.tempFlag==1||S1_pro.st_hardWare.chargeFlag==1)//温度过高，充电不够普通打色
				{
				   S1_pro.diceContolFlag=0;
				   normal_work_Pro();

				}
				else//温度正常，控制打色
				{
					S1_pro.diceContolFlag=1;
					//骰子A：低4位；骰子B：高4位
					//power_work_pro();
				}
			}
			else//不是第一次打色
			{
			    S1_pro.diceTimes--;
				//判断第一次是否正常打色
				if(S1_pro.diceContolFlag==0)
				{
					normal_work_Pro();
				}
				else//
				{   
				    //骰子A：低4位；骰子B：高4位
					//power_work_pro();
				}
				
			}
			if(S1_pro.diceTimes==0) //打色完成
			{
				  S1_pro.diceFishedFlag=1;

				  S1_pro.diceTimesBack=0;//清除

		      }
		}
	}
   
}


void clearDirData(void)//清除四个方位的数据
{
   u8 i=0,j=0;
   for(i=0;i<4;i++)//每个方位的数据
   {
        for(j=0;j<CAER_REC_NUM;j++)
	 	S1_pro.card_recv[i][j]=0;
   }
}
/***
//打色
//如果打色次数超过2次，一次打色要根据按键
//下次打色就只检测电机的电平来判断打色
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
			  		//东和西互换 南和北互换
					if(S1_pro.st_hardWare.keyGetValue==1) S1_pro.st_hardWare.keyGetValue=3;
					//else if(S1_pro.st_hardWare.keyGetValue==2)S1_pro.st_hardWare.keyGetValue=4;//南和北不换
					else if(S1_pro.st_hardWare.keyGetValue==3) S1_pro.st_hardWare.keyGetValue=1;
					//else if(S1_pro.st_hardWare.keyGetValue==4)S1_pro.st_hardWare.keyGetValue=2;//南和北不换
			   	}
		   }
		   //se
		   //
           //1_pro.st_hardWare.keyGetValue=S1_pro.st_hardWare.keylastValue;  
		   //
		    printf("按键按下:%d\r\n",stPwm.keyValue[i]);
		    stPwm.keyValue[i]=0;
		}


      
	}
	#else
	//只有在打色的时候按下按键才是有效的
	//
	if()
	i=KEY_Read();
	if(i==0) 
	{
		
	}
	else
	{
		printf("按键按下:%d\r\n",i);
	}
	

	#endif
   
}
//机器的状态处理
void S1_HARDWARE_Pro(void)
{
    
	systick_delay(1);
    //1.无线充电
    charge_pro();
	//2.温度采集
	temp_pro();
  
	//3.马达状态
	control_dice_pro();
	//4.按键获取
	keyGet_pro();
	//5.洗牌结束信号获取
	washFinsh_pro();
	

}
//显示接受到打色的
static void displayDice(void)
{
   u8 i=0,j=0;
   u8 str[4][5]={"东","南","西","北"};
   printf("打色:\r\n");
   for(i=0;i<CARD_DIRTION_NUM;i++)
   {
      printf("%s:",str[i]);
      for(j=0;j<25;j++)
	  	printf("%x ",S1_pro.card_dice[i][j]);
	  	printf("\r\n");
   }

}

//协议处理
void S1_CTRL_PrO(void)
{

#if PCTL_PRO_ONLY//
   static u32 time=0;
   switch(S1_pro.PRO_step)
   {
        case PRO_CARD_NUM_TO_A83T://请求牌的数量
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
						  //清除数据
						  clearDirData();
						  time=S1_pro.timeoutCnt;
					  }
		          }
          
          break;
		}

        case PRO_CARD_ALL_CARD_TO_A83T://请求四个方位的牌
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
  	    case PRO_CARD_SEND_TO_A83T_DONG://发给东方位
  	   	{
  	   	  if(S1_pro.timeoutCnt%20==0)//1s
  	   	  	{
			  printf("PRO_CARD_SEND_TO_A83T_DONG\r\n");
	          //检查数据是否完整         
			  if(checkCard()==1)//数据有漏牌
			  {
					
			  }
			  else//数据没有漏牌
			  {
			    //发送全部数据		 
			    sendAllData();
				S1_pro.PRO_step=PRO_DICE_TO_A83T;	
				//
			  }

  	   	  	}
          break;
	    }
  	    case PRO_DICE_TO_A83T://请求点数(遥控控制知道庄家)
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
  	   case PRO_PLAY_DICE://接受到点数后打色 打色结束后要发送命令给A83T
  	   {
		 if(S1_pro.timeoutCnt%1000==0) 
		 {
		   printf("PRO_PLAY_DICE\r\n");
			 if(checkCard()==1)
			 {
			   displayDice();//显示打点
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
                //清除缓存
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
		 case PRO_CARD_NUM_TO_A83T://请求牌的数量
		 {//////
		
		   if(S1_pro.timeoutCnt%500==0)//500ms
		  {
			   printf("PRO_CARD_NUM_TO_A83T\r\n");

			    if(S1_pro.st_hardWare.washedFalg==0)//洗牌灯亮时，正在洗牌
			   	{
				   if(checkCard()==1)//
				   {
		
				   }
				   else//
				   {
					   //S1_pro.card_recv[i][CAER_REC_NUM-2]
					   S1_pro.PRO_step=PRO_CARD_ALL_CARD_TO_A83T;
					   S1_pro.st_hardWare.washedFalg=0;
					   	//清除数据
					   clearDirData();
					   time=S1_pro.timeoutCnt;
				   }
		   		}
		}
		
		   break;
		 }
	     
		 case PRO_CARD_ALL_CARD_TO_A83T://请求四个方位的牌
		 {
		      // printf("PRO_CARD_ALL_CARD_TO_A83T\r\n");
			  // time=S1_pro.timeoutCnt;
		       if(S1_pro.st_hardWare.washedFalg==1)//判断是否洗牌结束
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
						   S1_pro.st_hardWare.washedFalg=0;//清除标志
					   }
				   }
		       }
			break;
		 }
		 case PRO_CARD_SEND_TO_A83T_DONG://发给东方位
		 {
		   if(S1_pro.timeoutCnt%30==0)//1s
			 {
			   printf("PRO_CARD_SEND_TO_A83T_DONG\r\n");
			   //检查数据是否完整		  
			   if(checkCard()==1)//数据有漏牌
			   {
					 
			   }
			   else//数据没有漏牌
			   {
			     //清除串口的数据
			     init_rgb();
				 
				 //发送全部数据		  
				 sendAllData();
				 //
				 S1_pro.PRO_step=PRO_DICE_TO_A83T;			 
			   }
	
			 }
		   break;
		 }
		 case PRO_DICE_TO_A83T://请求点数(遥控控制知道庄家 即那位打色)
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
		case PRO_PLAY_DICE://接受到点数后打色 打色结束后要发送命令给A83T
		{
		  
		  if(S1_pro.timeoutCnt%1000==0) 
		  {
			  printf("PRO_PLAY_DICE\r\n");
			  if(checkCard()==1)
			  {
				displayDice();//显示打点
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
				 //清除缓存
				S1_clearData();
				 if(S1_pro.st_hardWare.washedFalg==0)//洗牌等亮，下一把数据
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


















