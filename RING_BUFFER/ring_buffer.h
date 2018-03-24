#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

typedef struct _RING_BUF
{
	unsigned char *src;
	int max_size;
	int write;
	int read;
}RING_BUF;

extern int init_ring_buf(RING_BUF *ring_buf,char *src,int size);
extern int push_buf(RING_BUF *ring_buf,char *src,int len);
extern int pop_buf(RING_BUF *ring_buf,char *dst,int len);
extern int get_buf_left_size(RING_BUF *ring_buf);
extern int is_buf_empty(RING_BUF *ring_buf);
extern int get_buf_data_size(RING_BUF *ring_buf);

#endif
