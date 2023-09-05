#include "Arduino.h"
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
#include "ArialNarrow7pt7b.h"
//#include "SourceHanMono_Regular7pt7b.h"
//#include "FreeMonoBold7pt7b.h"
#define DEFAULT_FONT ArialNarrow7pt7b
#include "Digital_7pt7b.h"
#include "BitmapFan.h"
#define DATA_UPDATE_INTERVAL 500
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// SDA SCL
TwoWire WireOLED(PB11, PB10);
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &WireOLED, OLED_RESET);

CSPS PowerSupply_1(0x5F, 0x57, PB9, PB8);

float VoltIn, VoltOut,
    CurrentIn, CurrentOut,
    PowerIn, PowerOut,
    Temp1,
    Temp2,
    FanSpeed;
float Efficiency;
bool PowerGood;
bool PowerGoodOled;

String OutputString;
uint8_t OutputLen = 0;

bool displayInitialized;

const float animation_update_intervals[] = {500, 250, 166.66, 125, 100, 83.33, 71.43, 62.5, 55.55, 50};
const int animation_update_intervals_len = 10;
const float fan_speed_lbound = 500;
const float fan_speed_hbound = 10000;
const float fan_speed_range = 9500;
int animation_frame = 0;
int animation_update_interval_idx = 0;
float data_update_timer = 0;

void update_oled_pg(bool force)
{
  if (PowerGoodOled == PowerGood && !force)
  {
    return;
  }
  display.setFont(&DEFAULT_FONT);
  if (PowerGood)
  {
    display.fillRoundRect(92, 0, 32, 14, 4, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(99, 11);
    display.print("ON");
  }
  else
  {
    display.fillRect(92, 0, 32, 14, SSD1306_BLACK);
    display.drawRoundRect(92, 0, 32, 14, 4, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(96, 11);
    display.print("OFF");
  }
  PowerGoodOled = PowerGood;
}

void setup()
{
  Serial.begin(115200);
  Wire.setClock(100000);

  displayInitialized = false;
  //display.setFont(&SourceHanMono_Regular7pt7b);
  //display.setFont(&FreeMono9pt7b);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // https://github.com/olikraus/u8g2/issues/1806
    // https://www.buydisplay.com/download/ic/SSD1312_Datasheet.pdf Pg. 51 Section 2.1.19
    //            normal    inverted
    // normal     A1 C8       A0 C0
    // mirrored   A0 C8       A1 C0
    // combination1: A0 C8
    // combination2: A1 C0
    display.ssd1306_command(0xA1);
    display.ssd1306_command(0xC0);
    displayInitialized = true;
  } else {
    //Serial.println(F("SSD1306 allocation failed"));
  }
  
  if (displayInitialized)
  {
    display.setTextSize(1);      // pixel scale
    //display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    //display.setRotation(2);      // rotate screen
    
    display.clearDisplay();
    display.fillScreen(SSD1306_BLACK);
    // fixed content
    // header
    display.fillRect(0, 0, 80, 16, SSD1306_WHITE);
    // vlines
    display.drawFastVLine(0, 0, 64, SSD1306_WHITE);
    display.drawFastVLine(40, 0, 16, SSD1306_BLACK);
    display.drawFastVLine(40, 16, 31, SSD1306_WHITE);
    display.drawFastVLine(80, 0, 64, SSD1306_WHITE);
    // hlines
    display.drawFastHLine(0, 47, 80, SSD1306_WHITE);
    display.drawFastHLine(0, 63, 80, SSD1306_WHITE);
    // header text
    display.setTextColor(SSD1306_BLACK);
    display.setFont(&DEFAULT_FONT);
    display.setCursor(12, 12);     // Start at top-left corner
    display.print("AC");
    display.setCursor(52, 12);
    display.print("DC");
    // unit
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(31, 28);
    display.print("V");
    display.setCursor(31, 42);
    display.print("A");
    display.setCursor(71, 28);
    display.print("V");
    display.setCursor(71, 42);
    display.print("A");
    display.setCursor(20, 60);
    display.print("%");
    display.setCursor(70, 60);
    display.print("W");
    // pg
    update_oled_pg(true);
  }
  
}

void get_csps_values()
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
  Efficiency = PowerOut * 100 / PowerIn;
  if (Efficiency >= 99)
    Efficiency = 99;
  else if (Efficiency < 0)
    Efficiency = 0;
  // FIXME: should be decided by PowerSupply_1.getFlags()
  PowerGood = true;
}

void test_csps_values()
{
  VoltIn = 226.23;
  VoltOut = 12.21;
  CurrentIn = 6.11;
  CurrentOut = 106.01;
  PowerIn = 1347.2;
  PowerOut = 1293.3;
  //Temp1 = PowerSupply_1.getTemp1();
  //Temp2 = PowerSupply_1.getTemp2();
  FanSpeed = 12000;
  Efficiency = PowerOut * 100 / PowerIn;
  PowerGood = true;
}

