#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "battery.h"
#include <EEPROM.h>
// Zone reference "euCET" Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule  CET = {"CET ", Last, Sun, Oct, 3, 60};      //Central European Standard Time
Timezone euCET(CEST, CET);
const char* ntpServerName = "time.google.com";
#define eepromAddrSSID 5
#define eepromAddrPASS 35

#define TIMEZONE euCET


WiFiUDP Udp;
String ssid;
String pass;
unsigned int localPort = 8888;  // local port to listen for UDP packets

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
#include "menu.h"

byte omm = 99;
bool initial = 1;
byte xcolon = 0;
bool networkConn = false;



time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
bool WiFiConn();

void setup(void) {
  EEPROM.begin(512);
  Serial.begin(115200);
  Serial.println("Booting...");
  ssid = EEPROM.readString(eepromAddrSSID);
  pass = EEPROM.readString(eepromAddrPASS);

  Serial.println(__TIME__);
  Serial.println(__DATE__);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK); // Note: the new fonts do not draw the background colour
  tft.setTextSize(2);
  tft.setTextPadding(20);

  pinMode(34, INPUT);
  pinMode(14, OUTPUT);
  pinMode(35, INPUT_PULLUP);
  if (!WiFiConn()) {
    menu();
    WiFiConn();
  }
  delay(500);
  Serial.println("Starting UDP");
  Serial.println("waiting for sync");
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval( 24*3600); //24*1h
}


time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay(TIMEZONE.toLocal(now()));
    }
  }
  delay(100);
  if (digitalRead(0) == LOW)
    menu();
}


byte printDigits(byte digits, byte xpos, const byte ypos)
{
  // utility for digital clock display: prints preceding colon and leading 0
  if (digits < 10)
    xpos += tft.drawChar('0', xpos, ypos, 6);
  xpos += tft.drawNumber(digits, xpos, ypos, 6);
  return xpos;
}

void digitalClockDisplay(time_t timeNow)
{
  byte xpos = 0;
  byte ypos = 0;
  byte m = minute(timeNow);
  if (omm != m) { // Only redraw every minute to minimise flicker
    //    tft.fillScreen(TFT_BLACK);
    tft.setCursor (8, 100);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(__DATE__); // This uses the standard ADAFruit small font
    tft.setTextColor(TFT_RED, TFT_BLACK);

    // Uncomment ONE of the next 2 lines, using the ghost image demonstrates text overlay as time is drawn over it
    //tft.setTextColor(0x39C4, TFT_BLACK);  // Leave a 7 segment ghost image, comment out next line!
    //      tft.setTextColor(TFT_BLACK, TFT_BLACK); // Set font colour to black to wipe image
    // Font 7 is to show a pseudo 7 segment display.
    // Font 7 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .
    //      tft.drawString("88:88",xpos,ypos,6); // Overwrite the text to clear it
    //      tft.setTextColor(TFT_RED); // Orange
    omm = m;
    xpos += printDigits(hour(timeNow), xpos, ypos);
    xcolon = xpos;
    xpos += tft.drawChar(':', xpos , ypos, 6);
    printDigits(m, xpos, ypos);
  }

  if (second(timeNow) % 2) { // Flash the colon
    digitalWrite(14, HIGH);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    xpos += tft.drawChar(':', xcolon, ypos, 6);
    tft.setTextColor(0xBDF7, TFT_BLACK);
    tft.setCursor (170, 100);
    tft.print(WiFi.RSSI());
    tft.print("dB");
    drawBattery(170, 120);

    digitalWrite(14, LOW);


  }
  else {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawChar(':', xcolon, ypos, 6);
  }
}

void drawBattery(const byte x, const byte y)
{
  float val = ((float)analogRead(34) * 7.26) / 4095.0;
  //  tft.setCursor(x+15,y);
  byte xofs = x + 15;
  xofs += tft.drawFloat(val, 2, xofs, y);
  tft.drawChar('V', xofs, y);
  if (val > 4.4)
    tft.drawXBitmap(x, y, battery5_bits, battery5_width, battery5_height, TFT_BLACK, TFT_MAGENTA);
  else if (val > 4.0)
    tft.drawXBitmap(x, y, battery4_bits, battery4_width, battery4_height, TFT_BLACK, TFT_GREEN);
  else if (val > 3.7)
    tft.drawXBitmap(x, y, battery3_bits, battery3_width, battery3_height, TFT_BLACK, TFT_LIGHTGREY);
  else if (val > 3.4)
    tft.drawXBitmap(x, y, battery2_bits, battery2_width, battery2_height, TFT_BLACK, TFT_LIGHTGREY);
  else if (val > 3.1)
    tft.drawXBitmap(x, y, battery1_bits, battery1_width, battery1_height, TFT_BLACK, TFT_ORANGE);
  else if (val > 2.9)
    tft.drawXBitmap(x, y, battery0_bits, battery0_width, battery0_height, TFT_BLACK, TFT_RED);

}



void printUTC(time_t utc)
{
  // Print the hour, minute and second:
  Serial.print(hour(utc)); // print the hour (86400 equals secs per day)
  Serial.print(':');
  uint8_t m = minute(utc);
  if (m < 10 ) Serial.print('0');
  Serial.print(m); // print the minute (3600 equals secs per minute)
  Serial.print(':');
  uint8_t s = second(utc);
  if ( s < 10 ) Serial.print('0');
  Serial.println(s); // print the second
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address
  time_t utc ;//= now();
  
    if (WiFi.status() != WL_CONNECTED)
      WiFiConn();
    if (WiFi.status() != WL_CONNECTED)
      return utc;
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      utc = secsSince1900 - 2208988800UL;
      Serial.print("Received NTP UTC time : ");
      printUTC(utc);
      Serial.print("Received NTP Local time : ");
      printUTC(TIMEZONE.toLocal(utc));
      return utc;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress & address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


bool WiFiConn()
{
  byte cntTry = 0;
  Serial.print("Connecting to ");
  Serial.println(ssid);
  tft.println("Connecting to:");
  tft.println(ssid);
//  tft.println(pass);
  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED && (cntTry < 20) ) {
    delay(500);
    Serial.print(".");
    tft.print('.');
    cntTry++;
  }
  tft.println(cntTry);
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  return (cntTry < 20);
}
