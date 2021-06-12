/***************************************************************************
  This is a library for the CCS811 air

  This sketch reads the sensor

  Designed specifically to work with the Adafruit CCS811 breakout
  ----> http://www.adafruit.com/products/3566

  These sensors use I2C to communicate. The device's I2C address is 0x5A

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#include "Adafruit_CCS811.h"
Adafruit_CCS811 ccs;

/**************************************************************************************

  This is example for ClosedCube HDC1080 Humidity and Temperature Sensor breakout booard

  Initial Date: 13-May-2016

  Hardware connections for Arduino Uno:
  VDD to 3.3V DC
  SCL to A5
  SDA to A4
  GND to common ground

  Written by AA for ClosedCube

  MIT License

**************************************************************************************/
#include <Wire.h>
#include "ClosedCube_HDC1080.h"

ClosedCube_HDC1080 hdc1080;

/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMEP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bme; // I2C

// JSON reader for OpenWeatherMap optimized for NodeMCU ESP 12E
// by Michiel van Hoorn - 2016 - http://stuffingpi.azurewebsites.net/


// MIT License
// Copyright (c) 2016 Michiel van Hoorn
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Refer to http://openweathermap.org/api for API information and details

// Using Arduino JSON library by Benoit Blanchon
// https://github.com/bblanchon/ArduinoJson


#include <ArduinoJson.h>
#include <WifiLocation.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <EEPROM.h>

const char* owmServer = "api.openweathermap.org";  // owmServer's address
const char* resource1 = "/data/2.5/weather?lat=";
const char* resource2 = "&lon=";
const char* resource3 = "&units=metric&APPID=68d53c4027df7d99745d08109dbd30ba"; // insert your API key

char response[600]; // this fixed sized buffers works well for this project using the NodeMCU.
WiFiClient client;

/**********************************************
   WiFi GeoLocation
*/
#ifdef ARDUINO_ARCH_SAMD
#include <WiFi101.h>
#elif defined ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#error Wrong platform
#endif


ESP8266WebServer webServer(80);

const char* googleApiKey = "AIzaSyBvkSJNtL07ov-Pf4_hfHSfLUye4qhQjDc";
//const char* ssid = "JamiroWiFi";
//const char* passwd = "cicciopallino14";
const char* ssid = "CASAMATTA";
const char* passwd = "poterealpopolo";

WifiLocation location(googleApiKey);

/*

  Wather.ino

  Show weather icons from open-iconic set.
  Plus demo for text scrolling.
  This example requires display height >= 64.

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define U8LOG_WIDTH 32
#define U8LOG_HEIGHT 4
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8G2LOG u8g2log;

/***********************************
   GP2Y10 sensor based on PMsensor library
   https://github.com/ekkai/PMsensor
*/
#include <PMsensor.h>
PMsensor PM;

/********************************
   my code
*/
double lat = 0;
double lon = 0;

#define INTERVAL        60000
#define TIMETORESET     100
#define SAMPLE_LIMIT    1000
long countSamples = 0;

double slPressure;
const char* town;
#define CCS_WAKE        12     // SDD2 (9), prima era 12 (D6 su nodeMcu)
#define CCS_RESET       14    // SDD3 (10), prima era 14 (D5 on nodeMcu)
#define GPY_LED         16     // D3 (0), prima era 16 (D0 on nodeMcu)
#define GPY_SENSOR      A0    // A0 on nodeMcu

// Thingspeak
//const char* tsHost = "184.106.153.149";
const char* tsHost = "api.thingspeak.com";
const char* tsHostName = "api.thingspeak.com";
String privateKey = "YGXES1DISTOMQ00N";

boolean credentials;

// *** Additional functions
void sendToCloud(bool error, float temp, long pressure, float pmData, float humidity,
                 long CO2, long TVOC);
void getLocation();
void printSerialNumber();
double getSeaLevelPressure(double lat, double lon);
void setupWifi();
void calibrateCCS811();
void writeOledSetupMsg(char *s1, char * s2);
void writeOledData(bool error, float temp, long pressure, float pmData, float humidity,
                   long CO2, long TVOC);
void ccsReset(bool error);
int tryConnecting(String ssid, String pass, int auth);
int save2Eeprom (String str, int first);


