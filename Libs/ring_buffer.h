#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_
typedef struct _RING_BUF
{
	char *src;
	int max_size;
	int write;
	int read;
}RING_BUF;


struct contex_lock_t
{
    void(*lock)(void);
    void(*unlock)(void);
};

extern int init_ring_buf(RING_BUF *ring_buf,char *src,int size);
extern int push_buf(RING_BUF *ring_buf,char *src,int len);
extern int pop_buf(RING_BUF *ring_buf,char *dst,int len);
extern int push_buf_t(RING_BUF *ring_buf,char *src,int len);
extern int pop_buf_t(RING_BUF *ring_buf,char *dst,int len);
extern int get_buf_left_size(RING_BUF *ring_buf);
extern int is_buf_empty(RING_BUF *ring_buf);
extern int get_buf_data_size(RING_BUF *ring_buf);
extern int reset_rng_buf(RING_BUF *ring_buf);
extern void set_contex_cb(struct contex_lock_t *p_contex);
extern int copy_rng_buf(RING_BUF *ring_buf,char *dst,int len);
extern void set_rng_read_v(RING_BUF *ring_buf,int offset);
extern void set_rng_write_v(RING_BUF *ring_buf,int offset);
#endif
