#ifndef _DELAY_WORK_
#define _DELAY_WORK_
#ifndef STR_LIST_HEAD
#define STR_LIST_HEAD
struct list_head 
{
	struct list_head *next;
	struct list_head *prev;
};
#endif
struct delay_work
{
	unsigned int time;
	//unsigned int delay;
	void (*handle)(struct delay_work *timer);
	void *pri_data;
	struct list_head m_list;
};
extern void dlw_clk();
extern void init_delay_work();
extern void shedule_init(struct delay_work *work);
extern int  shedule_delay(struct delay_work *work,unsigned int delay_time);
extern void cancel_delay_work(struct delay_work *work);
extern void delay_work_entry();
#endif