/********************************
   let's start doing things
*/
void setup() {
  u8g2.begin();
  u8g2log.begin(U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);

  writeOledSetupMsg("Initialising Serial", "");
  Serial.begin(115200);

  writeOledSetupMsg("Initialising Ports", "");
  pinMode(CCS_WAKE, OUTPUT);
  pinMode(CCS_RESET, OUTPUT);
  pinMode(GPY_LED, OUTPUT);
  digitalWrite(CCS_WAKE, LOW); // Activate CCS811 sensor
  delay(5);
  ccsReset(false);

  Serial.println(" ");
  Serial.println("CCS811 + HDC1080 + BMP280 + Location + OLED test");

  setupWifi();

  getLocation();

  // Init HDC1080
  writeOledSetupMsg("Initialising HDC1080", "");
  hdc1080.begin(0x40);
  writeOledSetupMsg("Initialising HDC1080:", "OK");
  delay(200);

  // Init CCS811
  writeOledSetupMsg("Initialising CCS811", "");
  while ( (!ccs.begin())) {
    writeOledSetupMsg("Init CCS811 failed!", "Check wiring");
    while (1);
  }
  writeOledSetupMsg("Initialising CCS811:", "OK");
  delay(200);
  writeOledSetupMsg("Calibrating CCS811", "");
  calibrateCCS811();
  writeOledSetupMsg("Calibrating CCS811:", "OK");
  delay(200);

  // Init BMP280
  writeOledSetupMsg("Initialising BMP280", "");
  if (!bme.begin()) {
    writeOledSetupMsg("Init BMP280 Failed!", "Check wiring");
    while (1);
  }
  slPressure = getSeaLevelPressure(lat, lon);
  writeOledSetupMsg("Initialising BMP280:", "OK");
  delay(200);

  writeOledSetupMsg("Init GP2Y10 PM sensor", "");
  PM.init(GPY_LED, GPY_SENSOR);
  writeOledSetupMsg("Init GP2Y10 PM sensor:", "OK");
  delay(200);

  writeOledSetupMsg("Init complete", "");
  delay(200);
  u8g2.clear();
}

void loop() {
  // Should we check whether location has changed?
  if ( countSamples >= SAMPLE_LIMIT ) {
    getLocation();
    slPressure = getSeaLevelPressure(lat, lon);
    countSamples = 0;
  }

  // get data from sensors
  float hdcTemp = hdc1080.readTemperature();
  float hdcRH = hdc1080.readHumidity();

  float bmpTemp = bme.readTemperature();
  long bmpPres = (bme.readPressure() / 100);
  float bmpAlt = bme.readAltitude(slPressure); // adjusted to local forcase

  float ccsTemp = -1;
  long ccsCO2 = -1;
  long ccsTVOC = -1;

  float gp2yData = 0;
  int gp2y_err = PMsensorErrSuccess;
  if ((gp2y_err = PM.read(&gp2yData, true, 0.1)) != PMsensorErrSuccess) {
    Serial.print("gp2yData Error = ");
    Serial.println(gp2y_err);
    delay(3000);
    return;
  }

  // get and print
  Serial.print("BMP280  T = ");
  Serial.print(bmpTemp);
  Serial.print(" *C; ");
  Serial.print("P = ");
  Serial.print(bmpPres);
  Serial.print(" hPa; ");
  Serial.print("h = ");
  Serial.print(bmpAlt);
  Serial.println(" m");

  Serial.print("HDC1080 T = ");
  Serial.print(hdcTemp);
  Serial.print(" *C; RH = ");
  Serial.print(hdcRH);
  Serial.println(" %");

  Serial.print("GP2Y10 PM2.5 = ");
  Serial.print(gp2yData);
  Serial.println(" ppm");

  if (ccs.available()) {
    unsigned char ccsError = ccs.readData();
    if (!ccsError) {
      float ccsTemp = ccs.calculateTemperature();
      long ccsCO2 = ccs.geteCO2();
      long ccsTVOC = ccs.getTVOC();
      Serial.print("CCS811  T = ");
      Serial.print(ccsTemp);
      Serial.print(" *C; CO2: ");
      Serial.print(ccsCO2);
      Serial.print(" ppm; TVOC: ");
      Serial.println(ccsTVOC);
      Serial.println();
      writeOledData(ccsError, hdcTemp, bmpPres, gp2yData, hdcRH, ccsCO2, ccsTVOC);
      sendToCloud(ccsError, hdcTemp, bmpPres, gp2yData, hdcRH, ccsCO2, ccsTVOC);

    }
    else {
      Serial.print("CCS811 ERROR no. ");
      Serial.println(ccsError);
      Serial.println("Resetting...");
      Serial.println();
      ccsReset(true);
      writeOledData(ccsError, hdcTemp, bmpPres, gp2yData, hdcRH, ccsCO2, ccsTVOC);
      sendToCloud(ccsError, hdcTemp, bmpPres, gp2yData, hdcRH, ccsCO2, ccsTVOC);
    }
  }
  Serial.println("");
  countSamples++;
  delay(INTERVAL);
}

void printSerialNumber() {
  Serial.print("Device Serial Number=");
  HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
  char format[12];
  sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
  Serial.println(format);
}

