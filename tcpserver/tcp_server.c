#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "net_demo.h"
#include "net_common.h"
#include "net_params.h"
#include "wifi_connecter.h"
#include "storage.h"
#include "thread.h"

#include "iot_gpio.h"
#include "hi_io.h"

#define LED_GPIO 9

#include "ohos_init.h" // 用于初始化服务(services)和功能(features)
#include "cmsis_os2.h" // CMSIS-RTOS API V2

#define STACK_SIZE (10240)
#define DELAY_TICKS_10 (10)
#define DELAY_TICKS_100 (100)

#define DELAY_1S (1)

static char request[30] = "";
static int sockfd = -1;
static osThreadId_t TCP_thread_tid;

int GLOBAL_CONN_FD = -1;
int TCP_SEND_MUTEX = 0;

extern struct SensorData SENSOR_DATA;

/**
 * 间隔0.5秒闪灯
 */
void flashLED()
{
    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);

    // 等待0.5秒。单位是10ms。
    osDelay(50);

    // 输出高电平，熄灭LED。
    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1);
}

void disconnect_tcp(int connfd)
{
    sleep(DELAY_1S);
    close(connfd);
    sleep(DELAY_1S); // for debug
}

void cleanup_tcp(int connfd)
{
    printf("do_cleanup...\r\n");
    close(connfd);
}
/**
 * 处理接收数据，若命中command则进行相应逻辑处理
 * @note 接收到的数据都存到request里了，暂时不需要参数
 */
void revController(int connfd)
{
    printf("revController=========%d---%d\r\n", connfd, GLOBAL_CONN_FD);
    ssize_t retval = 0;
    if (strcmp(request, "switch1:on") == 0)
    {
        // 执行开关1打开操作
        printf("Switch 1 is turned on.\n");
        flashLED();
        TCP_SEND_MUTEX = 1;
        retval = send(connfd, request, strlen(request), 0);
        TCP_SEND_MUTEX = 0;
    }
    else if (strcmp(request, "switch1:off") == 0)
    {
        // 执行开关1关闭操作
        printf("Switch 1 is turned off.\n");
        flashLED();
        TCP_SEND_MUTEX = 1;
        retval = send(connfd, request, strlen(request), 0);
        TCP_SEND_MUTEX = 0;
    }
    else if (strcmp(request, "switch2:on") == 0)
    {
        // 执行开关2打开操作
        printf("Switch 2 is turned on.\n");
        flashLED();
        TCP_SEND_MUTEX = 1;
        retval = send(connfd, request, strlen(request), 0);
        TCP_SEND_MUTEX = 0;
    }
    else if (strcmp(request, "switch2:off") == 0)
    {
        // 执行开关2关闭操作
        printf("Switch 2 is turned off.\n");
        flashLED();
        TCP_SEND_MUTEX = 1;
        retval = send(connfd, request, strlen(request), 0);
        TCP_SEND_MUTEX = 0;
    }
    else
    {
        // 对于其他请求不进行处理
        printf("Unknown command: %s\n", request);
    }
    //
    if (retval < 0)
    {
        printf("send commendResponse failed, %ld!\r\n", retval);
    }
    else
    {
        printf("requestBuffer:{%s}!\r\n", request);
    }
    // 清空request，防止判断错误
    memset(request, 0, sizeof(request));
}

// tcp线程收发函数
void tcp_thread(void *arg)
{
    ssize_t retval = 0;
    // static int count = 0;
    // 得到当前线程ID和名字
    osThreadId_t tid = osThreadGetId();
    const char *c_name = osThreadGetName(tid);
    // 输出当前线程名字和ID
    printf("[%s] osThreadGetId, thread id:%p\r\n", c_name, tid);
    // 输出参数
    printf("[%s] %s\r\n", c_name, (char *)arg);
    // 转换参数得到connfd的值
    int connfd = *(int *)arg;
    printf("=======================%d\n\n", connfd);
    while (1)
    {
        // 接收tcp逻辑部分
        retval = recv(connfd, request, sizeof(request), 0);
        if (retval < 0) // 小于表示失败
        {
            printf("recv request failed, %ld!\r\n", retval);
            disconnect_tcp(connfd);
            if (TCP_thread_tid)
            {
                osThreadTerminate(TCP_thread_tid);
            }
        }
        else
        {
            printf("recv request{%s} from client done!\r\n", request);
            // 进行接收数据的逻辑控制
            revController(connfd);
        }
    }
}

