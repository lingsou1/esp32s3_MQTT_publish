/*
接线说明:无

程序说明:开发板发布MQTT信息,在电脑端通过MQTT.fx软件来对开发板发布的主题进行订阅,
         在电脑端就可以实现查看开发板发布的信息

注意事项:尽量不要在loop()函数中使用延时函数,可能会对MQTT服务有影响,所以使用 #include <Ticker.h> 这个库来定时执行任务,以替代延时

函数示例:无

作者:灵首

时间:2023_4_12

*/
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <WiFiMulti.h>


WiFiMulti wifi_multi; // 建立WiFiMulti 的对象,对象名称是 wifi_multi
WiFiClient wifiClient;  //建立WiFiClient
PubSubClient mqttClient(wifiClient);  //根据WiFiClient来建立PubSubClient对象
Ticker ticker;    //建立定时对象

const char* mqttServer = "test.ranye-iot.net";  //这是需要连接的MQTT服务器的网址,可更改
int count;  //用来实现延时的计数

#define LED_A 10
#define LED_B 11



/**
* @brief 连接WiFi的函数
*
* @param 无
* @return 无
*/
void wifi_multi_con(void)
{
  int i = 0;
  while (wifi_multi.run() != WL_CONNECTED)
  {
    delay(1000);
    i++;
    Serial.print(i);
  }
}



/**
* @brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
*
* @param 无
* @return 无
*/
void wifi_multi_init(void)
{
  wifi_multi.addAP("haoze2938", "12345678");
  wifi_multi.addAP("LINGSOU1029", "12345678");
  wifi_multi.addAP("haoze1029", "12345678"); // 通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/**
* @brief 生成客户端名称并连接服务器同时串口输出信息
*
* @param 无
* @return 无
*/
void connectMQTTServer(){
  //生成客户端的名称(同一个服务器下不能存在两个相同的客户端名称)
  String clientId = "esp32s3---" + WiFi.macAddress();

  //尝试连接服务器,并通过串口输出有关信息
  if(mqttClient.connect(clientId.c_str())){
    Serial.println("MQTT Server Connect successfully!!!.\n");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.print("\n");
    Serial.println("ClientId:");
    Serial.println(clientId);
    Serial.print("\n");
  }
  else{
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    Serial.print("\n");
    delay(3000);
  }

}



/**
* @brief 发布MQTT信息,包含建立主题以及发布消息
*
* @param 无
* @return 无
*/
void pubMQTTmsg(){
  static int value; // 客户端发布信息用数字
 
  // 建立发布主题。主题名称以lingsou-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP32s3客户端名称各不相同，
  //同时建立主题后的两句程序是将 string 的数据转换为 char[] 字符数组类型
  //因为在之后的发布操作中只支持字符数组作为参数
  String topicString = "lingsou-" + WiFi.macAddress();  
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());  //将字符串数据 topicString 转换为字符数组类型的数据 publishTopic
 
  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。
  String messageString = "Hello World " + String(value++); 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  
  // 实现ESP32s3向主题发布信息
  if(mqttClient.publish(publishTopic, publishMsg)){
    Serial.print("Publish Topic:");
    Serial.print(publishTopic);
    Serial.print("\n");
    Serial.print("Publish message:");
    Serial.print(publishMsg);
    Serial.print("\n");    
  } else {
    Serial.print("Message Publish Failed.\n"); 
  }
}



/**
* @brief 计数,用来实现替代延时实现3秒发布一次MQTT信息
*
* @param 无
* @return 无
*/
void tickerCount(){
  count++;
}



void setup() {
  // 连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //led灯设置
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_A,0);
  digitalWrite(LED_B,0);

  // wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  // 输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //设置连接的MQTT服务器
  mqttClient.setServer(mqttServer,1883);

  //连接MQTT服务器
  connectMQTTServer();

  //每一秒执行一次tickerCount函数,来配合MQTT发布消息
  ticker.attach(1,tickerCount);


}

void loop() {
  //检查MQTT连接,若连接则且计数次数大于等于3则发布消息,同时保持心跳
  //若未服务器未连接则尝试重连
  if(mqttClient.connected()){
     if(count >= 3){
      pubMQTTmsg(); //发布消息
      count = 0;  //计数清零
     }
    mqttClient.loop();  //这是在保持客户端心跳
  }
  else{
    connectMQTTServer();  //重连服务器
  }
}