double getSeaLevelPressure(double lat, double lon) {
  double slPressure;
  // connect to owmServer
  Serial.print("Connecting to ");
  Serial.println(owmServer);
  bool ok = client.connect(owmServer, 80);
  bool beginFound = false;

  while (!ok) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected to ");
  Serial.println(owmServer);

  delay(500);

  Serial.println("Sending request...");
  //Send request to resource
  client.print("GET ");
  client.print(resource1);
  client.print(lat);
  client.print(resource2);
  client.print(lon);
  client.print(resource3);

  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(owmServer);
  client.println("Connection: close");
  client.println();

  delay(100);

  //Reading stream and remove headers
  client.setTimeout(10000);

  bool ok_header = client.find("\r\n\r\n");

  while (!ok_header) {
    // wait
  }

  client.readBytes(response, 800);

  // uncomment line below to see data received for debugging purposes or just fun
  Serial.println(response);

  // process JSON
  DynamicJsonDocument jsonBuffer;
  DeserializationError error = deserializeJson(jsonBuffer, response);
  if (error) {
    Serial.println("There was an error while deserializing");
  } else {
    // But.....make sure the stream header is valid
    // Sometime OWM includes invalid data after the header
    // Parsing fails if this data is not removed

    Serial.println("Response received");

    JsonObject root = jsonBuffer.as<JsonObject>();

    town = root["name"];
    const char* shortWeather = root["weather"][0]["main"];
    const char* weather = root["weather"][0]["description"];
    double temp = root["main"]["temp"];
    int pressure = root["main"]["pressure"];
    slPressure = pressure;
    int humidity = root["main"]["humidity"];

    // Print data to Serial
    Serial.print("*** ");
    Serial.print(town);
    Serial.println(" ***");
    Serial.print("Type: ");
    Serial.println(shortWeather);
    Serial.print(" (");
    Serial.print(weather);
    Serial.println(")");
    Serial.print("Temp ");
    Serial.print(temp);
    Serial.println("C");
    Serial.print("pressure ");
    Serial.print(pressure);
    Serial.println(" bar");
    Serial.print("humidity ");
    Serial.print(humidity);
    Serial.println("%");
    Serial.println("----------");

    client.stop(); // disconnect from owmServer

    Serial.println("");
  }
  return slPressure;
}

void setupWifi() {
  writeOledSetupMsg("Connecting to WiFi:", const_cast<char*>(ssid));
  // Connect to WPA/WPA2 network
#ifdef ARDUINO_ARCH_ESP32
  WiFi.mode(WIFI_MODE_STA);
#endif
#ifdef ARDUINO_ARCH_ESP8266
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // wait 5 seconds for connection:
    Serial.print("Status = ");
    Serial.println(WiFi.status());
    delay(500);
  }
}

void calibrateCCS811() {
  //calibrate CCS811
  while (!ccs.available());
  double temp = hdc1080.readTemperature();
  uint8_t rh = (uint8_t)hdc1080.readHumidity();
  float tccs = ccs.calculateTemperature();

  Serial.print("HDC1080 T = ");
  Serial.print(temp);
  Serial.print(" *C; RH = ");
  Serial.print(rh);
  Serial.println(" %");
  Serial.print("CCS811  T = ");
  Serial.print(tccs);
  Serial.println(" *C");
  ccs.setEnvironmentalData(rh, temp);
  ccs.setTempOffset(temp - 25.0);
  Serial.println("CCS811 Calibrated");
}

void writeOledSetupMsg(char *s1, char * s2) {
  Serial.print (s1);
  Serial.print (" ");
  Serial.println(s2);
  Serial.println("");

  u8g2log.print(s1);
  u8g2log.print(" ");
  u8g2log.print(s2);
  u8g2log.print("\n");

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_crox5h_tf);
    u8g2.drawStr(10, 17, "Air Quality");
    u8g2.drawStr(22, 35, "Monitor");
    u8g2.setFont(u8g2_font_4x6_tr);   // set the font for the terminal window
    u8g2.drawLog(0, 42, u8g2log);   // draw the terminal window on the display
  } while ( u8g2.nextPage() );
}

