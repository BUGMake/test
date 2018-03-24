#ifndef _SZ_LIBC_H_
#define _SZ_LIBC_H_
#define printk_green(args,...) sz_printk("\033[0;32m"args"\033[0;39m",##__VA_ARGS__)
#define printk_red(args,...) sz_printk("\033[0;31m"args"\033[0;39m",##__VA_ARGS__)
#define printf_red(args,...) sz_printf("\033[0;31m"args"\033[0;39m",##__VA_ARGS__)
#define printf_green(args,...) sz_printf("\033[0;32m"args"\033[0;39m",##__VA_ARGS__)
#define printf_blue(args,...) sz_printf("\033[0;34m"args"\033[0;39m",##__VA_ARGS__)
#define printf_purple(args,...) sz_printf("\033[0;35m"args"\033[0;39m",##__VA_ARGS__)
extern void sz_memset(char *buf,int value,int len);
extern void *sz_memcpy(char *dst,char *src,int len);
extern int sz_memcmp(const void * cs,const void * ct,int count);
extern int  sz_strlen(const char *s);
extern int  sz_strcmp(char *cs,char *ct);
extern int  sz_itoa(int data,char *dst,int len);
extern unsigned long sz_strtoul(const char *cp,char **endp,unsigned int base);
extern char * sz_strcat(char * dest, const char * src);
extern char * sz_strsep(char **s, const char *ct);
extern char * strstr(const char * s1,const char * s2);

extern void sz_set_libc_callback(int (*print)(char *buf,int len),void (*get_format_buf)(char **p_buf,int *len));
extern int sz_printf(const char *fmt, ...);
extern int sz_printk(const char *fmt, ...);
extern int sz_snprintf(char *buf, int size, const char *fmt, ...);
extern int sz_kmp_find_string(const char *src,const char *dst);
extern int sz_htonl(int v_data);
extern short sz_htons(short v_data);
extern int sz_ctoi(char *buf,int len);
extern void sz_itoc(char *buf,int value);

extern void sz_set_system_tick(int (*cb_)(void));
#endif
