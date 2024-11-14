/* 
 * Project Flower Power
 * Author: Joeseph Arnoux
 * Date: 11/12/24
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

#include "Particle.h"
#include "Adafruit_BME280.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Joebitmap.h"
#include "IotClassroom_CNM.h"
#include "Button.h"
#include "Grove_Air_quality_Sensor.h"
#include <credentials.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h" 
#include "Adafruit_MQTT/Adafruit_MQTT.h"

SYSTEM_MODE(AUTOMATIC);
bool status;
float tempC;
float tempF;
float humid;
int soilRead;
int pumpTimer;
int currentTime;
int lastSecond;
bool onoff;
int currentQual = -1;
unsigned long duration;
unsigned long startTime;
unsigned long sampleTime_ms = 30000;
unsigned long lowPulseOcc = 0;
float ratio = 0;
float concentration = 0;
char degree = 0xF8;
char perc = 0x25;
const int soilSensor = A1; 
const int pump = S1;
Button encoButton(D3);
float buttonOnOffValue=D3;
const int OLED_RESET=-1;
void WebPublish();
void MQTT_connect();
bool MQTT_ping();

AirQualitySensor aqSensor(A2);
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Subscribe buttonOnOffFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttonOnOff");
Adafruit_MQTT_Publish pubAQ = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Air Quality");  
Adafruit_MQTT_Publish pubTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature"); 
Adafruit_MQTT_Publish pubMoist = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Soil Moisture"); 
Adafruit_MQTT_Publish pubHumid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_BME280 bme;
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
Serial.begin(9600);
waitFor(Serial.isConnected,10000);
  pinMode(pump,OUTPUT);
  pinMode(soilSensor,INPUT);
mqtt.subscribe(&buttonOnOffFeed);

status = bme.begin(0x76);
    if (status == false) {
      Serial.printf("BME280 at address 0x%02X failed to start",0x76);
    }
   display.clearDisplay();
   display.drawBitmap(0, 0, wheel, 128, 64, 1);
   display.display();
   delay(3000);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();
  delay(500);
  
}

void loop() {
MQTT_connect();
MQTT_ping();

Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &buttonOnOffFeed) {
      buttonOnOffValue = atof((char *)buttonOnOffFeed.lastread);
    }
  }

if (encoButton.isClicked() ||buttonOnOffValue ==1) {
    digitalWrite(pump,HIGH);
    Serial.printf("Pumping H20!\n");
    }
    else {
    digitalWrite(pump,LOW);
    }

tempC = bme.readTemperature();
  tempF = (tempC*9.0/5.0)+32;
  currentTime = millis();
    if ((currentTime-lastSecond)>500) {
      lastSecond = millis();
      display.clearDisplay();
      display.setCursor(0,0);
      display.printf("TEMP: %f\n",tempF);
    }
    display.display();

soilRead = analogRead(soilSensor);
humid = bme.readHumidity();

Serial.printf("Moisture Level = %i\n",soilRead);
  if (soilRead>2000) {
    Serial.printf("Hydration Station!");
    digitalWrite(pump,HIGH);
    delay(500);
    digitalWrite(pump,LOW);
    }

  if ((millis()-pumpTimer)>500) {
    digitalWrite(pump,LOW);
    pumpTimer = millis();
    }
    
currentQual = aqSensor.slope();
  if (currentQual>= 0) {
    if (currentQual==3)
      Serial.printf("Dangerous Pollution Levels! Force signal active\n");
    else if (currentQual==2)
      Serial.printf("Warning High pollution!\n");
    else if (currentQual==1)
      Serial.printf("Caution Low Pollution\n");
    else if (currentQual==0)
      Serial.printf("Fresh Air\n");

  }

WebPublish();
  
}

//END

void MQTT_connect() {
  int8_t ret;
 
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { 
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

void WebPublish() {
  static int tempTimer;
  static int soilTimer;
  static int humidTimer;
  static int aqTimer;

  if ((millis()-tempTimer>5000)) {
    if (mqtt.Update()) {
      pubTemp.publish(tempF);
      Serial.printf("Publishing Temperature: %0.2f\n",tempF);
    }
  tempTimer = millis();
  }

  if ((millis()-soilTimer>10000)) {
    if (mqtt.Update()) {
      pubMoist.publish(soilRead);
      Serial.printf("Publishing Soil Moisture %i\n",soilRead);
    }
  soilTimer = millis();
  }

  if((millis()-humidTimer>15000)) {
    if(mqtt.Update()) {
      pubHumid.publish(humid);
      Serial.printf("Publishing Humidity %i\n",soilRead);
    }
  humidTimer = millis();
  }

  // if((millis()-aqTimer>8000)) {
  //   switch(currentQual) {
  //     case 0:
  //     if(mqtt.Update()) {
  //       pubAQ.publish("Fresh Air");
  //     }
  //   break;

  //     case 1:
  //     if(mqtt.Update()) {
  //       pubAQ.publish("Caution Low Pollution");
  //     }
  //   break;

  //     case 2:
  //     if(mqtt.Update()) {
  //       pubAQ.publish("Warning High Pollution!");
  //     }
  //   break;

  //     case 3:
  //     if(mqtt.Update()) {
  //       pubAQ.publish("Dangerous Pollution Levels");
  //     }
  //   break;

  //   }
  // }
}

