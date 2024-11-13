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


SYSTEM_MODE(SEMI_AUTOMATIC);
bool status;
float tempC;
float tempF;
int currentTime;
int lastSecond;
const int OLED_RESET=-1;
Button encoButton(D3);
bool onoff;

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme;

void setup() {
Serial.begin(9600);
waitFor(Serial.isConnected,10000);

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

    if (encoButton.isClicked()) {
    onoff = !onoff;
    }

    if (onoff == true) {
    
    }
    else {
    
    }
  
}
