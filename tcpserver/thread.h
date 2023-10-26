#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg, osPriority_t Prio,uint32_t memsize);