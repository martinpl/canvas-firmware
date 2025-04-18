#include <settings.h>
#include <SPI.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "GDEP073E01/GDEP073E01.cpp"
#include "Adafruit_MAX1704X.h"

HTTPClient https;
WiFiClientSecure client;
StaticJsonDocument<200> fetchJson();
uint8_t buffer[16384];
Adafruit_MAX17048 maxlipo;

void setup()
{
   baseSetup();
   connectWifi();
   StaticJsonDocument<200> json = fetchJson();
   fetchAndDisplay(json["img"].as<String>());
   sleep(json["refresh"].as<int>());
}

StaticJsonDocument<200> fetchJson()
{
   StaticJsonDocument<200> json;
   https.begin(client, ENDPOINT);
   https.addHeader("Content-Type", "application/json");
   int httpCode = https.POST(batteryStats());
   if (httpCode != HTTP_CODE_OK)
   {
      error("[HTTP] Json failed " + String(httpCode) + " " + https.errorToString(httpCode).c_str(), "http");
   }
   deserializeJson(json, https.getString());
   https.end();

   return json;
}

void fetchAndDisplay(String image)
{
   https.begin(client, image);
   int httpCode = https.GET();
   int size = https.getSize();

   if (httpCode != HTTP_CODE_OK)
      error("[HTTP] Image failed " + String(httpCode) + " " + https.errorToString(httpCode).c_str(), "http");

   if (size != 384000)
      error("[CONTENT] Bad file size " + String(size), "content");

   WiFiClient *stream = https.getStreamPtr();
   int bytesRead = 0;
   int lastByte = -1;
   int firstOrLast;

   while (https.connected() && size != 0)
   {
      if (stream->available())
      {
         firstOrLast = -1;
         bytesRead = stream->readBytes(buffer, 16384);

         if (bytesRead % 2 != 0) // If the number of bytes is odd, save the last byte for the next iteration
         {
            lastByte = buffer[bytesRead - 1];
            bytesRead--;
         }

         if (size == 384000)
            firstOrLast = 1;

         if (size <= bytesRead)
            firstOrLast = 9;

         displayInChunks(firstOrLast, bytesRead, buffer);

         if (lastByte != -1) // If there's a pending last byte, add it to the buffer in the next iteration
         {
            buffer[0] = lastByte;
            lastByte = -1;
            bytesRead++;
         }

         size -= bytesRead;
      }
   }
   https.end();
}

void sleep(int minutes)
{
   Serial.printf("Sleeping for %d minutes \n", minutes);
   esp_sleep_enable_timer_wakeup(minutes * 60 * 1000000ULL);
   esp_deep_sleep_start();
}

void connectWifi()
{
   WiFiMulti WiFiMulti;
   WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

   int attempts = 0;
   while (WiFiMulti.run() != WL_CONNECTED && attempts < 10)
   {
      Serial.println("[WIFI] Not connected, retrying...");
      delay(1000);
      attempts++;
   }

   if (WiFiMulti.run() != WL_CONNECTED)
   {
      error("[WIFI] Failed to connect after 10 attempts.", "wifi");
   }
}

String batteryStats()
{
   StaticJsonDocument<200> json;
   String serializedJson;
   if (!maxlipo.begin())
   {
      Serial.println("Could not find MAX17048");
   }

   delay(50); // for getting proper battery values
   json["batteryVoltage"] = maxlipo.cellVoltage();
   json["batteryPercent"] = maxlipo.cellPercent();
   serializeJson(json, serializedJson);

   return serializedJson;
}

void baseSetup()
{
   pinMode(PIN_BUSY, INPUT);
   pinMode(PIN_RES, OUTPUT);
   pinMode(PIN_DC, OUTPUT);
   pinMode(PIN_CS, OUTPUT);
   // SPI
   SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
   SPI.begin();
   //
   Serial.begin(115200);
   client.setInsecure(); // Disable SSL certificate verification
}

void error(String message, String type)
{
   Serial.println("[ERROR]" + message);

   EPD_init();

   if (type == "wifi")
   {
      EPD_Display_blue();
   }

   if (type == "http")
   {
      EPD_Display_red();
   }

   if (type == "content")
   {
      EPD_Display_Yellow();
   }

   EPD_sleep();
   esp_deep_sleep_start();
}

void loop()
{
}