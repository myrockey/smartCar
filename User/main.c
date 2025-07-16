#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <string.h>
#include "OLED.h"
#include "Delay.h"
#include "RingBuff.h"
#include "Timer.h"
#include "SmartCar.h"
#include "Bluetooth.h"
#include "Ultrasonic.h"
#include "WIFI.h"
#include "Tracking.h"
#include "Buzzer.h"
#include "DHT11.h"
#include "LED.h"
#include "Servo.h"
#include "IR_Nec.h"
#include "VoiceIdentify.h"

cJSON* cjson_test = NULL;//json
cJSON* cjson_params = NULL;

uint8_t RxData;//串口接收数据的变量
uint8_t RxDataClearFlag;//串口接收的数据已读后是否清除标志位 0-不清除，1-清除
uint8_t wifiState;//记录循迹位置值的变量
int trackingVal;//记录循迹位置值的变量
int distance;//离障碍物距离
char temp;//温度
char humi;//湿度
char str[16]; // 定义一个长度为16的字符数组作为字符串

//初始化
void BSP_Init(void);

//根据参数，执行对应功能
void Exec_Function(uint8_t type);
//根据参数，播报语音
void Voice_broadcast(uint8_t type);
void WIFI_Run(uint8_t* wifiState);
void WIFI_Receive_Task(uint8_t* RxData);
void WIFI_Send_DHT(char *temp, char *humi);


/************************** task **************************/
// WIFI
uint8_t WIFI_Task(void);
// 红外遥控
uint8_t IR_Task(void);
// 蓝牙
uint8_t Bluetooth_Task(void);
// 语音识别
uint8_t VoiceIdentify_Task(void);
// 舵机
void Servo_Task(uint8_t angle);
//小车电机驱动
void SmartCar_Task(uint8_t state);
//LED
void LED_Task(uint8_t state);
// 自动循迹
void Tracking_Task(void);
//读取温度并上传到IOT服务器
void DHT11_Task(void);
//超声波测距
void Ultrasonic_Distance_Task(void);
// 超声波避障
void Ultrasonic_Task(void);
// 接收数据并执行操作
uint8_t Receive_Task(void);
/************************** task **************************/

int main(void)
{
	BSP_Init();

	OLED_ShowString(2,1,"D:");
	OLED_ShowString(3,1,"J:");
	OLED_ShowString(4,1,"T:");
	
	// printf("IR NEC Decoder Ready\r\n");
//    while (1)
//    {
//        if (IR_GetDataFlag())          // 收到完整数据帧
//        {
//            uint8_t addr = IR_GetAddress();
//            uint8_t cmd  = IR_GetCommand();
//            printf("Addr: 0x%02X, Cmd: 0x%02X\r\n", addr, cmd);
//        }

//        if (IR_GetRepeatFlag())        // 连发帧（长按）
//        {
//            printf("Repeat\r\n");
//        }
//    }
	
	while(1)
	{
		//continue;
		if(WIFI_Task() != 0)
		{
			continue;
		}

		if(IR_Task() != 0)
		{
			continue;
		}
		
		if(Bluetooth_Task() != 0)
		{
			continue;
		}
		
		if(VoiceIdentify_Task() != 0)
		{
			continue;
		}
		
		if(Receive_Task() != 0)
		{
			continue;
		}
	}
}

void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//配置NVIC中断为分组4
	TIM1_Init();
	OLED_Init();//显示屏初始化
	SmartCar_Init();//电机驱动初始化
	Bluetooth_Init();//蓝牙初始化
	Ultrasonic_Init();//超声波初始化
	Tracking_Init();//循迹初始化
	Buzzer_Init();//蜂鸣器初始化
	DHT11_Init();
	LED_Init();//LED初始化
	WIFI_Init();
	Servo_Init();
	IR_Nec_Init();
	VoiceIdentify_Init();//语音识别初始化
}

/*
*根据参数，执行对应功能。
具体原理：蓝牙模块或语音识别模块（发送方） 通过串口传输数据 stm32（接收方），根据接收的数据，执行对应功能。
*/
void Exec_Function(uint8_t type)
{
	//默认清除
	if(RxData > 0){
		RxDataClearFlag = 1;
	}
	switch(RxData)
	{
		case 1:
			SmartCar_Task(RxData);
			break;
		case 2:
			SmartCar_Task(RxData);
			break;
		case 3:
			SmartCar_Task(RxData);
			break;
		case 4:
			SmartCar_Task(RxData);
			break;
		case 5:
			SmartCar_Task(RxData);
			break;
		case 6://顺时针旋转
			SmartCar_Task(RxData);
			break;
		case 7://逆时针旋转
			SmartCar_Task(RxData);
			break;
		case 8://超声波测距
			Ultrasonic_Distance_Task();
		 	break;
		case 9://循迹
			Tracking_Task();
			break;
		case 10://LED ON
			LED_Task(1);
			break;
		case 11://LED OFF
			LED_Task(0);
			break;
		case 12://读取温湿度
			DHT11_Task();
			break;
		case 13://Servo 0
			Servo_Task(0);
			break;
		case 14://Servo 45
			Servo_Task(45);
			break;
		case 15://Servo 90
			Servo_Task(90);
			break;
		case 16://Servo 135
			Servo_Task(135);
			break;
		case 17://Servo 180
			Servo_Task(180);
			break;
		case 18://超声波避障
			Ultrasonic_Task();
			break;	
	}
}