void writeOledData(bool error, float temp, long pressure, float pmData, float humidity,
                   long CO2, long TVOC) {
  if (error) {
    u8g2.firstPage();
    do {
      writeOledSetupMsg("ERROR!", "Resetting");
    } while ( u8g2.nextPage() );
  } else {
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_crox2h_tf);
      u8g2.drawStr(0, 12, "Air Quality Monitor");
      u8g2.setFont(u8g2_font_crox1h_tf);
      u8g2.setCursor(0, 24);
      u8g2.print(town);
      u8g2.print(" (");
      u8g2.print(lat);
      u8g2.print(";");
      u8g2.print(lon);
      u8g2.print(")");
      u8g2.setCursor(0, 39);
      u8g2.print("T: ");
      u8g2.print(temp, 1);
      u8g2.print((char)176);
      u8g2.print("C");
      u8g2.setCursor(0, 51);
      u8g2.print("P: ");
      u8g2.print(pressure);
      u8g2.print("hPa");
      u8g2.setCursor(0, 63);
      u8g2.print("PM2.5: ");
      u8g2.print(pmData, 1);
      //      u8g2.print("m");
      u8g2.setCursor (62, 39);
      u8g2.print("RH: ");
      u8g2.print(humidity, 1);
      u8g2.print("%");
      u8g2.setCursor (62, 51);
      u8g2.print("CO2:");
      u8g2.print(CO2);
      u8g2.print("ppm");
      u8g2.setCursor (62, 63);
      u8g2.print("TVOC: ");
      u8g2.print(TVOC);
      u8g2.print("ppb");

    } while ( u8g2.nextPage() );
  }
}

void ccsReset(bool error) {

  if (error) {
    writeOledSetupMsg("ERROR!", "Resetting");
  } else {
    writeOledSetupMsg("Resetting", "");
  }
  digitalWrite(CCS_RESET, LOW); // hardware reset CCS811 sensor
  delay(10);
  digitalWrite(CCS_RESET, HIGH); // normal  CCS811 sensor
  delay(100);
  ccs.SWReset();
  delay(TIMETORESET);

}

void getLocation() {
  writeOledSetupMsg("Getting location", "");

  location_t loc = location.getGeoFromWiFi();

  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson());
  lat = loc.lat;
  lon = loc.lon;
  Serial.println("Latitude: " + String(loc.lat, 7));
  Serial.println("Longitude: " + String(loc.lon, 7));
  Serial.println("Accuracy: " + String(loc.accuracy));
}

