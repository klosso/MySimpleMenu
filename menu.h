#include <WiFi.h>
#include<EEPROM.h>

#define BTN_1 35
#define BTN_2 0
#define MAX_LINES 15
int LINE_HEIGHT = 12;
byte FONT_LINE_WIDTH = tft.width() / tft.textWidth(".", 1);

#define LONG_PRESS 500
#define eepromSSID 5
#define eepromPASS 35

#ifndef eepromTimeZ
#define eepromTimeZ 60
#endif
#ifndef eepromAddrRotation
#define eepromAddrRotation 65
#endif

long pressTime;
bool pressedBT_DW = false;
bool pressedBT_UP = false;
extern byte btn_up;
extern byte btn_dw;
int16_t DISPLAY_WIDTH = tft.width();

int menuList(const char* title, const String list[]);
String getStringMenu(const char* title);
int getIntVal(const String& title, const String& valName, const int min, const int max, const int initVal, const String& units);

byte checkBT_DW();
byte checkBT_UP();

void checkTimeZone(int in);



// Zone reference "euCET" Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule  CET = {"CET ", Last, Sun, Oct, 3, 60};      //Central European Standard Time
TimeChangeRule  UTC = {"UTC ", First, Sun, Jan, 1, 0};      //Universal Standard Time

// Zone reference "UK" United Kingdom (London, Belfast)
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        //British Summer (Daylight saving) Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         //Standard Time
Timezone UK(BST, GMT);

// Zone reference "ausET" Australia Eastern Time Zone (Sydney, Melbourne)
TimeChangeRule aEDT = {"AEDT", First, Sun, Oct, 2, 660};    //UTC + 11 hours
TimeChangeRule aEST = {"AEST", First, Sun, Apr, 3, 600};    //UTC + 10 hours
// Zone reference "usET US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //Eastern Standard Time = UTC - 5 hours
// Zone reference "usCT" US Central Time Zone (Chicago, Houston)
TimeChangeRule usCDT = {"CDT", Second, dowSunday, Mar, 2, -300};
TimeChangeRule usCST = {"CST", First, dowSunday, Nov, 2, -360};
// Zone reference "usMT" US Mountain Time Zone (Denver, Salt Lake City)
// Zone reference "usAZ" Arizona is US Mountain Time Zone but does not use DST
TimeChangeRule usMDT = {"MDT", Second, dowSunday, Mar, 2, -360};
TimeChangeRule usMST = {"MST", First, dowSunday, Nov, 2, -420};
// Zone reference "usPT" US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule usPDT = {"PDT", Second, dowSunday, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, dowSunday, Nov, 2, -480};
// Zone reference "eufET US EEST
TimeChangeRule EEDT = {"EEDT", Last, Sun, Mar, 2, 180};  //Eastern Daylight Time = UTC - 4 hours
TimeChangeRule EEST = {"EEST", Last, Sun, Oct, 3, 120};   //Eastern Standard Time = UTC - 5 hours

TimeChangeRule UTC_1 = {"UTC+1", First, Sun, Jan, 1, 60}; //UTC+1
TimeChangeRule UTC_2 = {"UTC+2", First, Sun, Jan, 1, 120}; //UTC+2
TimeChangeRule UTC_3 = {"UTC+3", First, Sun, Jan, 1, 180}; //UTC+3
TimeChangeRule UTC_4 = {"UTC+4", First, Sun, Jan, 1, 240}; //UTC+4
TimeChangeRule UTC_5 = {"UTC+5", First, Sun, Jan, 1, 300}; //UTC+5
TimeChangeRule UTC_6 = {"UTC+6", First, Sun, Jan, 1, 360}; //UTC+6
TimeChangeRule UTC_7 = {"UTC+7", First, Sun, Jan, 1, 420}; //UTC+7
TimeChangeRule UTC_8 = {"UTC+8", First, Sun, Jan, 1, 480}; //UTC+8
TimeChangeRule UTC_9 = {"UTC+9", First, Sun, Jan, 1, 540}; //UTC+9
TimeChangeRule UTC_10 = {"UTC10", First, Sun, Jan, 1, 600}; //UTC+10
TimeChangeRule UTC_11 = {"UTC11", First, Sun, Jan, 1, 660}; //UTC+11
TimeChangeRule UTC_12 = {"UTC12", First, Sun, Jan, 1, 720}; //UTC+12


void menuWiFi();
void menuRotation();
void menuSave();
void menuTimeZone();

void menu()
{
  int item = 0;
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  String menuItems[] = {"WiFi", "Set timezone", "Set rotation", "Save setting", "Exit", ""};
  do {
    item = menuList("Setup:", menuItems);
    switch (item) {
      case 0:
        menuWiFi();
        break;
      case 1:
        menuTimeZone();
        break;
      case 2:
        menuRotation();
        break;
      case 3:
        menuSave();
        break;
    }
  } while (item != 4  );
  tft.fillScreen(TFT_BLACK);
}


void setBtnAcordingrotation(int in)
{
  const int MenuLeftTop =2;
  if (in < MenuLeftTop ) {
    btn_up = BTN_1;
    btn_dw = BTN_2;
  }
  else {
    btn_up = BTN_2;
    btn_dw = BTN_1;
  }
}

void menuRotation()
{
  String menuItems[] = {"left", "top", "right", "bottom", ""};
  int item = menuList("Rotation:", menuItems);
  tft.setRotation(item);
  setBtnAcordingrotation(item);
  EEPROM.writeInt(eepromAddrRotation, item);
  EEPROM.commit();
  delay(1000);
}


