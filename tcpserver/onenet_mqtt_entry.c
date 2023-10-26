#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include "onenet.h"
#include "MQTTClient.h"
#include "wifi_connecter.h"
#include "net_demo.h"
#include "net_common.h"
#include "net_params.h"

#include "storage.h"
#include "thread.h"

extern struct SensorData SENSOR_DATA;

int MQTT_COUNT = 0;
osThreadId_t MQTT_TID = NULL;

char *generateJSONData(double id)
{
    cJSON *root, *dp, *tempArray, *humiArray;
    char *jsonStr;

    // 创建 JSON 对象
    root = cJSON_CreateObject();

    // 添加 id 字段
    cJSON_AddNumberToObject(root, "id", id);

    // 创建 dp 对象
    dp = cJSON_CreateObject();

    // 创建 temperature 数组
    tempArray = cJSON_CreateArray();
    cJSON_AddItemToArray(tempArray, cJSON_CreateObject());
    cJSON_AddNumberToObject(cJSON_GetArrayItem(tempArray, 0), "v", SENSOR_DATA.temp);
    cJSON_AddItemToObject(dp, "temperature", tempArray);

    // 创建 humidity 数组
    humiArray = cJSON_CreateArray();
    cJSON_AddItemToArray(humiArray, cJSON_CreateObject());
    cJSON_AddNumberToObject(cJSON_GetArrayItem(humiArray, 0), "v", SENSOR_DATA.humi);
    cJSON_AddItemToObject(dp, "humi", humiArray);

    // 将 dp 对象添加到根对象中
    cJSON_AddItemToObject(root, "dp", dp);

    // 将 JSON 格式字符串存入字符数组
    jsonStr = cJSON_PrintUnformatted(root);

    // printf("====createdJSONis====%s\n", jsonStr);

    // 释放内存
    cJSON_Delete(root);
    return jsonStr;
}

/*
 * MQTTClient mq_client已是全局变量，无需传参
 */
void MQTT_SensorDataPublish_thread()
{
    while (1)
    {
        int rc = 0;
        char *jsonString = generateJSONData(123);
        rc = onenet_mqtt_publish("$sys/583419/mqtt-can1/dp/post/json", jsonString, strlen(jsonString));
        if (rc < 0)
        {
            printf("===========MQTTPublish error=%d\n", rc);
        }
        free(jsonString);
        // else
        // {
        //     printf("===========MQTTPublish success=%d\n", rc);
        // }
        // MQTT_COUNT++;
        // if (MQTT_COUNT == 8)
        // {
        //     osThreadTerminate(MQTT_TID);
        //     osDelay(10);
        //     MQTT_TID = newThread("MQTT_Report_Task", MQTT_SensorDataPublish_thread, NULL, osPriorityNormal, 10 * 1024);
        // }
        osDelay(300);
    }
}

void MQTT_Report_Task(void)
{
    // AP部分
    // WifiDeviceConfig config = {0};
    // // strcpy(config.preSharedKey, PARAM_HOTSPOT_PSK);
    // strcpy_s(config.ssid, WIFI_MAX_SSID_LEN, PARAM_HOTSPOT_SSID);
    // strcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, PARAM_HOTSPOT_PSK);
    // config.securityType = PARAM_HOTSPOT_TYPE;
    // int netId = ConnectToHotspot(&config);
    device_info_init(ONENET_INFO_DEVID, ONENET_INFO_PROID, ONENET_INFO_AUTH, ONENET_INFO_APIKEY, ONENET_MASTER_APIKEY);
    onenet_mqtt_init();
    MQTT_SensorDataPublish_thread();
    // return 0;
}
static void MQTT_Demo(void)
{
    osThreadAttr_t attr;

    attr.name = "MQTT_Report_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10 * 1024;
    attr.priority = osPriorityNormal;
    MQTT_TID = osThreadNew((osThreadFunc_t)MQTT_Report_Task, NULL, &attr);
    if (MQTT_TID == NULL)
    {
        printf("Falied to create MQTT_Report_Task!\n");
    }
    else
    {
        // printf("sensor_res=%d\n", newThread("sensor_data_thread", (osThreadFunc_t)generateSensorData_thread, NULL, osPriorityNormal2));
        // printf("===============%d\n", newThread("MQTT_PUB", MQTT_SensorDataPublish_thread, NULL, osPriorityNormal, 1024 * 5));
    }
}

APP_FEATURE_INIT(MQTT_Demo);