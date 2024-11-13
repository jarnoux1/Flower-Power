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

SYSTEM_MODE(SEMI_AUTOMATIC);
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
const int OLED_RESET=-1;
void WebPublish();

AirQualitySensor aqSensor(A2);
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish pubAQ = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Air Quality");  
Adafruit_MQTT_Publish pubTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temp"); 
Adafruit_MQTT_Publish pubMoist = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Soil Moisture"); 
Adafruit_MQTT_Publish pubHumid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_BME280 bme;
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
Serial.begin(9600);
waitFor(Serial.isConnected,10000);
  pinMode(pump,OUTPUT);
  pinMode(soilSensor,INPUT); 

status = bme.begin(0x76);
    if (status == false) {
      Serial.printf("BME280 at address 0x%02X failed to start",0x76);
    }
   display.clearDisplay();
   display.drawBitmap(0, 0, wheel, 128, 64, 1);
   display.display();
   delay(1000);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.display();
  delay(500);
  
}

void loop() {
tempC = bme.readTemperature();
  tempF = (tempC*9/5)+32;
  currentTime = millis();
    if((currentTime-lastSecond)>500) {
      lastSecond = millis();
      display.clearDisplay();
      display.setCursor(0,0);
      display.printf("TEMP: %f\n",tempF);
    }
    display.display();

soilRead = analogRead(soilSensor);
humid = bme.readHumidity();

Serial.printf("Moisture Level = %i\n",soilRead);
  if(soilRead>2000) {
    Serial.printf("pump on");
    digitalWrite(pump,HIGH);
    delay(500);
    digitalWrite(pump,LOW);
    }

  if((millis()-pumpTimer)>500) {
    Serial.printf("Pump Off");
    digitalWrite(pump,LOW);
    pumpTimer = millis();
    }

  if (encoButton.isClicked()) {
    onoff = !onoff;
    }

  if (onoff == true) {
    digitalWrite(pump,HIGH);
    }
    else {
    digitalWrite(pump,LOW);
    }
  
}