void TcpServerTest()
{
    ssize_t retval = 0;
    int backlog = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    int connfd = -1;
    // server信息
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PARAM_SERVER_PORT); // 端口号，从主机字节序转为网络字节序
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 允许任意主机接入， 0.0.0.0

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); // 绑定端口
    if (retval < 0)
    {
        printf("bind failed, %ld!\r\n", retval);
        // goto do_cleanup;
        cleanup_tcp(connfd);
    }
    printf("===========================bind to port %hu success!\r\n", PARAM_SERVER_PORT);

    retval = listen(sockfd, backlog); // 开始监听
    if (retval < 0)
    {
        printf("listen failed!\r\n");
        // goto do_cleanup;
        cleanup_tcp(connfd);
    }
    printf("=========================listen with %d backlog success!\r\n", backlog);
}

// 监听tcp client连接的task
void Tcp_Accept_Task()
{
    while (sockfd >= 0)
    {
        // client信息初始化
        struct sockaddr_in clientAddr = {0};
        socklen_t clientAddrLen = sizeof(clientAddr);
        printf("============waitinf for client...\n");
        int connfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (connfd > 0)
        {
            // 创建tcp线程
            char thread_name[20] = {0};
            GLOBAL_CONN_FD = connfd;
            printf("accept success, connfd = %d!\r\n", connfd);
            printf("client addr info: host = %s, port = %hu\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            snprintf(thread_name, sizeof(thread_name), "tcp_thread_%d", connfd);
            TCP_thread_tid = newThread(thread_name, (osThreadFunc_t)tcp_thread, (void *)&connfd, osPriorityNormal, 1024 * 5);
        }
        osDelay(10);
    }
}

static void TcpTask(void)
{
    // AP部分
    WifiDeviceConfig config = {0};
    // strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    strcpy_s(config.ssid, WIFI_MAX_SSID_LEN, PARAM_HOTSPOT_SSID);
    strcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, PARAM_HOTSPOT_PSK);
    config.securityType = PARAM_HOTSPOT_TYPE;
    int netId = ConnectToHotspot(&config);
    osDelay(DELAY_TICKS_10);

    // LED部分
    IoTGpioInit(LED_GPIO);
    hi_io_set_func(LED_GPIO, HI_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);

    osThreadId_t server_tid = newThread("tcp_server_main", (osThreadFunc_t)TcpServerTest, NULL, osPriorityNormal, 1024 * 5);
    // 创建一个线程，并将其加入活跃线程组中
    if (server_tid == NULL)
    {
        printf("[MutexTestTask] Falied to create rtosv2_mutex_main!\n");
    }
    else
    {
        if (sockfd)
        {
            // 延迟3s等待tcp连接完毕

            osDelay(300);
            osThreadTerminate(server_tid);
            printf("============== create tcp & sensor threadt ===========\n");
            // 创建 TCP线程和传感器模拟数据线程
            printf("tcp_res=%d\n", newThread("tcp_accept_thread", (osThreadFunc_t)Tcp_Accept_Task, NULL, osPriorityNormal, 1024 * 10));
            osDelay(10);
            printf("sensor_res=%d\n", newThread("sensor_data_thread", (osThreadFunc_t)generateSensorData_thread, NULL, osPriorityNormal, 1024 * 5));
        }
    }
}

SYS_RUN(TcpTask);