#include <ESP8266WiFi.h>  // WiFi
#include <PubSubClient.h> // mqtt
#include "DHTesp.h"       // 온습도 센서


//// 아두이노가 접속할 WiFi AP/password
//const char* ssid = "U+NetA96C_5G"; 
//const char* password = "2$C6EAG59C";  

// 아두이노가 접속할 WiFi AP/password
const char* ssid = "networking407_2.4G"; 
const char* password = "networking407";  

// mqtt 브로커 주소 & mqtt 브로커에 접속하는 클라이언트 이름
const char* mqtt_server = "192.168.15.79";
const char* clientName = "Client1"; // 중복되지 않도록 수정

DHTesp dht;                      // 온습도 처리 객체
WiFiClient espClient;            // WiFi 접속 객체
PubSubClient client(espClient);  // mqtt 클라이언트 객체

// WiFi 접속-----------------------------------
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // WiFi AP 접속 시도

  // WiFi AP 접속 시도
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // WiFi AP 접속시 시리얼 모니터에 성공 메시지 출력
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // D4 핀을 온습도 센서 데이터 입력으로 설정
  dht.setup(D4, DHTesp::DHT11); 
}

// mqtt 토픽 수신/처리 콜백 함수-----------------------
// pBuffer: 토픽에 담긴 데이터 저장
void callback(char* topic, byte* payload, unsigned int uLen) {
   char pBuffer[uLen+1];
   int i;
   for(i=0;i<uLen;i++){
         pBuffer[i]=payload[i];
   }
   pBuffer[i]='\0';
   Serial.println(pBuffer); // 1, 0

   // 토픽으로 들어온 데이터가 0[1]이면 LED RED[GREEN]
   if(pBuffer[0]=='0'){         // LED_RED(ON), LED_GREEN(OFF)
      digitalWrite(14, HIGH);
      digitalWrite(12, LOW);
   }else if(pBuffer[0]=='1'){   // LED_RED(OFF), LED_GREEN(ON)
      digitalWrite(14, LOW);
      digitalWrite(12, HIGH);
   }  
}

// mqtt 브로커에 접속 시도-------------------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // mqtt 브로커 접속 시도
    if (client.connect(clientName)) {
      Serial.println("connected");
      // mqtt 브로커로부터 "led_ctr_state" 토픽 수신
      client.subscribe("led_ctr_state");
      // mqtt 브로커로부터 "led" 토픽 수신
   //   client.subscribe("led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // 5초 마다 접속 재시도
    }
  }
}



// 아두이노 설정, WiFi AP 및 mqtt 브로커 접속-----------
void setup() {
  pinMode(14, OUTPUT);     // GPIO 14번 핀을 출력모드(LED)로 설정 (RED)
  pinMode(12, OUTPUT);     // GPIO 12번 핀을 출력모드(LED)로 설정 (GREEN)
  Serial.begin(115200); 
  setup_wifi();            // WiFi AP 접속
  client.setServer(mqtt_server, 1883); // mqtt 브로커에 접속
  client.setCallback(callback); // mqtt 브로커거 보낸 토픽 수신(callback 함수 등록)
}

// mqtt 브로커 접속 시도-------------------------------
 void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // mqtt 브로커로부터 토픽 청취

  float hum = dht.getHumidity();    // 습도 
  float tmp = dht.getTemperature(); // 온도
  char message[64]="", pTmpBuf[50], pHumBuf[50];
  dtostrf(tmp, 5,2, pTmpBuf); // 총 5자리, 소수점 아래 2째 자리까지 표시
  dtostrf(hum, 5,2, pHumBuf); 

  //led 제어

  
  // mqtt 브로커에 전송할 메시지(JSON 형식) 생성
  sprintf(message, "{\"tmp\":%s,\"hum\":%s}", pTmpBuf, pHumBuf);   
  Serial.print("Publish message: ");
  Serial.println(message);

  // mqtt 브로커에 "dht11" 토픽을 발행해서 메시지를 전송
  client.publish("dht11", message);   
  delay(3000); // 3초에 한 번씩 메시지 전송
}
