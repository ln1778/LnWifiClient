/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/

#include <DNSServer.h>
#include <WiFiClient.h>
#include "LnWifiClient.h"
#include <WebServer.h>


LnWifiClient mlnWifiClient;
IPAddress apIP(192, 168,4, 1);
 DNSServer dnsServer;
WebServer webServer(80);
 const byte DNS_PORT = 53;
const char* AP_NAME = "LNESP";
String macAddress=String(WiFi.macAddress()).c_str();

static WiFiClient espClient;

//暂时存储wifi账号密码
char sta_ssid[32] = {0};
char sta_password[64] = {0};
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
const char* restbtn = " <form name='input' action='/esprest' method='POST'><input type='submit' value='重启'/></form>/";
//配网页面代码
const char* page_html = "\
<!DOCTYPE html>\r\n\
<html lang='en'>\r\n\
<head>\r\n\
  <meta charset='UTF-8'>\r\n\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\r\n\
  <title>Document</title>\r\n\
</head>\r\n\
<body>\r\n\
  <form name='input' action='/' method='POST'>\r\n\
        wifi名称: <br>\r\n\
        <input type='text' name='ssid'><br>\r\n\
        wifi密码:<br>\r\n\
        <input type='text' name='password'><br>\r\n\
        <input type='submit' value='保存'>\r\n\
    </form>\r\n\
</body>\r\n\
</html>\r\n\
";

 String successpage="<!DOCTYPE html>\
<html lang='en'><head>\
    <meta charset='UTF-8'>\
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
    <title>LNESP 物联</title>\
    <style>\
   body{\
    width: 100vw;\
    height: 100vh;\
    display: flex;\
    align-items: center;\
    justify-content: center;\
    flex-direction: column;\
    background: linear-gradient(45deg, #30a33a, #2389ea);\
    margin: 0;\
    padding: 0;\
   }\
   .formbox{\
       margin-top: 30px;\
       display: flex;\
       align-items: center;\
       justify-content: center;\
       flex-direction: column;\
   }\
    .submitbtn{\
      margin-top:15px;\
      border:none;\
      outline: none;\
      background:hsl(207, 100%, 63%);\
      color:#fff;\
      border-radius:20px;\
      width:240px;\
      height:40px;\
    }\
    .inputitemvalue{\
        border-radius: 20px;\
        border:none;\
        outline: none;\
        padding:10px;\
    }\
    .inputitem{\
        padding-bottom:15px;\
        display: flex;\
        align-items: center;\
        color:#fff;\
    }\
    .inputitemlabel{\
        margin-right: 10px;\
        color:#fff;\
        font-size: 16px;\
    }\
    .titlebox{\
        text-align: center;\
        font-size: 20px;\
        color:#fff;\
        padding:30px 0;\
    }\
    </style>\
  </head>\
  <body>\
    <div class='titlebox'>LNESP 物联连接WiFi成功</div>\
         <div class='inputitem'>\
            此设备编码:<span id='device_id'>"+macAddress+"</span>\
            ,去官网 <a href='https://ws.allianceshopping.cn/devices/index.html?device_id="+macAddress+"'>绑定设备</a>\
         </div>\
         <script>\
             window.open('https://ws.allianceshopping.cn/devices/index.html?device_id="+macAddress+"');\
         </script>\
  </body>\
  </html>";

void LnWifiClient::startWifi ()
{
   connectNewWifi();
}

void handleRoot() {//访问主页回调函数
  webServer.send(200, "text/html", page_html);
}

void handleRootPost() {//Post回调函数
  Serial.println("handleRootPost");
  if (webServer.hasArg("ssid")) {//判断是否有账号参数
    Serial.print("got ssid:");
    strcpy(sta_ssid, webServer.arg("ssid").c_str());//将账号参数拷贝到sta_ssid中
    Serial.println(sta_ssid);
  } else {//没有参数
    Serial.println("error, not found ssid");
    webServer.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid");//返回错误页面
    return;
  }
  //密码与账号同理
  if (webServer.hasArg("password")) {
    Serial.print("got password:");
    strcpy(sta_password, webServer.arg("password").c_str());
    Serial.println(sta_password);
  } else {
    Serial.println("error, not found password");
    webServer.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }
 webServer.send(200, "text/html",successpage);//返回保存成功页面
  delay(2000);
  //连接wifi
 mlnWifiClient.connectNewWifi();
}



void LnWifiClient::initSoftAP(void) { //初始化AP模式
  Serial.println("initSoftAPstart");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(AP_NAME)) {
    Serial.println("ESP8266 SoftAP is right");
  }
}


void LnWifiClient::loop(void) { //初始化AP模式
    dnsServer.processNextRequest();
  webServer.handleClient();
}

void LnWifiClient::initWebServer(void) { //初始化WebServer
  webServer.on("/",handleRoot);
  //上面那行必须以下面这种格式去写否则无法强制门户
 webServer.on("/", HTTP_GET, handleRoot);//设置主页回调函数
 webServer.onNotFound(handleRoot);//设置无法响应的http请求的回调函数
 webServer.on("/", HTTP_POST, handleRootPost);//设置Post请求回调函数
 webServer.begin();//启动WebServer
 Serial.println("WebServer started!");
}

void LnWifiClient::initDNS(void) { //初始化DNS服务器
  if (dnsServer.start(DNS_PORT, "*", apIP)) { //判断将所有地址映射到esp8266的ip上是否成功
    Serial.println("start dnsserver success.");
  }
  else Serial.println("start dnsserver failed.");
   //ESP.wdtFeed();
}

void LnWifiClient::connectNewWifi(void) {
  Serial.println("connectNewWifistart");
      initSoftAP();
      initWebServer();
      initDNS();
        Serial.print("Connect to wifiAP");
}