void sendToCloud(bool error, float temp, long pressure, float pmData, float humidity,
                 long CO2, long TVOC) {
  Serial.println("Sending this data");
  Serial.print("T: ");
  Serial.print(temp);
  Serial.print("; P: ");
  Serial.print(pressure);
  Serial.print("; RH: ");
  Serial.print(humidity);
  Serial.print("; PM2.5: ");
  Serial.print(pmData);
  Serial.print("; CO2: ");
  Serial.print(CO2);
  Serial.print("; TVOC: ");
  Serial.println(TVOC);
  Serial.println();

  if (!error) {
    Serial.print("Sending data to cloud ");
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(tsHost, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    // We now create a body for the request
    String body = "";
    //    body += "/update";
    //    body += "?api_key=";
    body += privateKey;
    body += "&field1=";
    body += String(temp);
    body += "&field2=";
    body += String(humidity);
    body += "&field3=";
    body += String(pressure);
    body += "&field4=";
    body += String(CO2);
    body += "&field5=";
    body += String(TVOC);
    body += "&field6=";
    body += String(pmData);
    body += "\r\n\r\n";

    Serial.println("Sending POST data: ");
    Serial.println(body);

    client.println("POST /update HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("User-Agent: ESP8266 (nothans)/1.0");
    client.println("Connection: close");
    client.println("X-THINGSPEAKAPIKEY: " + privateKey);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Content-Length: " + String(body.length()));
    client.println("");
    client.print(body);
    delay(10);

    // Read all the lines of the reply from thingspeak server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");
    client.stop();
  }
}


/*********************************************************************+
   Function:    STA_setupWiFi
   Description:
   Parameters:  void
   Returns:     void
 *********************************************************************/
void STA_setupWiFi() {
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("My IP address as a Station is: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  webServer.on("/", STA_handleRoot);
  webServer.on("/reset", STA_handleReset);
  webServer.on("/restart", STA_handleRoot);
  webServer.on("/setup", STA_handleRoot);

  webServer.onNotFound(STA_handleNotFound);

  webServer.begin();
  Serial.print("HTTP webServer started (STA mode) at ");
  Serial.println(WiFi.localIP());
}

/*********************************************************************+
   Function:    AP_setup
   Description:
   Parameters:  void
   Returns:     String
 *********************************************************************/
String AP_setup()
{
  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  Serial.println (" ");
  String AP_NameString = ssid;
  AP_NameString += " " + macID;
  //  Serial.print ("AP Name (String): ");
  //  Serial.println (AP_NameString);
  //  Serial.print ("AP Name (length): ");
  //  Serial.println (AP_NameString.length());

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, AP_NameString.length() + 1, NULL);

  //  Serial.println ("AP Name (char by char): ");
  for (int i = 0; i < AP_NameString.length(); i++) {
    AP_NameChar[i] = AP_NameString.charAt(i);
    //    Serial.print ("  char(");
    //    Serial.print (i);
    //    Serial.print ("): ");
    //    Serial.println (AP_NameChar[i]);
  }
  AP_NameChar[AP_NameString.length()] = NULL;
  //  Serial.print ("  char(");
  //  Serial.print (AP_NameString.length());
  //  Serial.print ("): ");
  //  Serial.println (AP_NameChar[AP_NameString.length()]);
  //  Serial.print ("AP Name (char): ");
  //  Serial.println (AP_NameChar);

  //  WiFi.softAP(AP_NameChar, wifiPwd);
  WiFi.softAP(AP_NameChar);
  Serial.print(AP_NameChar);
  Serial.println(" started");

  return AP_NameString;
}

/********************************************************************
   name: AP_handleNotFound
   parameters: void
   returns: void
   description: function that manages a call to the "scan" page.
    it generates a web page that lists all the available WiFi
    networks. You can select the one you want to connect to and
    press a button to go to next step (password)
 ********************************************************************/
void AP_handleNotFound()
{
  //  digitalWrite(led, HIGH);
  String message = htmlHeader(0);
  message += "File Not Found\n\n";
  message += "<br>URI: ";
  message += webServer.uri();
  message += "\n<br>Method: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\n<br>Arguments: ";
  message += webServer.args();
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "<br>\n";
  }
  message += "\n<p>Go <a href=\"/\">home</a></p></body></html>";
  webServer.send(404, "text/html", message);
  Serial.println("handle not found");
  Serial.print("URI: ");
  Serial.println(webServer.uri());
  Serial.print("Method: ");
  Serial.println((webServer.method() == HTTP_GET) ? "GET" : "POST");
  Serial.print("Arguments: ");
  Serial.println(webServer.args());
  for (uint8_t i = 0; i < webServer.args(); i++) {
    Serial.print(" ");
    Serial.print(webServer.argName(i));
    Serial.print(": ");
    Serial.println(webServer.arg(i));
  }
  //  digitalWrite(led, LOW);
}

/********************************************************************
   name: STA_handleNotFound
   parameters: void
   returns: void
   description: function that manages a call to the "scan" page.
    it generates a web page that lists all the available WiFi
    networks. You can select the one you want to connect to and
    press a button to go to next step (password)
 ********************************************************************/
void STA_handleNotFound()
{
  //  digitalWrite(led, HIGH);
  String message = htmlHeader(0);
  message += "File Not Found\n\n";
  message += "<br>URI: ";
  message += webServer.uri();
  message += "\n<br>Method: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\n<br>Arguments: ";
  message += webServer.args();
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "<br>\n";
  }
  message += "\n<p>Go <a href=\"/\">home</a></p></body></html>";
  webServer.send(404, "text/html", message);
  Serial.println("handle not found");
  Serial.print("URI: ");
  Serial.println(webServer.uri());
  Serial.print("Method: ");
  Serial.println((webServer.method() == HTTP_GET) ? "GET" : "POST");
  Serial.print("Arguments: ");
  Serial.println(webServer.args());
  for (uint8_t i = 0; i < webServer.args(); i++) {
    Serial.print(" ");
    Serial.print(webServer.argName(i));
    Serial.print(": ");
    Serial.println(webServer.arg(i));
  }
  //  digitalWrite(led, LOW);
}

/********************************************************************
   name: STA_handleReset
   parameters: void
   returns: void
   description: function that manages a call to the "scan" page.
    it generates a web page that lists all the available WiFi
    networks. You can select the one you want to connect to and
    press a button to go to next step (password)
 ********************************************************************/
void STA_handleReset()
{
  //  digitalWrite(led, HIGH);
  clearEeprom ();
  String message = htmlHeader(0);
  if ((EEPROM.commit()) ? 1 : 0) {
    message += "EEPROM cleared";
  } else {
    message += "I could not clear the EEPROM";
  }
  message += "\n<p>Go <a href=\"/\">home</a></p></body></html>";
  webServer.send(200, "text/html", message);
  Serial.println("handle not found");
  Serial.print("URI: ");
  Serial.println(webServer.uri());
  Serial.print("Method: ");
  Serial.println((webServer.method() == HTTP_GET) ? "GET" : "POST");
  Serial.print("Arguments: ");
  Serial.println(webServer.args());
  for (uint8_t i = 0; i < webServer.args(); i++) {
    Serial.print(" ");
    Serial.print(webServer.argName(i));
    Serial.print(": ");
    Serial.println(webServer.arg(i));
  }
  //  digitalWrite(led, LOW);
}

/********************************************************************
   name: STA_handleRoot
   parameters: void
   returns: void
   description: ...
 ********************************************************************/
