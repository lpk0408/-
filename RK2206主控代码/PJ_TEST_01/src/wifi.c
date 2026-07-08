#include "ohos_init.h"
#include "cmsis_os2.h"
#include "los_task.h"
#include "config_network.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h" 

#include "MQTTClient.h"

#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "gpio_con.h"
#include "pwm_sg.h"
#include "UART0_M0.h"

#define ROUTE_SSID      "lpk"
#define ROUTE_PASSWORD  "woshilpk123"

// #define CPOLAR_ADDR     "29.tcp.cpolar.top"
// #define DEVICE_ID       "txsmart_rk2206"
// #define CPOLAR_PORT     10745

#define CPOLAR_ADDR     "101.35.238.82"
#define DEVICE_ID       "txsmart_rk2206"
#define CPOLAR_PORT     1883

#define SUBSCRIBE_TOPIC     "test/rk2206/recv"
#define PUBLISH_TOPIC       "test/rk2206/send"

#define REV_TOPIC_FEED1     "rk2206/feed"
#define REV_TOPIC_WATE1     "rk2206/fwater"

#define REV_TOPIC_FANON     "rk2206/fan_con"
#define REV_TOPIC_HUMION    "rk2206/humi_con"
#define REV_TOPIC_DAT       "rk2206/dat_con"

#define LORA_A1_FEED        "afd"
#define LORA_A1_WATER       "awd"

int feed_id = 1;

int water_id = 1;

uint8_t lora_get_flag = 1;

extern unsigned int m_mutex_WIFI;

static Network network;
static MQTTClient client;
static unsigned int mqttConnectFlag = 0;

#define MAX_BUFFER_LENGTH 512
static unsigned char sendBuf[MAX_BUFFER_LENGTH];
static unsigned char readBuf[MAX_BUFFER_LENGTH];


/* 全局标志位：1 表示 WiFi 已连接且获取到有效 IP */
volatile int g_wifi_connected = 0;

volatile int g_mqtt_connected = 0;
/**
 * 检测 WiFi 是否已连接
 * 寻找第一个可用的 STA 网卡，判断其链路是否 UP 且 IP 非 0
 */
static int IsWifiConnected(void)
{
    struct netif *iface;
    /* 遍历所有网卡，找到合适的接口 */
    for (iface = netif_list; iface != NULL; iface = iface->next) {
        /* 跳过回环接口 */
        if (iface->name[0] == 'l' && iface->name[1] == 'o') continue;
        /* 检查链路是否 UP 且 IP 地址不是 0.0.0.0 */
        if (netif_is_link_up(iface) && !ip4_addr_isany_val(*netif_ip4_addr(iface))) {
            return 1;
        }
    }
    return 0;
}

void wifi_sta_mode(void *args)
{
    // 重置 MAC 地址（多人实验请修改）
    uint8_t mac_address[6] = {0x00, 0xdc, 0xb6, 0x90, 0x00, 0x00};

    FlashInit();
    VendorSet(VENDOR_ID_WIFI_MODE, "STA", 3);                     // 配置为 STA 模式
    VendorSet(VENDOR_ID_MAC, mac_address, 6);
    VendorSet(VENDOR_ID_WIFI_ROUTE_SSID, ROUTE_SSID, sizeof(ROUTE_SSID));
    VendorSet(VENDOR_ID_WIFI_ROUTE_PASSWD, ROUTE_PASSWORD, sizeof(ROUTE_PASSWORD));

    SetWifiModeOff();
    SetWifiModeOn();

    /* 等待 WiFi 连接成功，最多 30 秒 */
    printf("Waiting for WiFi connection...\n");
    for (int i = 0; i < 300; i++) {
        if (IsWifiConnected()) {
            g_wifi_connected = 1;   // 置位全局标志

            // 打印连接成功后的 IP 地址（遍历查找对应网卡）
            struct netif *iface;
            for (iface = netif_list; iface != NULL; iface = iface->next) 
            {
                if (iface->name[0] == 'l' && iface->name[1] == 'o') continue;
                if (netif_is_link_up(iface) && !ip4_addr_isany_val(*netif_ip4_addr(iface))) {
                    printf("WiFi connected! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(iface)));
                    break;
                }
            }
            break;
        }
        osDelay(100);  // 每 100ms 检查一次
    }

    if (!g_wifi_connected) 
    {
        printf("WiFi connection timeout!\n");
    }

    // 如果需要在任务中持续运行，可以在这里加 while(1) { ... }
    // 否则任务退出后 g_wifi_connected 仍然保留，其他任务可以读取
}

/* 消息回调 */
static void mqtt_message_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);
}