/*
*根据参数，播报语音。
具体原理：stm32（发送方）通过串口传输数据 语音识别模块ASRPRO（接收方），根据接收的数据，执行语音播报。
*/
void Voice_broadcast(uint8_t type)
{
	switch(type)
	{
		case 10://LED ON
			VoiceIdentify_SendByte(type);
			break;
		case 11://LED OFF
			VoiceIdentify_SendByte(type);
			break;
	}
}

/*
@函数名：WIFI_Run
@功能说明：WIFI运行并心跳检测，断开自动重连
@参数：
@返回值：无
*/
void WIFI_Run(uint8_t* wifiState)
{
	//服务器或者wifi已断开，清除事件标志，继续执行本任务，重新连接
	if(WIFI_CONNECT != 1)
	{
		printf("wifi connecting...\r\n");                 
		TIM_Cmd(WIFI_TIM, DISABLE);                       //关闭TIM3
		PING_MODE = 0;//关闭发送PING包的定时器3，清除事件标志位
		ESP8266_Buf_Clear();//清空接收缓存区
		*wifiState = ESP8266_WiFi_MQTT_Connect_IoTServer();
		if(*wifiState == 0)			  //如果WiFi连接云服务器函数返回0，表示正确，进入if
		{   			     
			printf("wifi connect success and mqtt sub success\r\n");
			OLED_ShowString(1,4,"wifi OK ");      
			ESP8266_Buf_Clear();//清空接收缓存区

			WIFI_CONNECT = 1;  //服务器已连接，抛出事件标志 

			//启动定时器30s模式
			TIM_WIFI_ENABLE_30S();
			pingFlag = 0;
			PING_MODE = 1; //30s的PING定时器，设置事件标志位
		}
	}
}

/*
@函数名：WIFI发送温湿度给服务器
@功能说明：Send_Task任务主体
@参数：
@返回值：无
*/
void WIFI_Send_DHT(char *temp, char *humi)
{
	//服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，否则挂起任务
	if(PING_MODE != 1)
	{
		return;
	}
	//读取DHT11温度模块
	char message[CMD_BUFFER_SIZE] = {0};
	snprintf(message,sizeof(message),"{\\\"temperature\\\": %d}",*temp);
	printf("wifi send dht:%s\n",message);
	ESP8266_MQTT_Publish(message);//添加数据，发布给服务器
}

void WIFI_Receive_Task(uint8_t* RxData)
{
	//服务器连接事件发生执行此任务，否则挂起
	if(WIFI_CONNECT != 1)
	{
		return;
	}

	//等待接收数据通知
	if(WIFI_Receive_Flag != 1)
	{
		return;
	}
	WIFI_Receive_Flag = 0;

	int len;
	//printf("KEY_Task Running\r\n");
	len = RingBuff_GetLen(&encoeanBuff);
	if (len) {
		uint8_t received_str[len+1];
		RingBuff_ReadNByte(&encoeanBuff,received_str,len);
		received_str[len] = '\0';
		// 输出接收到的字符串
		printf("Received: %s\n", received_str);

		// ping状态，mqtt连接成功
		//+MQTTCONN:0,6,1,"gz-3-mqtt.iot-api.com","1883","",1\r\n\r\nOK
		if (strstr((const char*)received_str, "+MQTTCONN:0,6") != NULL && strstr((const char*)received_str, "OK") != NULL) {
			printf("PING success\r\n");                       
			if(pingFlag == 1)
			{                   						     //如果pingFlag=1，表示第一次发送
				pingFlag = 0;    				       		 //要清除pingFlag标志
			}
			else if(pingFlag > 1)	
			{ 				 								 //如果pingFlag>1，表示是多次发送了，而且是2s间隔的快速发送
				pingFlag = 0;     				      		 //要清除pingFlag标志
				TIM_WIFI_ENABLE_30S(); 				      		 //PING定时器重回30s的时间
				PING_MODE = 1; //30s的PING定时器，设置事件标志位
			}
		}
		
		// 获取远程命令(TODO:待完善，接收的数据不完整。或调整设备传输数据类型为hex)
		if(strstr((const char*)received_str, MQTT_ATTR_PUSH_SUB) != NULL && strstr((const char*)received_str, "temp") != NULL){
			printf("IOT push data:%s \r\n",received_str); 		   	 //串口输出信息
			char json[256] = {0};//初始化缓冲区
			if(extract_json((const char*)received_str, json))
			{
				cjson_test = cJSON_Parse((const char*)json);
				if(cjson_test) {
					cjson_params = cJSON_GetObjectItem(cjson_test, "temp");
					if(cjson_params && cJSON_IsNumber(cjson_params)) {
						*RxData = (uint8_t)cjson_params->valueint;
					}
					cJSON_Delete(cjson_test); // 必须添加
				} else {
					printf("JSON parse fail: %s\n", json);
				}
			}
		}
	}
}

