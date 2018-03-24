#ifndef _TERMINAL_H_
#define _TERMINAL_H_

extern void init_console(void);
extern int console_proc(char ch);
extern int register_console_write(int (*console_write_fun)(char *buf,int len));
extern int unregister_console_write(void);
extern int register_cmd(char *buf,int len);
extern int register_console_handl_data(int(*console_handl_fun)(int argc,char *argv[]));
extern void unregister_console_handl_data(void);
extern int set_login_info(const char *user,const char *psd);
extern int get_login_status(void);


//ÂÞÐ£¾´
extern char console_cmd_adc(void);

#endif
