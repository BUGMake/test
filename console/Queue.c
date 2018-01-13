#include "Queue.h"


/***************************************************************************
º¯Êý:³õÊ¼»¯

***************************************************************************/
void Queue_Init(QueueStruct *Q,unsigned char *data,unsigned short Len)
{
	Q->Data = data;
	Q->Front = 0;
	Q->Rear =0;
	Q->Qsize = Len;
	Q->FullFlag =QUEUE_BOOL_FALSE;
}

void Queue_Clr(QueueStruct *Q)
{
	Q->Front = 0;
	Q->Rear =0;
	Q->FullFlag =QUEUE_BOOL_FALSE;
}


/***************************************************************************

***************************************************************************/
unsigned short Queue_In(QueueStruct *Q,unsigned char *data,unsigned short Len)
{
	unsigned short i = 0;
	unsigned short Front = Q->Front;
	unsigned short Rear =  Q->Rear;
	
	while(1)
	{
		if(Q->FullFlag )break;
		Q->Data[Front++] = data[i];
		if(Front >= Q->Qsize)
			Front = 0;
		i++;
		if(Front == Rear)
		{
			Q->FullFlag = QUEUE_BOOL_TRUE;
		}
		if(i>=Len)
			break;
	}
	Q->Front = Front;
	return i;
}

/***************************************************************************

***************************************************************************/
unsigned short Queue_Out(QueueStruct *Q,unsigned char *data,unsigned short Len)
{
	unsigned short i = 0;
	unsigned short Front = Q->Front;
	unsigned short Rear = Q->Rear;
	while((Rear != Front)||(Q->FullFlag ==QUEUE_BOOL_TRUE))
	{
		Q->FullFlag =QUEUE_BOOL_FALSE;
		data[i] = Q->Data[Rear++];
		if(Rear>= Q->Qsize)
			Rear = 0;
		i++;
		if(i == Len)break;
		
	}
	Q->Rear = Rear;
	return i;
}


unsigned short Queue_GetLen(QueueStruct *Q)
{
	unsigned short Len;
	if(Q->FullFlag)
		return Q->Qsize;
	if(Q->Front >= Q->Rear)
		Len=Q->Front - Q->Rear;
	else
		Len=Q->Qsize - Q->Rear + Q->Front;
	return Len;
}


unsigned short Queue_GetRemainLen(QueueStruct *Q)
{
	return (Q->Qsize -Queue_GetLen(Q));
}

