#include <WiFi.h>
#include<EEPROM.h>

#define BTN_UP 0
#define BTN_DW 35
#define MAX_LINES 15
int LINE_HEIGHT = 12;
byte FONT_LINE_WIDTH = tft.width() / tft.textWidth(".", 1);

#define LONG_PRESS 500
#define eepromSSID 5
#define eepromPASS 35

long pressTime;
bool pressedBT_DW = false;
bool pressedBT_UP = false;
int16_t DISPLAY_WIDTH = tft.width();

int menuList(const char* title, const String list[]);
String getStringMenu(const char* title);

byte checkBT_DW();
byte checkBT_UP();

void menu()
{
  String menuItems[30];
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DW, INPUT_PULLUP);
  byte i = 2;
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Scanning WiFi:...", 0, 0);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  for (i = 0; i < n; i++)
    menuItems[i] = WiFi.SSID(i).c_str();
  menuItems[++i] = String();

  int ret = menuList("WiFi Netrorks:", menuItems);
  ssid = menuItems[ret];
  //  EEPROM.writeString(eepromSSID, ssid);
  delay(1000);

  pass = getStringMenu("Set password");
  //  EEPROM.writeString(eepromPASS, pass);
  //  EEPROM.commit();
  delay(1000);

  String menuItemsZone[30] = {"Eur/Warsaw", "Eur/+2", "Eur/+3", "US/TX", "dupa jasiu stasiu pupasiu pierdziasiu", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M"};
  menuList("Time zone:", menuItemsZone);
}



int16_t printLine(const String& txt, const int16_t x, const int16_t y, const uint16_t bg);
int16_t printRotateLine(const String& txt, const int16_t x, const int16_t y, const uint16_t bg);

int menuList(const char* title, const String list[])
{
  byte bt;

  LINE_HEIGHT = tft.fontHeight();
  byte DISPLAY_AREA_MAX = (tft.height() / tft.fontHeight(1)) - 1;
  byte DISPLAY_AREA_MIN = 1;
  byte indx = 0;
  byte line = DISPLAY_AREA_MIN;

  tft.fillScreen(TFT_BLACK);
  tft.setTextPadding(300);
  printLine(title, 0, 0, TFT_BLUE);

  //print list
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(0, LINE_HEIGHT);
  while (list[indx] != String() && line <= DISPLAY_AREA_MAX)
    printLine(list[indx++], 0, line++ * LINE_HEIGHT, TFT_BLACK);
  if (indx == 0)  //if there is no any element in list
    return -1;
  printLine(list[0], 0, DISPLAY_AREA_MIN * LINE_HEIGHT, TFT_GREEN);


  line = DISPLAY_AREA_MIN;
  indx = 0;
  for (;;)
  {
    bt = checkBT_DW();
    if (bt == 2) {
      if ( list[indx + 1] != String()) {
        if ( (line + 1) > DISPLAY_AREA_MAX) {
          line = DISPLAY_AREA_MAX;
          indx++;
          for (byte i = DISPLAY_AREA_MIN; i <= DISPLAY_AREA_MAX - 1; i++)
            printLine(list[indx + i - DISPLAY_AREA_MAX], 0, i * LINE_HEIGHT , TFT_BLACK);
        }
        else {
          printLine(list[indx++], 0, line++ * LINE_HEIGHT, TFT_BLACK);
        }
        //          printLine(list[indx], 0, line * LINE_HEIGHT, TFT_GREEN);
      }

    }
    else if (bt == 3)
      return indx;

    bt = checkBT_UP();
    if (bt == 2) {
      if ( indx > 0) {
        if ( (line - 1) < DISPLAY_AREA_MIN) {
          line = DISPLAY_AREA_MIN;
          --indx;
          for (byte i = DISPLAY_AREA_MIN + 1; i <= DISPLAY_AREA_MAX; i++)
            printLine(list[indx + i - DISPLAY_AREA_MIN], 0, i * LINE_HEIGHT , TFT_BLACK);
        }
        else {
          printLine(list[indx--], 0, line-- * LINE_HEIGHT, TFT_BLACK);
        }
        //          printLine(list[indx], 0, line * LINE_HEIGHT, TFT_GREEN);
      }
    }
    delay(30);
    printRotateLine(list[indx], 0, line * LINE_HEIGHT, TFT_GREEN);
  }
}