void print_to_serial()
{
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
}

uint32_t digits10(uint64_t v) {
    if (v < 10) return 1;
    if (v < 100) return 2;
    if (v < 1000) return 3;
    if (v < 1000000000000) {    // 10^12
        if (v < 100000000) {    // 10^8
            if (v < 1000000) {  // 10^6
                if (v < 10000) return 4;
                return 5 + (v >= 100000); // 10^5
            }
            return 7 + (v >= 10000000); // 10^7
        }
        if (v < 10000000000) {  // 10^10
            return 9 + (v >= 1000000000); // 10^9
        }
        return 11 + (v >= 100000000000); // 10^11
    }
    return 12 + digits10(v / 1000000000000); // 10^12
}

float round_to(float value, float precision = 1.0)
{
    return std::round(value / precision) * precision;
}

void update_oled_values()
{
  // dynamic data
  display.setTextColor(SSD1306_WHITE);
  display.setFont(&Digital_7pt7b);
  // AC info
  display.fillRect(1, 16, 30, 31, SSD1306_BLACK);
  if (VoltIn >= 0 && VoltIn < 999)
  {
    display.setCursor(2, 30);
    display.print(String(round(VoltIn), 0));
  }
  display.setCursor(2, 44);
  if (CurrentIn >= 10 && CurrentIn <= 99)
  {
    display.print(String(round_to(CurrentIn, 0.1), 1));
  }
  else if (CurrentIn >= 0 && CurrentIn < 10)
  {
    display.print(String(round_to(CurrentIn, 0.01), 2));
  }
  // DC info
  display.fillRect(41, 16, 30, 31, SSD1306_BLACK);
  if (VoltOut >= 0 && VoltOut < 99)
  {
    display.setCursor(42, 30);
    display.print(String(round_to(VoltOut, 0.1), 1));
  }
  display.setCursor(42, 44);
  if (CurrentOut >= 100 && CurrentOut <= 999)
    display.print(String(round(CurrentOut), 0));
  else if (CurrentOut >= 0 && CurrentOut < 100)
    display.print(String(round_to(CurrentOut, 0.1), 1));
  // PWR info
  display.setCursor(2, 61);
  display.print(String(round(Efficiency), 0));
  display.fillRect(35, 48, 34, 13, SSD1306_BLACK);
  // display.setCursor(35, 61);
  display.setCursor(67 - 8*digits10((int)PowerOut), 61);
  display.print(String(round(PowerOut), 0));
  // Fan info
  display.fillRect(81, 48, 48, 13, SSD1306_BLACK);
  //display.setCursor(81, 61);
  display.setCursor(105-(digits10((int)FanSpeed)+1)*8/2, 61);
  display.print(String(round(FanSpeed), 0)+ "R");
  // On/Off
  update_oled_pg(false);
}

void update_oled_animation_frame()
{
  display.fillRect(92, 16, 32, 32, SSD1306_BLACK);
  display.drawBitmap(92, 16, fan_bitmap_allArray[animation_frame], 32, 32, SSD1306_WHITE);
  animation_frame++;
  if (animation_frame >= fan_bitmap_allArray_LEN)
  {
    animation_frame = 0;
  }
}

void loop()
{
  unsigned long timeBegin = millis();
  if (data_update_timer == 0)
  {
    //get_csps_values();
    test_csps_values();
    // update animation speed
    if (FanSpeed <= fan_speed_lbound)
    {
      animation_update_interval_idx = 0;
    }
    else if (FanSpeed >= fan_speed_hbound)
    {
      animation_update_interval_idx = animation_update_intervals_len - 1;
    }
    else
    {
      animation_update_interval_idx = floor((FanSpeed - fan_speed_lbound) * animation_update_intervals_len / fan_speed_range);
    }
  }

  if (data_update_timer == 0)
  {
    print_to_serial();
  }

 
  if (displayInitialized)
  {
    if (data_update_timer == 0)
    {
      update_oled_values();
    }
    // Fan anim
    update_oled_animation_frame();
      
    display.display();
  }
  
  float delay_ms = animation_update_intervals[animation_update_interval_idx] - (float)(millis() - timeBegin);
  data_update_timer += animation_update_intervals[animation_update_interval_idx];
  if (abs(DATA_UPDATE_INTERVAL-data_update_timer) < 10)
    data_update_timer = 0;
  if (delay_ms > 0)
    delay((int)delay_ms);
}

void serialEvent()
{
  PowerSupply_1.setFanRPM(Serial.readStringUntil('\n').toInt());
}
