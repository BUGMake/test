#include "ring_buffer.h"

static unsigned int get_higest_bit(int value)
{
	register unsigned int res ;
	register unsigned int shift;
	res = (~0u)-((~0u)>>1);//res = 0b1000...
	for(shift = 0; res!=0;shift++)
	{
		if((value & res)!=0)
		{
			break;
		}
		res = res >>1;
	}
	return res - 1;
}
//size :min 32Byte max 8MByte
int init_ring_buf(RING_BUF *ring_buf,char *src,int size)
{
	unsigned int res;
	res =  get_higest_bit(size);
	if(size < 32 || size > 0x800000)
		return -1;
	ring_buf->read = ring_buf->write = 0;
	ring_buf->src = (unsigned char *)src;
	ring_buf->max_size = (int)res;
	return 0;
}

int push_buf(RING_BUF *ring_buf,char *src,int len)
{
	char *end;
	char *pos = src;
	if(src == 0 || len == 0)
		return -1;
	end = src + len ;
	while(pos<end)
	{
		if(((ring_buf->write+1)&ring_buf->max_size) == ring_buf->read)
		{
			break;
		}
		else
		{
			ring_buf->src[ring_buf->write] = *pos;
			ring_buf->write += 1;
			ring_buf->write  = ring_buf->write&ring_buf->max_size;
			pos++;
		}

	}
	return pos - src;
}

int pop_buf(RING_BUF *ring_buf,char *dst,int len)
{
	char *end;
	char *pos;;
	if(dst == 0 || len ==0)
		return -1;
	end = dst + len ;
	pos = dst;
	while(pos<end)
	{
		if(ring_buf->write != ring_buf->read)
		{
			*pos = ring_buf->src[ring_buf->read];
			ring_buf->read += 1;
			ring_buf->read  = ring_buf->read&ring_buf->max_size;
			pos++;
		}
		else
		{
			break;
		}
	}
	return pos - dst;
}




int get_buf_left_size(RING_BUF *ring_buf)
{
	int ret = ring_buf->write-ring_buf->read;
	if(ret<0)
		return -ret;
	else
		return ring_buf->max_size-ret;
}

int is_buf_empty(RING_BUF *ring_buf)
{
	return (get_buf_left_size(ring_buf) == (ring_buf->max_size+1));
}

int get_buf_data_size(RING_BUF *ring_buf)
{
	int ret = ring_buf->write-ring_buf->read;
	if(ret > 0)
		return ret;
	else
		return ring_buf->max_size+ret;
}
