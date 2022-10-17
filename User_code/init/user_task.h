#ifndef __USER_TASK_H_
#define __USER_TASK_H_
#include "main.h"

extern void begin(void);
extern void loop(void);
extern void user_task_init(void);
extern char* Int2String(int num, char* str);
extern int String2Int(char* str);

#endif // !__USER_TASK_H_