String getStringMenu(const char* title)
{
  char indx = 0;
  String retStr;
  byte bt;
  tft.fillScreen(TFT_BLACK);
  tft.setCursor (0, 0);
  //print title
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.println(title);

  tft.setCursor (0, 3 * LINE_HEIGHT);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.print('0');
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (char i = '1'; i <= 'z'; i++)
    tft.print(i);

  for (;;) {
    bt = checkBT_UP();
    if (bt == 2) {
      tft.setCursor (0, 3 * LINE_HEIGHT);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      for (char i = '0'; i <= '0' + indx; i++)
        tft.print(i);
      if ('0' + indx < ('z' ))
        indx++;
      else {
        indx = 0;
        tft.setCursor (0, 3 * LINE_HEIGHT);
      }
      tft.setTextColor(TFT_WHITE, TFT_GREEN);
      tft.print((char)('0' + indx));

    } else if ( bt == 3) { //long press
      // display string
      retStr.concat((char)( '0' + indx));
      tft.setCursor (0, LINE_HEIGHT);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.print(retStr);
    }

    bt = checkBT_DW();
    if (bt == 2) {
      tft.setCursor (0, 3 * LINE_HEIGHT);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      if (indx > 0) {
        for (char i = '0'; i <= '0' + indx - 2; i++)
          tft.print(i);
        indx--;
      }
      else {
        indx = 'z' - '0';
        for (char i = '0'; i <= 'y'; i++)
          tft.print(i);
      }
      tft.setTextColor(TFT_WHITE, TFT_GREEN);
      tft.print((char)('0' + indx));
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      if (indx + '0' != 'z')
        tft.print((char)('0' + indx + 1));
    } else if ( bt == 3)
      return retStr;
  }
  delay(100);
}




byte checkBT_DW()
{
  //ret 1 - pressed ,2 released , 3- long press
  if ( (digitalRead(BTN_DW) == LOW) && (pressedBT_DW == false) ) {
    pressedBT_DW = true;
    pressTime = millis();
    return 1;

  } else if ( (digitalRead(BTN_DW) == HIGH)  && (pressedBT_DW == true) ) {
    pressedBT_DW = false;
    return 2;
  }
  else if ((digitalRead(BTN_DW) == LOW) && (pressedBT_DW == true) && (( millis() - pressTime) > LONG_PRESS)) {
    pressedBT_DW = false;
    return 3;
  }
}

byte checkBT_UP()
{
  //ret 1 - pressed ,2 released , 3- long press
  if ( (digitalRead(BTN_UP) == LOW) && (pressedBT_UP == false) ) {
    pressedBT_UP = true;
    pressTime = millis();
    return 1;

  } else if ( (digitalRead(BTN_UP) == HIGH)  && (pressedBT_UP == true) ) {
    pressedBT_UP = false;
    return 2;
  }
  else if ((digitalRead(BTN_UP) == LOW) && (pressedBT_UP == true) && (( millis() - pressTime) > LONG_PRESS)) {
    pressedBT_UP = false;
    return 3;
  }
}



int16_t printLine(const String& txt, const int16_t x, const int16_t y, const uint16_t bg)
{
  tft.setTextColor(TFT_WHITE, bg);
  if (tft.textWidth(txt) + x  > tft.width()) {
    tft.drawString(txt.substring(0, (tft.width() - x) / tft.textWidth(".") - 3), x, y);
    tft.drawString("...", tft.width() - tft.textWidth("..."), y);
    return tft.width();
  }
  else
    return x + tft.drawString(txt, x, y);
}

int16_t printRotateLine(const String& txt, const int16_t x, int16_t y, const uint16_t bg)
{
    int16_t txtW = tft.textWidth(txt);
    static int16_t coly =0;
    static bool shiftL = false;
    tft.setTextColor(TFT_WHITE, bg);
    if ((x+txtW) > DISPLAY_WIDTH) {
      tft.drawString(txt, coly+x, y);
      if (shiftL) {
        if ( coly >= -( txtW - DISPLAY_WIDTH+x +10))
          coly--;
        else
          shiftL = false;
      }
      else {

        if (coly < 10)
          coly++;
        else
          shiftL = true;
      }
      return DISPLAY_WIDTH;
    }
    else
      return x+tft.drawString(txt, x, y);
}