void STA_handleRoot()
{
  String ssid = "";
  int timeout = -1;

  //  digitalWrite(led, HIGH);
  String message = htmlHeader(0);
  message += "\n<h1>Hello from ESP8266</h1>";
  message += "\n<p>Connected to: ";
  message += WiFi.SSID();
  message += "</p>";
  message += "\n<p><a href=\"reset\">Reset Credentials</a></p>";
  message += "\n</body>";
  message += "\n</html>";
  webServer.send(200, "text/html", message);

  Serial.println("root handled");
  //  digitalWrite(led, LOW);
}

/********************************************************************
   name: AP_handleScanWiFi
   parameters: void
   returns: void
   description: function that manages a call to the "scan" page.
    it generates a web page that lists all the available WiFi
    networks. You can select the one you want to connect to and
    press a button to go to next step (password)
 ********************************************************************/
void AP_handleScanWiFi()
{
  //  digitalWrite(led, HIGH);

  int n = WiFi.scanNetworks();
  Serial.println("scan done");

  String message = htmlHeader(1);

  if (n == 0)
  {
    message += "<h1>no networks found</h1>";
    Serial.println(message);
  }
  else
  {
    message += "\n<form id=\"scanForm\" action=\"http://192.168.4.1/login\" enctype=\"text/plain\" method=\"POST\">";
    message += "\n<fieldset>";
    message += "\n<legend>";
    message += "I have found ";
    message += n;
    message += " networks.";
    message += "</legend>";
    for (int i = 0; i < n; ++i)
    {
      message += "\n";
      //      message += "\n<input name=\"auth\" type=\"hidden\" value=\"";
      //      message += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?"0":"1";
      //      message += "\" />";
      message += "<input type=\"radio\" name=\"ssid\" ";
      message += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "onclick=\"setPassVisibility('none');" : "onclick=\"setPassVisibility('inline');\" ";
      message += "value=\"";
      message += WiFi.SSID(i);
      message += "\"/>";
      message += WiFi.SSID(i);
      message += " (";
      message += WiFi.RSSI(i);
      message += " dBm)";
      message += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "* ";
      message += "<br />";
      delay(10);
    }
    message += "\n</fieldset>";
    message += "\n<div id=\"authDiv\">Auth: <input name=\"auth\" type=\"hidden\"/><br /></div>";
    message += "\n<div id=\"pass\">Password: <input name=\"pwd\" type=\"password\"/><br /></div>";
    message += "\n<input type=\"submit\"value=\"Connect to selected network\">";
    message += "\n<input type=\"reset\" value=\"Reset\">";
    message += "\n</form>";
    //    Serial.println(message);
  }
  message += "\n</body>";
  message += "\n</html>";
  webServer.send(200, "text/html", message);

  Serial.println("*** Scan page ***");
  //  digitalWrite(led, LOW);
}

/********************************************************************
   name:        htmlHeader
   returns:     [String]
   description:
 ********************************************************************/
String htmlHeader(int type) {
  String message;

  message = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  message += "\n<html>";
  message += "\n<head>";
  message += "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">";
  message += "\n<title>ESP8266 Sensor Setup</title>";
  if (type == 0) {
    message += "\n</head>";
    message += "\n<body>";
  } else if (type == 1) {
    message = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    message += "\n<html>";
    message += "\n<head>";
    message += "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">";
    message += "\n<title>ESP8266 Sensor Setup</title>";
    message += "\n<script>";
    message += "\nfunction init(){";
    message += "\nsetVisibility('pass', 'none');";
    message += "\n}";
    message += "\nfunction setVisibility(id, visibility) {";
    message += "\ndocument.getElementById(id).style.display = visibility;";
    message += "\n}";
    message += "\nfunction setPassVisibility(vis) {";
    message += "\nif ( vis == 'inline' ) {";
    message += "\ndocument.forms[\"scanForm\"][\"auth\"].value = \"1\";";
    message += "\n} else {";
    message += "\ndocument.forms[\"scanForm\"][\"auth\"].value = \"0\";";
    message += "\n}";
    message += "\ndocument.getElementById('pass').style.display = vis;";
    message += "\n}";
    message += "\nfunction myFunction(){";
    message += "\ndocument.getElementById(\"test\").innerHTML = \"new content\";";
    message += "\nsetVisibility('scanning', 'inline');";
    message += "\n}";
    message += "\n</script>";
    message += "\n</head>";
    message += "\n<body onload=\"init()\">";
  } else if (type == 2) {
    message = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    message += "\n<html>";
    message += "\n<head>";
    message += "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">";
    message += "\n<title>ESP8266 Sensor Setup</title>";
    message += "\n<script>";
  }

  return message;
}

/********************************************************************
   name: AP_handleLogin
   parameters: void
   returns: void
   description:
 ********************************************************************/

