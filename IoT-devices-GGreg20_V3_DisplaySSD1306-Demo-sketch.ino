/*
    IoT-devices GGreg20_V3 with Display SSD1306 Demo sketch
    Filename: IoT-devices-GGreg20_V3_DisplaySSD1306-Demo-sketch.ino

    This is a demonstration sketch that clearly shows how 
    the GGreg20_V3 radiation detector connects and works with 
    the Arduino UNO controller. 
    This example shows a valid code that can be used with a real controller 
    and module GGreg20_V3 manufactured by IoT-devices LLC.
    In addition to the Arduino controllers, the GGreg20_V3 can work with any 
    other controller that can handle GPIO events, or even those that can measure 
    logic through an ADC (if needed). 
    For example, such controllers are ESP32, ESP8266, STM32, Raspberry Pi and many others.

    The circuit:
    * GGreg20_V3 Ionizing Radiation Detector's pulse output is emulated via simple pushbutton
    * Arduino UNO
    * OLED Display SSD1306, I2C
    * LED blinks with pulses

    Created 13-01-2022
    By IoT-devices LLC, Kyiv, Ukraine
    Modified 13-01-2022
    By IoT-devices LLC
    Product page: https://iot-devices.com.ua/en/product/ggreg20_v3-ionizing-radiation-detector-with-geiger-tube-sbm-20/
    Hackaday Project: https://hackaday.io/project/183103-ggreg20v3-ionizing-radiation-detector
    
    Licensed under Apache-2.0 License

*/
//
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

int pin = 13;
byte interruptPin = 2;
int cpm = 0;
volatile byte state = LOW;
volatile unsigned long now = millis ();
const int debounceTime = 5;  // debounce in milliseconds
volatile unsigned long measure_time = 60000;
unsigned long meas_start = millis ();
int ma5_pointer = 0;
float ma5_arr[5] = {};
float ma5_val = 0;
int ma5_arr_elements = 0;

SSD1306AsciiWire oled;

void setup() {
  ma5_arr[0] = 0;
  ma5_arr[1] = 0;
  ma5_arr[2] = 0;
  ma5_arr[3] = 0;
  ma5_arr[4] = 0;
  ma5_arr[5] = 0;
  ma5_pointer = 0;
  ma5_val = 0;
  ma5_arr_elements = 0;

  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000L);
  pinMode(pin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), counter, LOW);

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0
Serial.println(meas_start);
}


void loop() {
  digitalWrite(pin, state);
  state = LOW;
  // Main 1-pass measure 60 seconds timer
  if (millis() - meas_start >= measure_time) {
    calc();
    cpm = 0;
    meas_start = millis();
  }
  Serial.println(millis() - meas_start);
  Serial.println(ma5_pointer);
  Serial.println(ma5_val);
  Serial.println(ma5_arr_elements);
  Serial.println(ma5_arr[0]);
  Serial.println(ma5_arr[1]);
  Serial.println(ma5_arr[2]);
  Serial.println(ma5_arr[3]);
  Serial.println(ma5_arr[4]);
  Serial.println(cpm);
  delay(1000);
  disp();

}

void counter() {
  // Pin deBounce conditioning
  if (digitalRead(interruptPin) == LOW && (millis() - now) >= debounceTime) {
    cpm = cpm + 1;
    now = millis ();
    blink();
  }
}

void blink() {
  state = HIGH;
  Serial.println(state);
  delay(30);
}

void disp() {
  oled.clear();
  oled.setFont(lcdnums14x24);
  oled.setCursor(0,0);
  oled.print(cpm);
  oled.setFont(Verdana12);
  oled.println(" CPM");
  oled.setCursor(0,4);
  oled.setFont(lcdnums14x24);
  oled.print(ma5_val);
  oled.setFont(Verdana12);
  oled.print(" uSv/h MA5");
  oled.setCursor(92,0);
  oled.print((measure_time - (millis() - meas_start))/1000);
  oled.print(" sec");
  delay(500);
}

void calc() {
  ma5_arr[ma5_pointer] = cpm * 0.0054; // 0.0054 conversion factor for SBM-20 GM tube

  byte i = 0;
  while(i < 5 && ma5_arr[i] != 0){
    ma5_arr_elements = (i + 1);
    i++;
  }

  byte j = 0;
  while(j < ma5_arr_elements && ma5_arr[j] != 0){
    ma5_val = ma5_val + ma5_arr[j];
    j++;
  }

  if(ma5_arr_elements != 0){ma5_val = ma5_val / ma5_arr_elements;}
  if (ma5_pointer < 4){ma5_pointer++;} else {ma5_pointer = 0;}

}