// WIFI
uint8_t WIFI_Task(void)
{
	if(WIFI_CONNECT == 0)
	{
		OLED_ShowString(1,4,"wifi CON.");
	}
	WIFI_Run(&wifiState);//WIFI运行
	OLED_ShowNum(1,1,wifiState,2);//显示wifi连接状态值
	
	WIFI_Receive_Task(&RxData);//WIFI接收数据，并ping连接状态
	//服务器连接以及ping心跳包30S发送模式事件发生时执行此任务，否则挂起任务
	if(PING_MODE == 0)
	{
		printf("WIFI connect error\r\n");
		OLED_ShowString(1,4,"wifi ERR");
		return 1;
	}

	Voice_broadcast(RxData);

	return 0;
}

// 红外遥控
uint8_t IR_Task(void)
{
	if (IR_GetDataFlag())          // 收到红外遥控的完整数据帧
	{
		RxData = IR_GetData();
	}

	return 0;
}

// 蓝牙
uint8_t Bluetooth_Task(void)
{
	if (Bluetooth_Serial_GetRxFlag())          // 收到数据标志
	{
		RxData = Bluetooth_Serial_GetRxData();

		Voice_broadcast(RxData);
	}

	return 0;
}

// 语音识别
uint8_t VoiceIdentify_Task(void)
{
	if (VoiceIdentify_Serial_GetRxFlag())          // 收到数据标志
	{
		RxData = VoiceIdentify_Serial_GetRxData();
	}

	return 0;
}

// 舵机
void Servo_Task(uint8_t angle)
{
	Servo_SetAngle(angle);
	sprintf(str,"servo %d ",angle);
}

//小车电机驱动
void SmartCar_Task(uint8_t state)
{
	switch(state)
	{
		case 1:
			Move_Forward();
			strcpy(str, "forword ");
			break;
		case 2:
			Move_Backward();
			strcpy(str, "backword");
			break;
		case 3:
			Car_Stop();
			strcpy(str, "  stop  ");
			break;
		case 4:
			Turn_Left();
			strcpy(str, "  left  ");
			break;
		case 5:
			Turn_Right();
			strcpy(str, " right  ");
			break;
		case 6://顺时针旋转
			Clockwise_Rotation();
			strcpy(str, " cycle  ");
			break;
		case 7://逆时针旋转
			CounterClockwise_Rotation();
			strcpy(str, " Ncycle ");
			break;
	}
}

//LED
void LED_Task(uint8_t state)
{
	if(state)
	{
		LED1_ON;
		strcpy(str, " led on ");
	}
	else
	{
		LED1_OFF;
		strcpy(str, " led off");
	}
}


// 自动循迹
void Tracking_Task(void)
{
	trackingVal = (L * 100)+ (M * 10) + (R * 1);
	OLED_ShowNum(3,4,trackingVal,3);//显示循迹模块的值

	RxDataClearFlag = 0;
	distance = Ultrasonic_Distance();
	OLED_ShowNum(2,4,distance,3);//显示超声波距离
	Tracking_Run();
	strcpy(str, "tracking");
}

//读取温度并上传到IOT服务器
void DHT11_Task(void)
{
	DHT11_Read_Data(&temp, &humi);
	sprintf(str,"%d oC H:%d",temp,humi);
	OLED_ShowString(4,4,str);
	WIFI_Send_DHT(&temp,&humi);
	strcpy(str, " dht11  ");
}

//超声波测距
void Ultrasonic_Distance_Task(void)
{
	RxDataClearFlag = 0;
	distance = Ultrasonic_Distance();
	OLED_ShowNum(2,4,distance,3);//显示超声波距离	
}

// 超声波避障
void Ultrasonic_Task(void)
{
	RxDataClearFlag = 0;
	distance = Ultrasonic_Distance();
	OLED_ShowNum(2,4,distance,3);//显示超声波距离	
	//距离太近时
	if(distance < 10)
	{
		Buzzer_ON;
		LED1_ON;
	}
	else
	{
		Buzzer_OFF;
		LED1_OFF;
	}

	Ultrasonic_Run();			
	strcpy(str, " sonic  ");
}

// 接收数据并执行操作
uint8_t Receive_Task(void)
{
	if(RxData == 0)
	{
		return 1;
	}

	if(RxData){
		OLED_ShowNum(1,14,RxData,2);//显示接收的参数
	}

	Exec_Function(RxData);
	OLED_ShowString(1,4,str);//显示执行的动作
	
	//清除上次接收的数据
	if(RxDataClearFlag == 1){
		RxData = 0;
	}

	return 0;
}