void AP_handleLogin()
{
  String ssid = "";
  String pwd = "";
  int auth = 0;

  //  digitalWrite(led, HIGH);
  Serial.println("*** Login page ***");
  if ( webServer.args() == 1 && find_text("\n", webServer.arg(0)) != -1 ) {
    //    Serial.println("* Reading arguments: strategy 1");
    char serverArgs[webServer.arg(0).length() + 1];
    memset(serverArgs, webServer.arg(0).length() + 1, NULL);
    for (int i = 0; i < webServer.arg(0).length(); ++i) {
      serverArgs[i] = webServer.arg(0).charAt(i);
      //      Serial.print(webServer.arg(0).charAt(i));
    }
    serverArgs[webServer.arg(0).length()] = NULL;
    Serial.println(" ");

    char * pch = strtok (serverArgs, "\n");
    int j = 0;
    while (pch != NULL)
    {
      char * p2ch = strpbrk(pch, "=");
      if (p2ch == NULL) {
        if (webServer.argName(0) == "ssid") {
          pch[strlen(pch) - 1] = NULL;
          ssid = pch;
        } else if (webServer.argName(0) == "pwd") {
          pch[strlen(pch) - 1] = NULL;
          pwd = pch;
        } else if (webServer.argName(0) == "auth") {
          pch[strlen(pch) - 1] = NULL;
          auth = atoi(pch);
        }
      } else {
        ++p2ch;
        if (strncmp (pch, "auth", 4) == 0) {
          auth = atoi(p2ch);
        } else if (strncmp (pch, "ssid", 4) == 0) {
          ssid = p2ch;
        } else if (strncmp (pch, "pwd", 3) == 0) {
          pwd = p2ch;
        }
      }
      pch = strtok (NULL, "\n");
      j++;

    }
  } else {
    //    Serial.println("* Reading arguments: strategy 2");
    for (uint8_t i = 0; i < webServer.args(); i++) {
      if (webServer.argName(i) == "ssid") {
        ssid = webServer.arg(i);
      } else if (webServer.argName(i) == "pwd") {
        pwd = webServer.arg(i);
      } else if (webServer.argName(i) == "auth") {
        (webServer.arg(i) == 0) ? auth = 0 : auth = 1;
      }
    }
  }
  Serial.print(ssid);
  Serial.print(" network has been selected. ");
  Serial.print( (( auth == 1 ) ? "Password: " : "Open network"));
  Serial.println( ( auth == 1 ) ? pwd : " ");

  // 3. confirm writing in webpage
  // 4a. if connection is successful, ask to restart
  // 4b. if connection dosn't work, go to scan page
  String message = htmlHeader(2);
  message += "\nfunction init(){";
  message += "\nvar nocache = '?nocache=' + Math.random() * 1000000;";
  message += "\nvar ssid = '&ssid=";
  message += ssid.c_str();
  message += "';";
  message += "\nvar auth = '&auth=";
  message += auth;
  message += "';";
  message += "\nvar pwd = '&pwd=";
  message += pwd.c_str();
  message += "';";
  message += "\nvar params = nocache + ssid + auth + pwd;";
  message += "\nvar request = new XMLHttpRequest();";
  message += "\nrequest.onreadystatechange = function() {";
  message += "\nvar statusStr = 'onreadystatechange: ' + this.readyState; ";
  message += "\ndocument.getElementById('http').innerHTML = statusStr;";
  message += "\nif (this.readyState == 4) {";
  message += "\nstatusStr = statusStr + ' - readyState: ' + this.status; ";
  message += "\ndocument.getElementById('http').innerHTML = statusStr; ";
  message += "\nif (this.status == 200) {";
  message += "\nstatusStr = statusStr + ' - status: ' + this.status;";
  message += "\ndocument.getElementById('http').innerHTML = statusStr;";
  message += "\nif (this.responseText != null) {";
  message += "\ndocument.getElementById('http').innerHTML = statusStr + ' - no status responsetext';";
  message += "\n}}}}";
  message += "\nrequest.open('POST', 'connect', true);";
  message += "\nrequest.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  message += "\nrequest.send(params);";
  message += "\ndocument.getElementById('request').innerHTML = 'request sent to connect. params: ' + params;";
  message += "\n}";
  message += "\n</script>";
  message += "\n</head>";
  message += "\n<body onload=\"init()\">";
  message += "\n<h1>*** Login page ***</h1><br>\n";
  message += ssid;
  message += " network has been selected. <br>\n";
  message += ( auth == 1 ) ? "Password sent<br>" : "Open network<btr>";
  //  message += ( auth==1 )? pwd:" ";
  message += "\n<div id=\"status\">Connecting to ";
  message += ssid;
  message += "</div><br>";
  message += "\nSending request <br>";
  message += "\n<div id=\"request\">test</div><br>";
  message += "\nhttp request <br>";
  message += "\n<div id=\"http\">test</div><br>";
  message += "\n</body>";
  message += "\n</html>";
  webServer.send(200, "text/html", message);

  Serial.println (" ");
  //  Serial.println(message);

  //  digitalWrite(led, LOW);
}

