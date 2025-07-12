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
// #include "Serial.h"
#include "Bluetooth.h"
#include "Ultrasonic.h"
#include "WIFI.h"
#include "Tracking.h"
// #include "Buzzer.h"
#include "DHT11.h"
#include "LED.h"
#include "Servo.h"
#include "VoiceIdentify.h"

cJSON* cjson_test = NULL;//json
cJSON* cjson_params = NULL;
cJSON* cjson_params_state = NULL;

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
void Exec_Function(uint8_t type, char str[]);

//根据参数，播报语音
void Voice_broadcast(uint8_t type);

void WIFI_Run(uint8_t* wifiState);
void WIFI_Receive_Task(uint8_t* RxData);
void WIFI_Send_DHT(char *temp, char *humi);

int main(void)
{
	BSP_Init();

	OLED_ShowString(2,1,"D:");
	OLED_ShowString(3,1,"J:");
	OLED_ShowString(4,1,"T:");
	while(1)
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
			continue;
		}
		//OLED_ShowString(1,4,"wifi OK ");

		trackingVal = (L * 100)+ (M * 10) + (R * 1);
		OLED_ShowNum(3,4,trackingVal,3);//显示循迹模块的值
		
		// 接收到数据
		if(Serial_GetRxFlag() == 1)
		{
			RxData = Serial_GetRxData();
			OLED_ShowNum(1,14,RxData,2);//显示接收的参数
		}

		//Voice_broadcast(RxData);
		Exec_Function(RxData, str);
		OLED_ShowString(1,4,str);//显示执行的动作
		
		//清除上次接收的数据
		if(RxDataClearFlag == 1){
			RxData = 0;
		}
	}
}

void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//配置NVIC中断为分组4
	OLED_Init();//显示屏初始化
	SmartCar_Init();//电机驱动初始化
	Bluetooth_Init();//蓝牙初始化
	Ultrasonic_Init();//超声波初始化
	Tracking_Init();//循迹初始化
	// Buzzer_Init();//蜂鸣器初始化
	DHT11_Init();
	LED_Init();//LED初始化
	WIFI_Init();
	Servo_Init();
	VoiceIdentify_Init();//语音识别初始化
}

/*
*根据参数，执行对应功能。
具体原理：蓝牙模块或语音识别模块（发送方） 通过串口传输数据 stm32（接收方），根据接收的数据，执行对应功能。
*/
void Exec_Function(uint8_t type, char str[])
{
	//默认清除
	if(RxData > 0){
		RxDataClearFlag = 1;
	}
	switch(RxData)
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
		case 8://超声波测距
			RxDataClearFlag = 0;
			distance = Ultrasonic_Distance();
			OLED_ShowNum(2,4,distance,3);//显示超声波距离
		 	strcpy(str, "distance");
		 	break;
		case 9://循迹
			RxDataClearFlag = 0;
			distance = Ultrasonic_Distance();
			OLED_ShowNum(2,4,distance,3);//显示超声波距离
			Tracking_Run();
			strcpy(str, "tracking");
			break;
		case 10://LED ON
			LED1_ON;
			//LED2_ON;LED3_ON;
			strcpy(str, " led on ");
			break;
		case 11://LED OFF
			LED1_OFF;
			//LED2_OFF;LED3_OFF;
			strcpy(str, " led off");
			break;
		case 12://读取温湿度
			DHT11_Read_Data(&temp, &humi);
			sprintf(str,"%d oC H:%d",temp,humi);
			OLED_ShowString(4,4,str);
			WIFI_Send_DHT(&temp,&humi);
			strcpy(str, " dht11  ");
			break;
		case 13://Servo 0
			Servo_SetAngle(0);
			strcpy(str, "servo 0 ");
			break;
		case 14://Servo 45
			Servo_SetAngle(45);
			strcpy(str, "servo 45");
			break;
		case 15://Servo 90
			Servo_SetAngle(90);
			strcpy(str, "servo 90");
			break;
		case 16://Servo 135
			Servo_SetAngle(135);
			strcpy(str, "servo135");
			break;
		case 17://Servo 180
			Servo_SetAngle(180);
			strcpy(str, "servo180");
			break;
		case 18://超声波避障
			RxDataClearFlag = 0;
			distance = Ultrasonic_Distance();
			OLED_ShowNum(2,4,distance,3);//显示超声波距离	
			Ultrasonic_Run();			
			strcpy(str, " sonic  ");
			//距离太近时
			if(distance < 10)
			{
				// Buzzer_ON;
				LED1_ON;
			}
			else
			{
				// Buzzer_OFF;
				LED1_OFF;
			}
			break;
		// case 12://Buzzer ON
		// 	Buzzer_ON;
		// 	strcpy(str, " buzzer ");
		// 	break;
		// case 13://Buzzer OFF
		// 	Buzzer_OFF;
		// 	strcpy(str, " buzzer ");
		// 	break;
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
		wifiState = ESP8266_WiFi_MQTT_Connect_IoTServer();
		if(wifiState == 0)			  //如果WiFi连接云服务器函数返回0，表示正确，进入if
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
	snprintf(message,sizeof(message),"{\\\"temperature\\\": %d,\\\"humidity\\\": %d}",*temp,*humi);	
	printf("wifi send dht:%s\n",message);
	//ESP8266_MQTT_Publish(message);//添加数据，发布给服务器
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
			printf("PING报文回复\r\n");                       
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
		
		// 获取远程命令
		if(strstr((const char*)received_str, "getValue") != NULL && strstr((const char*)received_str, "state") != NULL){
			printf("服务器下发的数据:%s \r\n",received_str); 		   	 //串口输出信息
			char json[128];
			extract_json((const char*)received_str, json);
			/* 解析整段JSO数据 */
			cjson_test = cJSON_Parse((const char*)json);
			if(cjson_test == NULL)
			{
				printf("parse fail.\n");
				return;
			}
			/* 依次根据名称提取JSON数据（键值对） */
			cjson_params = cJSON_GetObjectItem(cjson_test, "params");
			cjson_params_state = cJSON_GetObjectItem(cjson_params, "state");
			*RxData = (uint8_t)cjson_params_state->valueint;
		}
	}
}
