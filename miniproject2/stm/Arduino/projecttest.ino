/*
  WiFiEsp test: ClientTest
  http://www.kccistc.net/
  작성일 : 2022.12.19
  작성자 : IoT 임베디드 KSH
*/
#define DEBUG
//#define DEBUG_WIFI

#include <WiFiEsp.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <Wire.h>
#include <DHT.h>
#include <MQ135.h>

#define AP_SSID "embA"
#define AP_PASS "embA1234"
#define SERVER_NAME "10.10.14.48"
#define SERVER_PORT 5000
#define LOGID "KSH_ARD"
#define PASSWD "PASSWD"

#define CDS_PIN A0
#define DHTPIN A2
#define WIFIRX 8  //8:RX-->ESP8266 TX
#define WIFITX 7  //7:TX -->ESP8266 RX
#define MOTOR_PIN 6
#define LED_BUILTIN_PIN 13

#define CMD_SIZE 100
#define ARR_CNT 6
#define DHTTYPE DHT11
bool timerIsrFlag = false;
boolean lastButton = LOW;     // 버튼의 이전 상태 저장
boolean currentButton = LOW;    // 버튼의 현재 상태 저장
boolean ledOn = false;      // LED의 현재 상태 (on/off)
boolean cdsFlag = false;

char sendId[10] = "CJW_ARD";
char sendBuf[CMD_SIZE];

int motorState = 0;  // 0: AUTO, 1: ON, 2: OFF
int motorPWM = 0;    // PWM value for motor speed
int cds = 0;
unsigned int secCount;

char getSensorId[10];
int sensorTime;
float temp, humi, rs,correctedRs,correctedPPM  = 0.0;
float ro = 55;
bool updateTimeFlag = false;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0};
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial wifiSerial(WIFIRX, WIFITX);
WiFiEspClient client;

MQ135 gasSensor(CDS_PIN);

void setup() {
  pinMode(CDS_PIN, INPUT);    // 조도 핀을 입력으로 설정 (생략 가능)
  pinMode(LED_BUILTIN_PIN, OUTPUT); //D13

#ifdef DEBUG
  Serial.begin(115200); //DEBUG
#endif
  wifi_Setup();


  MsTimer2::set(1000, timerIsr); // 1000ms period
  MsTimer2::start();

  dht.begin();
}

void loop() {
  if (client.available()) {
    socketEvent();
  }

  if (timerIsrFlag) //1초에 한번씩 실행
  {
    timerIsrFlag = false;
    if (!(secCount % 5)) //5초에 한번씩 실행
    {

      // float rzero = gasSensor.getRZero();  // 센서 캘리브레이션 값
      // gasSensor.setRZero(rzero);
      humi = dht.readHumidity();
      temp = dht.readTemperature();

      rs = gasSensor.getResistance();
  
      correctedRs = applyTemperatureHumidityCorrection(rs, temp, humi);
      correctedPPM = calculateVOSPPM(correctedRs, ro);

      Serial.println(" ");
      cds = map(correctedPPM, 0, 1023, 0, 100);
      cds = correctedPPM;
      motorPWM = map(correctedPPM, 0, 1023, 0, 255); 

      

      if (motorState == 0 && (cds >= 30 || humi >=35))
      {  // AUTO 모드
        analogWrite(MOTOR_PIN, 255);  // 모터 속도 설정
      }
      else if (motorState == 0 && (cds < 30 && humi <35))
      {  // AUTO 모드
        analogWrite(MOTOR_PIN, 0);  // 모터 속도 설정

      }
      else if (motorState == 1) 
      {  // ON 모드
        motorPWM = 255;  // 최대 속도로 설정
        analogWrite(MOTOR_PIN, motorPWM);  // 모터 속도 설정

      }
      else if (motorState == 2) {  // OFF 모드
        motorPWM = 0;  // 모터 끄기
        analogWrite(MOTOR_PIN, motorPWM);  // 모터 속도 설정
      }

#ifdef DEBUG
            Serial.print("Cds: ");
            Serial.print(cds);
            Serial.print(" Humidity: ");
            Serial.print(humi);
            Serial.print(" Temperature: ");
            Serial.println(temp);
 
#endif

      if (!client.connected()) {
        server_Connect();
      }
    }
    if (sensorTime != 0 && !(secCount % sensorTime ))
    {
        char tempStr[5];
        char humiStr[5];
        dtostrf(humi, 4, 1, humiStr);  //50.0 4:전체자리수,1:소수이하 자리수
        dtostrf(temp, 4, 1, tempStr);  //25.1
        sprintf(sendBuf,"[ALLMSG]%s@SENSOR@%d@%s@%s\r\n",getSensorId,cds,tempStr,humiStr);
      
      client.write(sendBuf, strlen(sendBuf));
      client.flush();

    }
    if (updateTimeFlag)
    {
      client.print("[GETTIME]\n");
      updateTimeFlag = false;
    }
  }
}
void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;

  sendBuf[0] = '\0';
  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  if (!strncmp(pArray[1], " New", 4)) // New Connected
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    updateTimeFlag = true;
    return ;
  }
  else if (!strncmp(pArray[1], " Alr", 4)) //Already logged
  {
#ifdef DEBUG
    Serial.write('\n');
#endif
    client.stop();
    server_Connect();
    return ;
  }
  else if (!strncmp(pArray[1], "GETSENSOR", 9)) {
    if (pArray[2] != NULL) {
      sensorTime = atoi(pArray[2]);
      strcpy(getSensorId, pArray[0]);
      return;
    } else {
      sensorTime = 0;
      sprintf(sendBuf, "[%s]%s@%d@%d@%d\n", pArray[0], pArray[1], cds, (int)temp, (int)humi);
    }
  }