/********************************************************************
   name: AP_handleConnect
   parameters: void
   returns: void
   description:
 ********************************************************************/

void AP_handleConnect()
{
  String qssid;
  String qpwd;
  int qauth;
  String message = "";

  Serial.println("*** Connection Page ***");

  // Reading arguments
  Serial.println(" Reading Arguments");
  if ( webServer.args() == 1 && find_text("\n", webServer.arg(0)) != -1 ) {
    char serverArgs[webServer.arg(0).length() + 1];
    memset(serverArgs, webServer.arg(0).length() + 1, NULL);
    for (int i = 0; i < webServer.arg(0).length(); ++i) {
      serverArgs[i] = webServer.arg(0).charAt(i);
    }
    serverArgs[webServer.arg(0).length()] = NULL;

    char * pch = strtok (serverArgs, "\n");
    int j = 0;
    while (pch != NULL)
    {
      char * p2ch = strpbrk(pch, "=");
      if (p2ch == NULL) {
        if (webServer.argName(0) == "ssid") {
          qssid = pch;
        } else if (webServer.argName(0) == "pwd") {
          qpwd = pch;
        } else if (webServer.argName(0) == "auth") {
          (pch == 0) ? qauth = 0 : qauth = 1;
        }
      } else {
        ++p2ch;
        if (strncmp (pch, "auth", 4) == 0) {
          qauth = atoi(p2ch);
        } else if (strncmp (pch, "ssid", 4) == 0) {
          qssid = p2ch;
        } else if (strncmp (pch, "pwd", 3) == 0) {
          qpwd = p2ch;
        }
      }
      pch = strtok (NULL, "\n");
      j++;
    }
  } else {
    for (uint8_t i = 0; i < webServer.args(); i++) {
      if (webServer.argName(i) == "ssid") {
        qssid = webServer.arg(i);
      } else if (webServer.argName(i) == "pwd") {
        qpwd = webServer.arg(i);
      } else if (webServer.argName(i) == "auth") {
        (webServer.arg(i) == 0) ? qauth = 0 : qauth = 1;
      }
    }
  }
  message += "\n SSID: ";
  message += qssid;
  message += "\n pwd: ";
  message += qpwd;
  message += "\n auth: ";
  message += qauth;
  Serial.println(message);

  // Try to connect

  // Save data to EEPROM
  if (qssid.length() > 0 && qpwd.length() > 0 && qauth == 1 ) {
    message += "SSID and password received";
    clearEeprom();
    save2Eeprom (qssid, 0);
    save2Eeprom (qpwd, 32);
    EEPROM.commit();
    credentials = true;
  } else if (qssid.length() > 0 && qauth == 0 ) {
    message += "SSID received";
    clearEeprom();
    save2Eeprom (qssid, 0);
    EEPROM.commit();
    credentials = true;
  }

  webServer.send(200, "application/json", message);
}

/*
   http://playground.arduino.cc/Main/FindText
*/
int find_text(String needle, String haystack) {
  int foundpos = -1;
  for (int i = 0; (i < haystack.length() - needle.length()); i++) {
    if (haystack.substring(i, needle.length() + i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}

/*

*/
void clearEeprom () {
  Serial.println("clearing EEPROM");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, NULL);
  }
}

/*
   returns number of characters written
*/
int save2Eeprom (String str, int first) {
  Serial.println("writing EEPROM");

  // verify that length is less than space
  int last = first + str.length();
  int i;
  for (i = 0; i < str.length(); ++i) {
    EEPROM.write(first + i, str[i]);
  }
  return i;
}

/*

*/
int tryConnecting(String ssid, String pass, int auth) {
  int len = 0;
  int completed = 0;
  while (completed == 0) {
    if (ssid.c_str()[len++] == NULL) {
      completed = 1;
    }
  }

  if ( len > 1 ) {
    Serial.println("SSID contains ");
    Serial.println(len - 1);
    Serial.println(" characters");
    // test essid
    // try to start wifi connection with just read SSID and password
    WiFi.begin(ssid.c_str(), pass.c_str());
    // test wifi connection with just read SSID and password
    if ( testWifi() == 20 ) {
      Serial.println("");
      Serial.println("WiFi connected");
      return 20;
    } else {
      Serial.println("");
      Serial.println("Could not connect to WiFi");
    }
  } else {
    return 0;
  }
}

