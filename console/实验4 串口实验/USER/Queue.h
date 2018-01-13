#ifndef _QUEUE_H
#define _QUEUE_H

typedef enum {
QUEUE_BOOL_FALSE=0,
QUEUE_BOOL_TRUE=!QUEUE_BOOL_FALSE,
}QUEUE_BOOL_ENUM;

typedef struct
{
	unsigned char *Data;
	unsigned short Rear;
	unsigned short Front;
	unsigned short Qsize;
	QUEUE_BOOL_ENUM FullFlag;
}QueueStruct;


extern void Queue_Init(QueueStruct *Q,unsigned char *data,unsigned short Len);
extern void Queue_Clr(QueueStruct *Q);
extern unsigned short Queue_In(QueueStruct *Q,unsigned char *data,unsigned short Len);
extern unsigned short Queue_Out(QueueStruct *Q,unsigned char *data,unsigned short Len);
extern unsigned short Queue_GetLen(QueueStruct *Q);
extern unsigned short Queue_GetRemainLen(QueueStruct *Q);

#endif