static void mqtt_feed1_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);

    char *payload_ptr = (char *)data->message->payload;
    int payload_len = data->message->payloadlen;

    // 2. 增加判断逻辑
    if (payload_len == 3) 
    {
        if (strncmp(payload_ptr, "one", 3) == 0) 
        {
            printf("Action: Feed One\n");
            LoRa_Send_string(1,"afd");
            lora_get_flag = 1;
            a_flag = 0;
        } 
        else if (strncmp(payload_ptr, "two", 3) == 0) 
        {
            printf("Action: Feed Two \n");

        }
        else 
        {
            printf("Unknown 3-byte payload.\n");
        }
    } 
    else 
    {
        // 如果长度不是 3，说明既不是 "one" 也不是 "two"
        printf("Payload length mismatch (not 'one' or 'two').\n");
    }
}

static void mqtt_water1_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);
    
    char *payload_ptr = (char *)data->message->payload;
    int payload_len = data->message->payloadlen;

    // 2. 增加判断逻辑
    if (payload_len == 3) 
    {
        if (strncmp(payload_ptr, "one", 3) == 0) 
        {
            printf("Action: water One\n");

            LoRa_Send_string(1,"awd");
            lora_get_flag = 1;
            // LoRa_Send_string(1,LORA_A1_WATER);
        } 
        else if (strncmp(payload_ptr, "two", 3) == 0) 
        {
            printf("Action: water Two \n");
  
            // LoRa_Send_string(2,LORA_A1_WATER); // 示例保留原逻辑
        }
        else 
        {
            printf("Unknown 3-byte payload.\n");
        }
    } 
    else 
    {
        // 如果长度不是 3，说明既不是 "one" 也不是 "two"
        printf("Payload length mismatch (not 'one' or 'two').\n");
    }
}

static void mqtt_fan_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);

        // 获取 payload 指针和长度
    char *payload_ptr = (char *)data->message->payload;
    int payload_len = data->message->payloadlen;

    // 2. 增加判断逻辑
    // 首先检查长度，再比对内容（防止 "true_and_more" 这种数据误判）
    if (payload_len == 4 && strncmp(payload_ptr, "true", 4) == 0) 
    {
        // 收到 "true" 的处理逻辑
        printf("Fan Action: TURN ON\n");
        GPIO_FAN_ENABLE();
        // 执行开灯/开风扇等操作...
    } 
    else if (payload_len == 5 && strncmp(payload_ptr, "false", 5) == 0) 
    {
        // 收到 "false" 的处理逻辑
        printf("Fan Action: TURN OFF\n");
        GPIO_FAN_DISABLE();
        // 执行关灯/关风扇等操作...
    } 
    else 
    {
        // 收到未知字符串的处理逻辑
        printf("Unknown payload received.\n");
    }
}

static void mqtt_humi_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);

    char *payload_ptr = (char *)data->message->payload;
    int payload_len = data->message->payloadlen;

    // 2. 增加判断逻辑
    // 首先检查长度，再比对内容（防止 "true_and_more" 这种数据误判）
    if (payload_len == 4 && strncmp(payload_ptr, "true", 4) == 0) 
    {
        // 收到 "true" 的处理逻辑
        printf("humi Action: TURN ON\n");
        // 执行开灯/开风扇等操作...
        GPIO_HUMI_ENABLE();
    } 
    else if (payload_len == 5 && strncmp(payload_ptr, "false", 5) == 0) 
    {
        // 收到 "false" 的处理逻辑
        printf("humi Action: TURN OFF\n");
        // 执行关灯/关风扇等操作...
        GPIO_HUMI_DISABLE();
    } 
    else 
    {
        // 收到未知字符串的处理逻辑
        printf("Unknown payload received.\n");
    }
}

static void mqtt_dat_arrived(MessageData *data)
{
    printf("Message arrived on topic %.*s: %.*s\n",
           data->topicName->lenstring.len, data->topicName->lenstring.data,
           data->message->payloadlen, (char *)data->message->payload);

    char *payload_ptr = (char *)data->message->payload;
    int payload_len = data->message->payloadlen;

    // 2. 增加判断逻辑
    // 首先检查长度，再比对内容（防止 "true_and_more" 这种数据误判）
    if (payload_len == 4 && strncmp(payload_ptr, "true", 4) == 0) 
    {
        // 收到 "true" 的处理逻辑
        printf("dat Action: TURN ON\n");
        dat_turnon();
        // 执行开灯/开风扇等操作...
        // GPIO_HUMI_ENABLE();
    } 
    else if (payload_len == 5 && strncmp(payload_ptr, "false", 5) == 0) 
    {
        // 收到 "false" 的处理逻辑
        printf("dat Action: TURN OFF\n");
        dat_turnoff();
        // 执行关灯/关风扇等操作...
        // GPIO_HUMI_DISABLE();
    } 
    else 
    {
        // 收到未知字符串的处理逻辑
        printf("Unknown payload received.\n");
    }
}