#ifdef DEBUG
    Serial.write('\n');
#endif
  if(!strncmp(pArray[1], "MOTOR", 5))
  {
    if(!strncmp(pArray[2], "AUTO", 4))
    {
      motorState = 0;
      Serial.println("MOTOR mode : AUTO");
    }
    else if(!strncmp(pArray[2], "ON",2))
    {
      motorState = 1;
      Serial.println("MOTOR mode : ON");
    }
    else if(!strncmp(pArray[2],"OFF",3))
    {
      motorState = 2;
      Serial.println("MOTOR mode : OFF");
    }
  }
  else if(!strcmp(pArray[0],"GETTIME")) {  //GETTIME
    dateTime.year = (pArray[1][0]-0x30) * 10 + pArray[1][1]-0x30 ;
    dateTime.month =  (pArray[1][3]-0x30) * 10 + pArray[1][4]-0x30 ;
    dateTime.day =  (pArray[1][6]-0x30) * 10 + pArray[1][7]-0x30 ;
    dateTime.hour = (pArray[1][9]-0x30) * 10 + pArray[1][10]-0x30 ;
    dateTime.min =  (pArray[1][12]-0x30) * 10 + pArray[1][13]-0x30 ;
    dateTime.sec =  (pArray[1][15]-0x30) * 10 + pArray[1][16]-0x30 ;
#ifdef DEBUG
#endif
    return;
  } 
  else
    return;

  client.write(sendBuf, strlen(sendBuf));
  client.flush();

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
  clock_calc(&dateTime);
}
void clock_calc(DATETIME *dateTime)
{
  int ret = 0;
  dateTime->sec++;          // increment second

  if(dateTime->sec >= 60)                              // if second = 60, second = 0
  { 
      dateTime->sec = 0;
      dateTime->min++; 
             
      if(dateTime->min >= 60)                          // if minute = 60, minute = 0
      { 
          dateTime->min = 0;
          dateTime->hour++;                               // increment hour
          if(dateTime->hour == 24) 
          {
            dateTime->hour = 0;
            updateTimeFlag = true;
          }
       }
    }
}

void wifi_Setup() {
  wifiSerial.begin(38400);
  wifi_Init();
  server_Connect();
}
void wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI
      Serial.println("WiFi shield not present");
#endif
    }
    else
      break;
  } while (1);

#ifdef DEBUG_WIFI
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif
  while (WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {
#ifdef DEBUG_WIFI
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);
#endif
  }

#ifdef DEBUG_WIFI
  Serial.println("You're connected to the network");
  printWifiStatus();
#endif
}
int server_Connect()
{
#ifdef DEBUG_WIFI
  Serial.println("Starting connection to server...");
#endif

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI
    Serial.println("Connect to server");
#endif
    client.print("["LOGID":"PASSWD"]");
  }
  else
  {
#ifdef DEBUG_WIFI
    Serial.println("server connection failure");
#endif
  }
}
void printWifiStatus()
{

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

float calculateRo() {
    float sensorResistance = gasSensor.getResistance();
    return sensorResistance / 3.6;  // NH₃에서 Rs/Ro 비율 기준
}

float calculateVOSPPM(float rs, float ro) {
    float ratio = rs / ro;
    float ppm = pow(10, (log10(ratio) - 0.8) / -0.3);  // 데이터시트 곡선 추정 상수
    return ppm;
}

float applyTemperatureHumidityCorrection(float rs, float temp, float humi) {
    // 입력값 범위 확인 및 기본값 설정
    if (temp < -10 || temp > 45) {
        Serial.println("Warning: Temperature out of range! Defaulting to 25°C");
        temp = 25.0;  // 기본 온도
    }
    if (humi < 0 || humi > 95) {
        Serial.println("Warning: Humidity out of range! Defaulting to 50%");
        humi = 50.0;  // 기본 습도
    }

    // 보정 계수 계산
    float correctionFactor = 0.00035 * temp * temp + 0.02718 * temp + 1.39538 -
                             (0.0018 * humi + 0.003333 * humi * humi) * (0.0018 * temp + 1.13);

    // 보정 계수가 0 이하로 계산되는 경우 최소값 설정
    if (correctionFactor <= 0) {
        correctionFactor = 1.0;  // 최소값
    }

    // 보정된 Rs 반환
    return rs / correctionFactor;
}

