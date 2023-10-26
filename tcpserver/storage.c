#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

#include "storage.h"

/**
 * 全局传感器模拟数据，用于TCP和MQTT数据交互
 */
struct SensorData SENSOR_DATA = {10, 20};
extern int GLOBAL_CONN_FD;
extern int TCP_SEND_MUTEX;

/**
 * 发送传感器数据
 */
void sendSensorData(int connfd)
{
    ssize_t retval = 0;
    char *dataFrame = (char *)malloc(50); // 假设数据不超过100个字符
    sprintf(dataFrame, "temp:%.2f, humi:%.2f", SENSOR_DATA.temp, SENSOR_DATA.humi);
    retval = send(connfd, dataFrame, strlen(dataFrame), 0);
    if (retval < 0)
    {
        printf("send response failed, %ld!\r\n", retval);
    }
    else
    {
        printf("send SensorData{%s} to client done!\r\n", dataFrame);
    }
    free(dataFrame);
}

/**
 * 每隔两秒生成模拟传感器数据
 */
void generateSensorData_thread(void *arg)
{
    while (1)
    {
        srand(time(NULL)); // 使用当前时间作为随机数种子

        double _humi = (rand() % 41) + 10; // 生成10-50范围内的humi数据
        double _temp = (rand() % 41) + 10; // 生成0-40范围内的temp数据

        SENSOR_DATA.humi = _humi;
        SENSOR_DATA.temp = _temp;

        // if (GLOBAL_CONN_FD >= 0 && TCP_SEND_MUTEX != 1)
        // {
        //     sendSensorData(GLOBAL_CONN_FD);
        //     printf("storage=========%d\r\n", GLOBAL_CONN_FD);
        // }

        osDelay(300);
    }
}
