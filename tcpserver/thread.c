#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

// 创建线程，返回线程ID。封装成一个函数，便于调用
osThreadId_t newThread(char *name, osThreadFunc_t func, void *arg, osPriority_t Prio, uint32_t memsize)
{
    // 定义线程属性
    osThreadAttr_t attr = {
        name, 0, NULL, 0, NULL, memsize, Prio, 0, 0};
    // 创建线程，拿到线程ID
    osThreadId_t tid = osThreadNew(func, arg, &attr);
    // 得到当前线程的名字
    const char *c_name = osThreadGetName(osThreadGetId());
    // 得到线程参数
    int argv = *(int *)arg;
    // 打印参数
    if (tid == NULL)
    {
        printf("[%s] osThreadNew(%s) failed.\r\n", c_name, name);
    }
    else
    {
        printf("[%s] osThreadNew(%s) success, thread id: %d.\r\n", c_name, name, tid);
    }
    return tid;
}
