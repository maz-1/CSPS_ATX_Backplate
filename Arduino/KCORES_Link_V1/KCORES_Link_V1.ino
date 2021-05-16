/*!
 * Work with KCORES Link V0.0.1 https://github.com/KCORES/kcores-link
 *
 * Written by AlphaArea
 *
 * GPL license, all text here must be included in any redistribution.
 *
 */

/* Modified by maz-1 for stm32f103c8t6 and 0.91 inch 12832 oled display
 * Should be compiled with stm32duino : https://github.com/stm32duino/BoardManagerFiles/raw/master/package_stmicroelectronics_index.json
 * Watch this video about how to install hid bootloader : https://www.youtube.com/watch?v=Myon8H111PQ
 * stm32f103c8t6 should be powered by onboard usb, it will also provide a virtual serial port that can be used by KCORES Link
 * Pin connections:
 *   PB8  -> CSPS SCL
 *   PB9  -> CSPS SDA
 *   G    -> CSPS GND
 *   PB10 -> 12832 SCL
 *   PB11 -> 12832 SDA
 *   V3   -> 12832 VCC
 *   G    -> 12832 GND
 */


#include "KCORES_CSPS.h"


#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <sarasa_light7pt7b.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// SDA SCL
TwoWire WireOLED(PB11, PB10);
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &WireOLED, OLED_RESET);
String OutputOLEDString;
uint8_t CurrentInAcc, CurrentOutAcc;
float efficiency;

CSPS PowerSupply_1(0x5F, 0x57, PB9, PB8);

float VoltIn, VoltOut,
    CurrentIn, CurrentOut,
    PowerIn, PowerOut,
    Temp1,
    Temp2,
    FanSpeed;

String OutputString;
uint8_t OutputLen = 0;

bool displayInitialized;

void setup()
{
  Serial.begin(115200);
  Wire.setClock(100000);

  
  display.setFont(&sarasa_light7pt7b);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    displayInitialized = false;
  } else {
    displayInitialized = true;
  }
  
  if (displayInitialized)
  {
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
  }
  
}

void loop()
{
  VoltIn = PowerSupply_1.getInputVoltage();
  VoltOut = PowerSupply_1.getOutputVoltage();
  CurrentIn = PowerSupply_1.getInputCurrent();
  CurrentOut = PowerSupply_1.getOutputCurrent();
  PowerIn = PowerSupply_1.getInputPower();
  PowerOut = PowerSupply_1.getOutputPower();
  Temp1 = PowerSupply_1.getTemp1();
  Temp2 = PowerSupply_1.getTemp2();
  FanSpeed = PowerSupply_1.getFanRPM();

  Serial.print(" ");
  OutputLen = 0;
  OutputString = String(VoltIn, 1) + ',' + String(CurrentIn, 2) + ',' + String(PowerIn, 1) + ',';
  OutputLen += OutputString.length();
  Serial.print(OutputString);

  OutputString = String(VoltOut, 2) + ',' + String(CurrentOut, 2) + ',' + String(PowerOut, 1) + ',';
  OutputLen += OutputString.length();
  Serial.print(OutputString);

  OutputString = String(Temp1, 1) + ',' + String(Temp2, 1) + ',' + String(FanSpeed, 0) + ',';
  OutputLen += OutputString.length();
  Serial.print(OutputString);
  Serial.print(",,");

  OutputLen = 59 - OutputLen;

  for (; OutputLen > 0; OutputLen--)
    Serial.print(" ");

  Serial.print("\n");

 
  if (displayInitialized)
  {
    display.clearDisplay();
    display.setCursor(0, 12);     // Start at top-left corner
    //display.println("I: 226V 268W 80%\nO: 12.23V 214.1W");
    efficiency = PowerOut * 100 / PowerIn;
    if (CurrentIn < 10)
      CurrentInAcc = 2;
    else
      CurrentInAcc = 1;
    if (CurrentOut < 10)
      CurrentOutAcc = 1;
    else
      CurrentOutAcc = 0;
    OutputOLEDString = "I: " + String(VoltIn, 0) + "V " + String(CurrentIn, CurrentInAcc) + "A " + String(PowerIn, 0) + "W\nO: " + String(VoltOut, 1) + "V " + String(CurrentOut, CurrentOutAcc) + "A " + String(PowerOut, 0) + "W";
    display.println(OutputOLEDString);
    display.display();
  }
  
  
  delay(500);
}

void serialEvent()
{
  PowerSupply_1.setFanRPM(Serial.readStringUntil('\n').toInt());
}
