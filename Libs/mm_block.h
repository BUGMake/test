#ifndef _MEMERY_MANAGEMENT_H_
#define _MEMERY_MANAGEMENT_H_
extern void sz_free(void *p);
extern void *sz_malloc(unsigned int size);
extern void *sz_calloc(unsigned int number , unsigned int size);
extern void *sz_realloc(void *p, unsigned int size);
extern void *sz_reallocf(void *p, unsigned int size);
#endif