/**
 * 发送消息到指定主题
 * @param topic   目标主题（字符串）
 * @param payload 消息内容（字符串，可以包含 \0 意外的二进制，但这里以普通 C 字符串为例）
 * @return 0 成功，非 0 失败（MQTTPublish 的错误码）
 */
int mqtt_publish_message(const char *topic, const char *payload)
{
    if (mqttConnectFlag == 0)
    {
        printf("MQTT not connected, cannot publish.\n");
        return -1;   // 自定义错误码表示未连接
    }

    MQTTMessage message;
    message.qos = 0;           // QoS 0
    message.retained = 0;
    message.payload = (void*)payload;
    message.payloadlen = strlen(payload);

    int rc = MQTTPublish(&client, topic, &message);
    if (rc != 0) {
        printf("Publish to %s failed: %d\n", topic, rc);
        mqttConnectFlag = 0;   // 标记连接失效，主循环会重连
    } else {
        printf("Published to %s: %s\n", topic, payload);
    }
    return rc;
}



/* MQTT 连接 + 保持在线 */
void mqtt_task(void *args)
{   

    int rc;

    printf("Starting MQTT connection...\n");

    printf("Waiting for WiFi connection...\n");
    while (!g_wifi_connected) {
        osDelay(100);   // 每100ms检查一次
    }

    NetworkInit(&network);

connect:
    printf("Connecting to %s:%d...\n", CPOLAR_ADDR, CPOLAR_PORT);
    rc = NetworkConnect(&network, CPOLAR_ADDR, CPOLAR_PORT);
    if (rc != 0) {
        printf("TCP connect failed: %d\n", rc);
        osDelay(500);
        goto connect;
    }

    MQTTClientInit(&client, &network, 2000,
                   sendBuf, sizeof(sendBuf),
                   readBuf, sizeof(readBuf));

    MQTTString clientId = MQTTString_initializer;
    clientId.cstring = DEVICE_ID;

    MQTTString userName = MQTTString_initializer;
    MQTTString password = MQTTString_initializer;
    userName.cstring = NULL;   // 无认证
    password.cstring = NULL;

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID = clientId;
    data.username = userName;
    data.password = password;
    data.MQTTVersion = 4;
    data.keepAliveInterval = 60;
    data.cleansession = 1;
    data.willFlag = 0;

    printf("Sending MQTT CONNECT...\n");
    rc = MQTTConnect(&client, &data);
    if (rc != 0) 
    {
        printf("MQTT connect failed: %d\n", rc);
        NetworkDisconnect(&network);
        MQTTDisconnect(&client);
        osDelay(500);
        goto connect;
    }

    mqttConnectFlag = 1;

    g_mqtt_connected = 1;

    printf("MQTT connected!\n");

    /* 订阅主题 */
    printf("Subscribing to %s...\n", SUBSCRIBE_TOPIC);
    rc = MQTTSubscribe(&client, SUBSCRIBE_TOPIC, 0, mqtt_message_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
        // 根据需求决定是否重连或继续
    }

    printf("Subscribing to %s...\n", REV_TOPIC_FEED1);
    rc = MQTTSubscribe(&client, REV_TOPIC_FEED1, 0, mqtt_feed1_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
        // 根据需求决定是否重连或继续
    }

    printf("Subscribing to %s...\n", REV_TOPIC_WATE1);
    rc = MQTTSubscribe(&client, REV_TOPIC_WATE1, 0, mqtt_water1_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
        // 根据需求决定是否重连或继续
    }

    printf("Subscribing to %s...\n", REV_TOPIC_FANON);
    rc = MQTTSubscribe(&client, REV_TOPIC_FANON, 0, mqtt_fan_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
        // 根据需求决定是否重连或继续
    }

    printf("Subscribing to %s...\n", REV_TOPIC_HUMION);
    rc = MQTTSubscribe(&client, REV_TOPIC_HUMION, 0, mqtt_humi_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
    }

    printf("Subscribing to %s...\n", REV_TOPIC_DAT);
    rc = MQTTSubscribe(&client, REV_TOPIC_DAT, 0, mqtt_dat_arrived);
    if (rc != 0)
    {
        printf("Subscribe failed: %d\n", rc);
    }

    printf("Subscribing to %s...\n", REV_TOPIC_HUMION);


    /* 主循环：保持心跳，接收消息 */
    while (1) {
        if (mqttConnectFlag == 0) {
            printf("MQTT disconnected, reconnecting...\n");
            NetworkDisconnect(&network);
            MQTTDisconnect(&client);
            goto connect;
        }

        // 使用 1 秒超时，让循环更灵敏
        rc = MQTTYield(&client, 1000);
        if (rc != 0) {
            printf("MQTTYield error %d, reconnecting...\n", rc);
            mqttConnectFlag = 0;
            continue;               // 立刻跳回循环开头重连，不执行发布
        }
    }
}




