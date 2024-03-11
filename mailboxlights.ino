// SPDX-FileCopyrightText: 2022 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_NeoPixel.h>

RTC_DATA_ATTR int lastPhoto;
RTC_DATA_ATTR int lastBatt;
RTC_DATA_ATTR int state; 

#define STATE_OFF 0
#define STATE_ON  1

// WiFi network name and password:
const char * networkName = "YourNetWorkName";
const char * networkPswd = "YourPassword";

// const char * udpAddress = "192.168.86.51";
// const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
//WiFiUDP udp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, -3600 * 8);

#define ENABLE_SLEEP

#define COLOR 0xFFFF00
#define BRIGHTNESS 30

#define DARK_THRESHOLD 500
#define VBAT_THRESHOLD 3800
#define SLEEP_SEC      30*60 //every half hour

#define PIN_NEOPIXEL  26
#define PIN_BATTERY   35
#define PHOTO_PIN     33 

#if defined(PIN_NEOPIXEL)
  Adafruit_NeoPixel pixel(9, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
#endif

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_BATTERY, INPUT);
//  pinMode(PHOTO_PIN, INPUT_PULLUP);
  analogReadResolution(12);

  connectToWiFi(networkName, networkPswd);
  timeClient.begin();
}

void loop() {
  int vbat, vphoto, hour;
  char str[128];
  delay(5000); //Wait a bit so that hopefully WiFi connects
  timeClient.update();
  hour = timeClient.getHours();
//  Serial.println(hour);

  vbat = analogReadMilliVolts(PIN_BATTERY) * 1.95;
//  vphoto = analogReadMilliVolts(PHOTO_PIN);
//  Serial.println(vbat);
// Serial.println(vphoto);
  // sprintf(str, "Batt: %d LastBatt: %d Photo: %d LastPhoto: %d State: %d", vbat, lastBatt, vphoto, lastPhoto, state);
  // Serial.println(str);

  // if (connected)
  // {
  //   //Send a packet
  //   udp.beginPacket(udpAddress,udpPort);
  //   udp.write((unsigned char*) str, strlen(str));
  //   udp.endPacket();
  //   sleep(1); //Give it time to send the packet
  //   Serial.println("Sent Packet");
  // }

  switch (state)
  {
    case STATE_OFF:
      if ((hour > 19) && (vbat > VBAT_THRESHOLD))
      {
        Serial.println("Turning On LEDs");
        LEDon();
        state = STATE_ON;
      }
    break;

    case STATE_ON:
      if ((hour < 19) || (vbat < VBAT_THRESHOLD))
      {
        Serial.println("Turning Off LEDs");
        state = STATE_OFF;
        LEDoff();
      }
    break;
  }

  lastBatt = vbat;
//  lastPhoto = vphoto;

  // wake up 1 second later and then go into deep sleep
#ifdef ENABLE_SLEEP  
  esp_sleep_enable_timer_wakeup(SLEEP_SEC * 1000000); //us 
  esp_deep_sleep_start(); 
#endif  
  // we never reach here
}

int dark(int val)
{
  if (val > DARK_THRESHOLD)
    return 1;
  else 
    return 0;
}

void LEDon() {
  pixel.begin(); // INITIALIZE NeoPixel
  pixel.setBrightness(BRIGHTNESS); // not so bright
  pixel.setPixelColor(0, COLOR);
  pixel.setPixelColor(1, COLOR);
  pixel.setPixelColor(2, COLOR);
  pixel.setPixelColor(3, COLOR);
  pixel.setPixelColor(4, COLOR);
  pixel.setPixelColor(5, COLOR);
  pixel.setPixelColor(6, COLOR);
  pixel.setPixelColor(7, COLOR);
  pixel.setPixelColor(8, COLOR);
  pixel.show();
}

void LEDoff() {
  pixel.setPixelColor(0, 0x0);
  pixel.setPixelColor(1, 0x0);
  pixel.setPixelColor(2, 0x0);
  pixel.setPixelColor(3, 0x0);
  pixel.setPixelColor(4, 0x0);
  pixel.setPixelColor(5, 0x0);
  pixel.setPixelColor(6, 0x0);
  pixel.setPixelColor(7, 0x0);
  pixel.setPixelColor(8, 0x0);
  pixel.show();
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
//          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}