void menuWiFi()
{
  String menuItems[] = {"WiFi Network", "WiFi password", "Save WiFi settings", "Set some shity variable", "...back", ""};
  String WiFiNetworks[30];
  int item = 0;
  int i, n, ret = 0;
  do {
    item = menuList("WiFi Setup:", menuItems);
    switch (item)
    {
      case 0:
        tft.fillScreen(TFT_BLACK);
        tft.drawString("Scanning WiFi:...", 0, 0);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        n = WiFi.scanNetworks();
        for (i = 0; i < n; i++)
          WiFiNetworks[i] = WiFi.SSID(i).c_str();
        WiFiNetworks[++i] = String();

        ret = menuList("WiFi Netrorks:", WiFiNetworks);
        ssid = menuItems[ret];
        break;
      case 1:
        pass = getStringMenu("Set password");
        break;
      case 2:
        menuSave();
        break;
      case 3:
        getIntVal("setup variable ", "Shit", -10, 30, -2, "dBm");
        break;
    }
  } while ( item < 4);
}

void menuSave()
{
  EEPROM.writeString(eepromPASS, pass);
  EEPROM.writeString(eepromSSID, ssid);
  EEPROM.commit();
  delay(1000);

}

void menuTimeZone()
{
  String menuItemsZone[30] = {"UTC", "Eur/Warsaw", "US/TX", "+1", "+2", "EEST +3h/DST(Finland,/Syria/Cyprus)", "+4", "+5", "+6", "+7", "+8", "+9", "+10", "+11", "+12", "-12", "-11", "-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3h", "-2h", "-1h"};
  int tz = menuList("Time zone:", menuItemsZone);
  checkTimeZone(tz);
  EEPROM.writeInt(eepromTimeZ, tz);
  EEPROM.commit();
  delay(1000);
}




void checkTimeZone(int in)
{

  switch (in)
  {
    case 0:
      TIMEZONE.setRules(UTC, UTC);
      break;
    default:
    case 1:
      TIMEZONE.setRules(CEST, CET);
      break;
    case 2:
      TIMEZONE.setRules(UTC_1, UTC_1);
      break;
    case 3:
      TIMEZONE.setRules(UTC_2, UTC_2);
      break;
    case 4:
      TIMEZONE.setRules(UTC_3, UTC_3);
      break;
    case 5:
      TIMEZONE.setRules(UTC_4, UTC_4);
      break;
    case 6:
      TIMEZONE.setRules(UTC_5, UTC_5);
      break;
    case 7:
      TIMEZONE.setRules(UTC_6, UTC_6);
      break;
    case 8:
      TIMEZONE.setRules(UTC_7, UTC_7);
      break;
    case 9:
      TIMEZONE.setRules(UTC_8, UTC_8);
      break;
    case 10:
      TIMEZONE.setRules(UTC_9, UTC_9);
      break;
    case 11:
      TIMEZONE.setRules(UTC_10, UTC_10);
      break;
    case 12:
      TIMEZONE.setRules(UTC_11, UTC_11);
      break;
    case 13:
      TIMEZONE.setRules(UTC_12, UTC_12);
      break;

      //      TIMEZONE.writeRules(eepromTimeZ);
  }
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
      }
    }
    delay(30);
    printRotateLine(list[indx], 0, line * LINE_HEIGHT, TFT_GREEN);
  }
}
int getIntVal(const String& title, const String& valName, const int min, const int max, const int initVal, const String& units)
{
  int val = initVal;
  byte bt;
  tft.fillScreen(TFT_BLACK);
  tft.setCursor (0, 0);
  //print title
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.println(title);

  tft.setCursor (0, 3 * LINE_HEIGHT);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (;;)
  {
    bt = checkBT_UP();
    if (bt == 2 && val < max)
      val ++;
    bt = checkBT_DW();
    if (bt == 2 && val > min)
      val --;
    else if ( bt == 3)
      return val;
    printLine(valName + " : " + val + units, 0, 3 * LINE_HEIGHT, TFT_BLACK);
    delay(100);
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
  if ( (digitalRead(btn_dw) == LOW) && (pressedBT_DW == false) ) {
    pressedBT_DW = true;
    pressTime = millis();
    return 1;

  } else if ( (digitalRead(btn_dw) == HIGH)  && (pressedBT_DW == true) ) {
    pressedBT_DW = false;
    return 2;
  }
  else if ((digitalRead(btn_dw) == LOW) && (pressedBT_DW == true) && (( millis() - pressTime) > LONG_PRESS)) {
    pressedBT_DW = false;
    return 3;
  }
}

byte checkBT_UP()
{
  //ret 1 - pressed ,2 released , 3- long press
  if ( (digitalRead(btn_up) == LOW) && (pressedBT_UP == false) ) {
    pressedBT_UP = true;
    pressTime = millis();
    return 1;

  } else if ( (digitalRead(btn_up) == HIGH)  && (pressedBT_UP == true) ) {
    pressedBT_UP = false;
    return 2;
  }
  else if ((digitalRead(btn_up) == LOW) && (pressedBT_UP == true) && (( millis() - pressTime) > LONG_PRESS)) {
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
  static int16_t coly = 0;
  static bool shiftL = false;
  tft.setTextColor(TFT_WHITE, bg);
  int16_t DISPLAY_WIDTH = tft.width();
  if ((x + txtW) > DISPLAY_WIDTH) {
    tft.drawString(txt, coly + x, y);
    if (shiftL) {
      if ( coly >= -( txtW - DISPLAY_WIDTH + x + 10))
        coly--;
      else
        shiftL = false;
    }
    else {

      if (coly < 0)
        coly++;
      else
        shiftL = true;
    }
    return DISPLAY_WIDTH;
  }
  else
    return x + tft.drawString(txt, x, y);
}
