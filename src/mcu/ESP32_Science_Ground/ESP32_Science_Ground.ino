/*
   There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.
   U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
   U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
   U2UXD is unused and can be used for your projects.
*/
#include <WiFi.h>
#include <math.h>
#include <Arduino.h>
#include <Wire.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> //v6

/*************************************************/
/*************** SENSORS DEFS #BEG ***************/

#define SHT2X 0
#define SHT1X 1

#define MOIST_SENSOR SHT1X

#if MOIST_SENSOR == SHT1X
#include <SHT1x-ESP.h>

// default to 5.0v boards, e.g. Arduino UNO
SHT1x sht1x(21, 22);
#endif

/*************** SENSORS DEFS #END ***************/
/*************************************************/

WiFiClient client;
HTTPClient http;

byte mac[6];

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

String orario;
String postMessage;
String terrariumID;

enum stato
{
  INIT,
  API_AUTH,
  MEASURING_SHT_20,
  REQUEST_MH_Z19B,
  RECEIVING_MH_Z19B,
  RECEIVED_MH_Z19B,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  NTP_GET_TIME,
  NTP_GOT_TIME,
  PREPARE_DATA,
  SEND_DATA,
  WAITING_RESPONSE,
  RESPONSE_OK,
  WIFI_DISCONNECT,
  STANDBY,
  ERRORE
};
const unsigned long HTTP_TIMEOUT = 10000; // max response time from server
char response[800];                       // this fixed sized buffers works well for this project using the NodeMCU.

int stato_macchina = INIT;
int auth_state = 0;
int session_state = 0;
const char *ssid = "Pie_2";
const char *password = "prova123";

#define RXD2 16
#define TXD2 17
#define BL 9
#define NSEC 10

// Set of commands for MH-Z19B
// last byte is checksum, calculated as negation of the sum of all bytes except FF
#define CMD_READCO2                                      \
  {                                                      \
    0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 \
  } // Read CO2 concentration
#define CMD_ZEROCAL                                      \
  {                                                      \
    0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78 \
  } // ZERO POINT CALIBRATION
#define CMD_SPANCAL                                      \
  {                                                      \
    0xFF, 0x01, 0x88, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x9F \
  } // SPAN POINT CALIBRATION (2000 ppm)
#define CMD_ABC_ON                                       \
  {                                                      \
    0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE5 \
  } // ABC logic on (Automatic Baseline Correction)
#define CMD_ABC_OFF                                      \
  {                                                      \
    0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85 \
  } // ABC logic off (Automatic Baseline Correction)
#define CMD_RNGSET2                                      \
  {                                                      \
    0xFF, 0x01, 0x99, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x8E \
  } // Sensor detection range setting to 2000 ppm
#define CMD_RNGSET5                                      \
  {                                                      \
    0xFF, 0x01, 0x99, 0x13, 0x88, 0x00, 0x00, 0x00, 0xCA \
  } // Sensor detection range setting to 5000 ppm

// Set of parameters for SHT20
#define SHT20_I2C 0x40
#define SHT20_TEMP 0xF3
#define SHT20_HUMID 0xF5
#define SHT20_WRITE_USER_REG 0xE6
#define SHT20_READ_USER_REG 0xE7
#define SHT20_RESET 0xFE
#define _DISABLE_ONCHIP_HEATER 0b00000000
#define _ENABLE_OTP_RELOAD 0b00000000
#define _DISABLE_OTP_RELOAD 0b00000010
#define _RESERVED_BITMASK 0b00111000
#define SOFT_RESET_DELAY 20
#define TEMPERATURE_DELAY 100
#define HUMIDITY_DELAY 40
#define SHT20_RESOLUTION_12BITS 0b00000000
#define SHT20_RESOLUTION_11BITS 0b10000001
#define SHT20_RESOLUTION_10BITS 0b10000000
#define SHT20_RESOLUTION_8BITS 0b00000001
uint8_t _resolution;
uint8_t _onchip_heater;
uint8_t _otp_reload;
uint8_t _address;
TwoWire *_i2cPort;
float tempC;
float vpd_kPa;
float dew_pointC;
float RH;
int CO2;

const char *server = "46.101.232.24";                         // server's address
const int serverPort = 8080;                                  // server port
const char *resource = "/data/measures";                      // resource requested
const char *login = "/devices/auth";                          // auth API
const char *startSession = "/data/terrariums/sessions/start"; // start session API
const char *stopSession = "/data/terrariums/sessions/stop";   // stop session API
StaticJsonDocument<1024> authResponse;                        // global var -> used in the main loop to extract login infos
StaticJsonDocument<512> sessionApiResponse;
String globalSession; // global session
// const char* resource = "/data/terrariums";    // resource requested

int i = 0;
byte resp[BL];

char getCheckSum(char *);
bool cmdSent = false;
long start = millis();
bool standby = false;

void sendCmdToMhz19(char *);
char getCheckSum(char *);
bool tryAuth(String *, int *);

bool startSessionFlag = false;
bool stopSessionFlag = false;
bool _SESSION_ACTIVE = false;
bool _FAST_MODE_ACTIVE = false;

// Start
void IRAM_ATTR startNewSession_ISR()
{
  Serial.println("Start new session interrupt");
  startSessionFlag = true;
  stopSessionFlag = false;
}

void IRAM_ATTR stopSession_ISR()
{
  Serial.println("Stop session interrupt");
  startSessionFlag = false;
  stopSessionFlag = true;
}

#define GPIO_START_SESSION 12
#define GPIO_START_SESSION_LED 14
#define GPIO_STOP_SESSION 33
#define GPIO_STOP_SESSION_LED 32
#define GPIO_FAST_MODE 19
#define GPIO_FAST_MODE_LED 18

bool ledFlag = false;

void i_o_task(void *null)
{
  for (;;)
  {
    // Inputs
    _FAST_MODE_ACTIVE = digitalRead(GPIO_FAST_MODE);
    // Outputs

    delay(100);
  }
}

void ledTask(void *you_need_this)
{

  for (;;)
  {
    ledFlag = !ledFlag;
    if(_SESSION_ACTIVE){
      digitalWrite(GPIO_START_SESSION_LED, _SESSION_ACTIVE);
    }else{
      digitalWrite(GPIO_START_SESSION_LED, startSessionFlag && ledFlag);
    }
    digitalWrite(GPIO_STOP_SESSION_LED, stopSessionFlag && ledFlag);
    digitalWrite(GPIO_FAST_MODE_LED, _FAST_MODE_ACTIVE && ledFlag);

    delay(500);
  }
}

void mainTask(void *you_need_this)
{
  for (;;)
  {
    switch (stato_macchina)
    {
    case (INIT): // Initial status
    {
      Serial.println("STATUS: INIT");
    }
      stato_macchina = API_AUTH;
      break;

    case (API_AUTH): // Try login to server before start loop
    {
      if (tryAuth(&terrariumID, &auth_state))
      {
        stato_macchina = MEASURING_SHT_20;
      }
      break;
    }

    case (MEASURING_SHT_20): // Status used for collecting measurements from SHT20
    {
      Serial.println("--------------------------");
      Serial.println("STATUS: MEASURING_SHT_20");
      shtMeasure_all();
      Serial.println("Temperatura: " + (String)tempC + "°C");
      Serial.println("Punto di rugiada: " + (String)dew_pointC + "°C dew point");
      Serial.println("Umidita' relativa: " + (String)RH + " %");
      Serial.println("deficit di pressione di vapore: " + (String)vpd_kPa + " kPa");
      stato_macchina = REQUEST_MH_Z19B;
    }
    break;

    case (REQUEST_MH_Z19B): // Status used for requesting measurements from MH-Z19B
    {
      Serial.println("STATUS: REQUEST_MH_Z19B");
      byte buf[BL] = CMD_READCO2;
      sendCmdToMhz19(buf);
      stato_macchina = RECEIVING_MH_Z19B;
      Serial.println("STATUS: RECEIVING_MH_Z19B");
    }
    break;

    case (RECEIVING_MH_Z19B): // Status used for waiting measurements from MH-Z19B
    {
      while (cmdSent && Serial2.available() > 0)
      {
        resp[i] = int(Serial2.read());
        i++;
        if (i > 8)
        {
          i = 0;
          cmdSent = false;
          start = millis();
          stato_macchina = RECEIVED_MH_Z19B;
        }
      }
    }
    break;

    case (RECEIVED_MH_Z19B): // Status used to print measurements from MH-Z19B
    {
      Serial.println("STATUS: RECEIVED_MH_Z19B");
      CO2 = resp[2] * 256 + resp[3];
      Serial.print("Concentrazione CO2: ");
      Serial.println((String)CO2);
      if (startSessionFlag || stopSessionFlag)
      {
        if (sessionHandler(authResponse["ID"].as<String>(), &session_state, startSessionFlag == true))
        {
          startSessionFlag = false;
          stopSessionFlag = false;
          session_state = 0;
        }
      }
      else
      {
        stato_macchina = WIFI_CONNECTING;
      }
      break;
    }

    case (WIFI_CONNECTING): // Status used to connect to a known Wi-Fi network
    {
      Serial.println("STATUS: WIFI_CONNECTING");
      WiFi.begin(ssid, password);
      int counter = 0;
      Serial.print("Connecting to ");
      Serial.print(ssid);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("");
      stato_macchina = WIFI_CONNECTED;
    }
    break;

    case (WIFI_CONNECTED): // Status used when connection has been established to a Wi-Fi network
    {
      Serial.println("STATUS: WIFI_CONNECTED");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      stato_macchina = NTP_GET_TIME;
    }
    break;

    case (NTP_GET_TIME): // Status used to init NTP client
    {
      Serial.println("STATUS: NTP_GET_TIME");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      stato_macchina = NTP_GOT_TIME;
    }
    break;

    case (NTP_GOT_TIME): // Status used to get time from NTP server
    {
      Serial.println("STATUS: NTP_GOT_TIME");
      // Init and get the time
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo))
      {
        Serial.println("*** Failed to obtain time");
        stato_macchina = NTP_GET_TIME;
        return;
      }
      else
      {
        Serial.println("*** Got time");
        char buf[30];
        strftime(buf, 30, "%Y-%m-%d %H:%M:%S", &timeinfo);
        orario = (String)buf;
        Serial.println("Time:" + orario);
        stato_macchina = PREPARE_DATA;
      }
    }
    break;

    case (PREPARE_DATA): // Status used to start connecting to the server that collects measurements
    {
      Serial.println("STATUS: PREPARE_DATA");
      StaticJsonDocument<768> doc;

      JsonArray Data = doc.createNestedArray("Data");

      JsonObject Data_Temp = Data.createNestedObject();
      Data_Temp["TerrariumID"] = authResponse["ID"];
      Data_Temp["SensorID"] = authResponse["Sensors"]["Temperature_1"]["ID"];
      Data_Temp["Value"] = (String)tempC;
      Data_Temp["Timestamp"] = (String)orario;
      Data_Temp["SessionKey"] = globalSession;

      JsonObject Data_Rugiada = Data.createNestedObject();
      Data_Rugiada["TerrariumID"] = authResponse["ID"];
      Data_Rugiada["SensorID"] = authResponse["Sensors"]["PuntoRugiada_1"]["ID"];
      Data_Rugiada["Value"] = (String)dew_pointC;
      Data_Rugiada["Timestamp"] = (String)orario;
      Data_Rugiada["SessionKey"] = globalSession;

      JsonObject Data_Umid = Data.createNestedObject();
      Data_Umid["TerrariumID"] = authResponse["ID"];
      Data_Umid["SensorID"] = authResponse["Sensors"]["Humid_1"]["ID"];
      Data_Umid["Value"] = (String)RH;
      Data_Umid["Timestamp"] = (String)orario;
      Data_Umid["SessionKey"] = globalSession;

      JsonObject Data_VPD = Data.createNestedObject();
      Data_VPD["TerrariumID"] = authResponse["ID"];
      Data_VPD["SensorID"] = authResponse["Sensors"]["VPD_1"]["ID"];
      Data_VPD["Value"] = (String)vpd_kPa;
      Data_VPD["Timestamp"] = (String)orario;
      Data_VPD["SessionKey"] = globalSession;

      JsonObject Data_CO2 = Data.createNestedObject();
      Data_CO2["TerrariumID"] = authResponse["ID"];
      Data_CO2["SensorID"] = authResponse["Sensors"]["CO2_1"]["ID"];
      Data_CO2["Value"] = (String)CO2;
      Data_CO2["Timestamp"] = (String)orario;
      Data_CO2["SessionKey"] = globalSession;

      postMessage = "";
      serializeJson(doc, postMessage);
      serializeJson(doc, Serial);
      stato_macchina = SEND_DATA;
    }
    break;

    case (SEND_DATA): // Status used to start connecting to the server that collects measurements
    {
      Serial.println("STATUS: SEND_DATA");
      if (WiFi.status() == WL_CONNECTED)
      { // Check WiFi connection status
        String URI = (String) "http://" + (String)server + (String) ":" + (String)serverPort + (String)resource;
        Serial.println("Connecting to " + URI);
        http.begin(URI); // Specify destination for HTTP request
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(postMessage);
        if (httpCode > 0)
        { // Check for the returning code
          String payload = http.getString();
          Serial.println(httpCode);
          Serial.println(payload);
          stato_macchina = RESPONSE_OK;
        }
        else
        {
          Serial.println("Error on HTTP request");
          Serial.println(httpCode);
          stato_macchina = ERRORE;
        }
      }
      else
      {
        Serial.println("Lost connection");
        stato_macchina = ERRORE;
      }
    }
    break;

    case (WAITING_RESPONSE): // Status used when waiting for a response from the server that collects measurements
    {
      Serial.println("STATUS: WAITING_RESPONSE");
      skipResponseHeaders();
      stato_macchina = RESPONSE_OK;
    }
    break;

    case (RESPONSE_OK): // Status used when response received from the server that collects measurements is OK
    {
      Serial.println("STATUS: RESPONSE_OK");
      Serial.println(response);
      if (!client.connected())
      {
        Serial.println();
        Serial.println("disconnecting http client...");
        client.stop();
      }
      stato_macchina = WIFI_DISCONNECT;
    }
    break;

    case (WIFI_DISCONNECT): // Status used when disconnecting from the Wi-Fi network
    {
      Serial.println("STATUS: WIFI_DISCONNECT");
      WiFi.disconnect();
      stato_macchina = STANDBY;
    }
    break;

    case (STANDBY): // Status used when MCU goes to standby (actually this can be better, by using deep sleep and so on)
    {
      if (standby)
      {
        int cycleTime = _FAST_MODE_ACTIVE ? 2 : NSEC;
        if (millis() - start > (1000 * cycleTime))
        {
          standby = false;
          stato_macchina = MEASURING_SHT_20;
        }
      }
      else
      {
        Serial.println("STATUS: STANDBY");
        standby = true;
      }
    }
    break;

    case (ERRORE): // status used in case of error
    {
      Serial.println("STATUS: ERRORE");
      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.disconnect();
      }
      stato_macchina = STANDBY;
    }
    break;

    default:
      break;
    }
    delay(100);
  }
}

void setup()
{
  Serial.begin(115200);                        // Seriale standard per la comunicazione di debug
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // Seriale 2 usata per connettersi al sensore MH-Z19B

#if MOIST_SENSOR == SHT2X
  sht2XBegin(); // Inizializzazione sensore SHT20
#endif
  byte buf[BL] = CMD_ABC_OFF; // We are setting Automatic Baseline Correction to OFF
  sendCmdToMhz19(buf);
  cmdSent = false;                 // ready to process new commands on MH-Z19B (to be changed, absorbed by state machine)
  client.setTimeout(HTTP_TIMEOUT); // set Client timeout
  /* Buttons interrupts */
  pinMode(GPIO_START_SESSION, INPUT);      // Start session  - button
  pinMode(GPIO_START_SESSION_LED, OUTPUT); // Start session  - led
  pinMode(GPIO_STOP_SESSION, INPUT);       // Stop session   - button
  pinMode(GPIO_STOP_SESSION_LED, OUTPUT);  // Stop session   - led
  pinMode(GPIO_FAST_MODE, INPUT);          // Fast-mode      - switch
  pinMode(GPIO_FAST_MODE_LED, OUTPUT);     // Fast-mode      - led
  attachInterrupt(GPIO_START_SESSION, startNewSession_ISR, RISING);
  attachInterrupt(GPIO_STOP_SESSION, stopSession_ISR, RISING);

  /***********************************/
  /************  TASKS ***************/
  // Now set up two tasks to run independently.
  xTaskCreate(
      mainTask, "mainTask" // A name just for humans
      ,
      10240 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 0 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL);

  xTaskCreate(
      ledTask, "ledTask" // A name just for humans
      ,
      1024 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL);

  xTaskCreate(
      i_o_task, "i_o_task" // A name just for humans
      ,
      1024 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL);
}

void loop()
{ // Choose Serial1 or Serial2 as required
  delay(50);
}

void sendCmdToMhz19(byte *buf)
{
  //  Serial.print("Bytes available for write in Serial2: "),
  //               Serial.println(Serial2.availableForWrite());
  Serial2.write(buf, BL);
  cmdSent = true;
}

char getCheckSum(char *packet)
{
  char i, checksum;
  for (i = 1; i < 8; i++)
  {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}

bool shtConnected()
{
  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_READ_USER_REG);
  Wire.endTransmission();
  Wire.requestFrom(SHT20_I2C, 1);
  uint8_t config = Wire.read();

  if (config != 0xFF)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void shtReset()
{
  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_RESET);
  Wire.endTransmission();
  delay(SOFT_RESET_DELAY);
  _onchip_heater = _DISABLE_ONCHIP_HEATER;
  _otp_reload = _DISABLE_OTP_RELOAD;

  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_READ_USER_REG);
  Wire.endTransmission();
  Wire.requestFrom(SHT20_I2C, 1);
  uint8_t config = Wire.read();
  config = ((config & _RESERVED_BITMASK) | _resolution | _onchip_heater | _otp_reload);
  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_WRITE_USER_REG);
  Wire.write(config);
  Wire.endTransmission();
}

float shtTemperature()
{
  shtReset();
  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_TEMP);
  Wire.endTransmission();
  delay(TEMPERATURE_DELAY);
  Wire.requestFrom(SHT20_I2C, 2);
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint16_t value = msb << 8 | lsb;
  tempC = value * (175.72 / 65536.0) - 46.85;
  return tempC;
}

float shtHumidity()
{
  shtReset();
  Wire.beginTransmission(SHT20_I2C);
  Wire.write(SHT20_HUMID);
  Wire.endTransmission();
  delay(HUMIDITY_DELAY);
  Wire.requestFrom(SHT20_I2C, 2);
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint16_t value = msb << 8 | lsb;
  RH = value * (125.0 / 65536.0) - 6.0;
  return RH;
}

float shtVpd()
{
#if MOIST_SENSOR == SHT1X
  tempC = sht1x.readTemperatureC();
  RH = sht1x.readHumidity();
#else
  tempC = shtTemperature();
  RH = shtHumidity();
#endif

  float es = 0.6108 * exp(17.27 * tempC / (tempC + 237.3));
  float ae = RH / 100 * es;
  vpd_kPa = es - ae;

  return vpd_kPa;
}

float shtDew_point()
{
#if MOIST_SENSOR == SHT1X
  tempC = sht1x.readTemperatureC();
  RH = sht1x.readHumidity();
#else
  tempC = shtTemperature();
  RH = shtHumidity();
#endif
  float tem = -1.0 * tempC;
  float esdp = 6.112 * exp(-1.0 * 17.67 * tem / (243.5 - tem));
  float ed = RH / 100.0 * esdp;
  float eln = log(ed / 6.112);
  dew_pointC = -243.5 * eln / (eln - 17.67);

  return dew_pointC;
}

bool sht2XBegin()
{
  Wire.begin(); // Inizializzazione I2C usata per SHT20
  _address = SHT20_I2C;
  _resolution = SHT20_RESOLUTION_12BITS;
  _i2cPort = &Wire;

  return shtConnected();
}

void shtMeasure_all()
{
  shtVpd();
  shtDew_point();
}

bool skipResponseHeaders()
{
  stato_macchina = WAITING_RESPONSE;

  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (ok)
  {
    client.readBytes(response, 800);
    stato_macchina = RESPONSE_OK;
  }
  else
  {
    Serial.println("No response or invalid response!");
    stato_macchina = ERRORE;
  }
  return ok;
}

/* Try to authenticate to service
 * On success receives terrarium ID
 */

#define AUTH_CONNECT 0
#define CREATE_REQUEST 1
#define SEND_LOGIN_REQ 2
#define WAIT_RESPONSE 3
#define CHECK_RESPONSE 4
#define DISCONNECT 5
#define _ERROR 255
#define AUTH_STANDBY 6
#define AUTH_CLOSE 7

#define _TYPE_OF_TERRARIUM "Terrain"
#define _TERRARIUM_ALIAS "Lorenzo's Terrarium"
#define _MAGIC_KEY "InmfNpOwCSJJhXbUnptbK5c9tdGb4CnDdPrx9WWSlu9FNELGKMfpCpAifJSpbMSHMxgN7IxKmyFlFmnF6dhF3dr3h4vnVGAze9Cpqf1z3dpIY5U37jbpmZqhNv09AaxK6WqIc3CqgYQRs7ROGWuTzBZ9vX2AVoATX0Nz0hixb9iuxUfCTRE8BqDmyhknYGWTGKubF2HuMcAsytgyL47pNiFMPcSMksBUm1hmA5EMSjSq91cjz3w2sJPldAezdZBV"

#define TRY_BACK_TIME 300 // 5 minutes

String loginResponse;
String sessionResponse;
String session;

bool tryAuth(String *_terrariumId, int *auth_step)
{

  switch (*auth_step)
  {

  case AUTH_CONNECT: // start connecion
  {
    Serial.println("STATUS: WIFI_CONNECTING");
    WiFi.begin(ssid, password);
    int counter = 0;
    Serial.print("Connecting to ");
    Serial.print(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    WiFi.macAddress(mac);
    *auth_step = CREATE_REQUEST;
    return false;
    break;
  }

  case CREATE_REQUEST:
  {
    Serial.println("STATUS: CREATE_REQUEST");
    StaticJsonDocument<2048> doc;
    StaticJsonDocument<2048> sensorsArray;

    String wifiMacString = WiFi.macAddress();

    doc["MACAddres"] = wifiMacString;
    doc["MagicKey"] = _MAGIC_KEY;
    doc["TypeOfTerrarium"] = _TYPE_OF_TERRARIUM;
    doc["TerrariumAlias"] = wifiMacString; // _TERRARIUM_ALIAS; in order to semplify terrariums gestion we set Alias using MACAddress TODO: remove gestion server-side

    sensorsArray[0]["Name"] = "Internal temperature sensor";
    sensorsArray[0]["TypeOfMeasure"] = "Temperature_1";
    sensorsArray[0]["Extra_data"] = "°C";

    sensorsArray[1]["Name"] = "Punto Rugiada";
    sensorsArray[1]["TypeOfMeasure"] = "PuntoRugiada_1";
    sensorsArray[1]["Extra_data"] = "";

    sensorsArray[2]["Name"] = "Humidity";
    sensorsArray[2]["TypeOfMeasure"] = "Humid_1";
    sensorsArray[2]["Extra_data"] = "";

    sensorsArray[3]["Name"] = "VPD";
    sensorsArray[3]["TypeOfMeasure"] = "VPD_1";
    sensorsArray[3]["Extra_data"] = "%";

    sensorsArray[4]["Name"] = "Punto CO2";
    sensorsArray[4]["TypeOfMeasure"] = "CO2_1";
    sensorsArray[4]["Extra_data"] = "ppm";

    doc["Sensors"] = sensorsArray;

    serializeJson(doc, postMessage);
    serializeJson(doc, Serial);

    *auth_step = SEND_LOGIN_REQ;
    return false;
    break;
  }

  case SEND_LOGIN_REQ:
  {

    Serial.println("STATUS: SEND_LOGIN_REQ");
    if (WiFi.status() == WL_CONNECTED)
    { // Check WiFi connection status
      String URI = (String) "http://" + (String)server + (String) ":" + (String)serverPort + (String)login;
      Serial.println("Connecting to " + URI);
      http.begin(URI); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(postMessage);
      if (httpCode > 0)
      { // Check for the returning code
        loginResponse = http.getString();
        Serial.println(httpCode);
        Serial.println(loginResponse);
        *auth_step = CHECK_RESPONSE;
      }
      else
      {
        Serial.println("Error on HTTP request");
        Serial.println(httpCode);
        *auth_step = _ERROR;
      }
    }
    else
    {
      Serial.println("Lost connection");
      *auth_step = _ERROR;
    }
    return false;
    break;
  }

  case _ERROR:
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFi.disconnect();
    }
    Serial.println("Error, try again");
    *auth_step = AUTH_CONNECT;
    delay(TRY_BACK_TIME * 1000);
    break;
  }

  case (CHECK_RESPONSE): // Status used when response received from the server that collects measurements is OK
  {
    Serial.println("STATUS: RESPONSE_OK");
    Serial.println(loginResponse);
    if (!client.connected())
    {
      Serial.println();
      Serial.println("disconnecting http client...");
      client.stop();
    }
    deserializeJson(authResponse, loginResponse);
    const char *id = authResponse["ID"];
    Serial.println(id);
    *auth_step = AUTH_CLOSE;
  }
    return false;
    break;

  case (AUTH_CLOSE): // Status used when disconnecting from the Wi-Fi network
  {
    Serial.println("STATUS: AUTH_CLOSE");
    WiFi.disconnect();
    return true;
  }
  break;
  }
}

/*
 * Request for a new session
 */
#define SESSION_START_CONNECTION 0
#define SESSION_CREATE_REQUEST 1
#define SEND_START_SESSION_REQ 2
#define SEND_STOP_SESSION_REQ 3
#define SESSION_WAIT_RESPONSE 4
#define SESSION_CHECK_RESPONSE 5
#define SESSION_DISCONNECT 6
#define SESSION__ERROR 255
#define SESSION_STANDBY 7
#define SESSION_CLOSE 8

String SessionResponse;

bool sessionHandler(String _terrariumId, int *session_step, bool startStop)
{

  switch (*session_step)
  {
  case SESSION_START_CONNECTION: // start connecion
  {
    Serial.println("STATUS: SESSION_START_CONNECTION");
    WiFi.begin(ssid, password);
    int counter = 0;
    Serial.print("Connecting to ");
    Serial.print(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    WiFi.macAddress(mac);
    *session_step = SESSION_CREATE_REQUEST;
    return false;
    break;
  }
  case SESSION_CREATE_REQUEST:
  {
    Serial.println("STATUS: SESSION_CREATE_REQUEST");
    StaticJsonDocument<512> doc;

    postMessage = "";

    doc["TerrariumID"] = _terrariumId;

    serializeJson(doc, postMessage);
    serializeJson(doc, Serial);

    *session_step = startStop == true ? SEND_START_SESSION_REQ : SEND_STOP_SESSION_REQ;
    return false;
    break;
  }
  case SEND_START_SESSION_REQ:
  {
    Serial.println("STATUS: SEND_START_SESSION_REQ");
    if (WiFi.status() == WL_CONNECTED)
    { // Check WiFi connection status
      String URI = (String) "http://" + (String)server + (String) ":" + (String)serverPort + (String)startSession;
      Serial.println("Connecting to " + URI);
      http.begin(URI); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(postMessage);
      if (httpCode > 0)
      { // Check for the returning code
        sessionResponse = http.getString();
        Serial.println(httpCode);
        Serial.println(sessionResponse);
        *session_step = SESSION_CHECK_RESPONSE;
      }
      else
      {
        Serial.println("Error on HTTP request");
        Serial.println(httpCode);
        *session_step = _ERROR;
      }
    }
    else
    {
      Serial.println("Lost connection");
      *session_step = _ERROR;
    }
    return false;
    break;
  }
  case SEND_STOP_SESSION_REQ:
  {
    Serial.println("STATUS: SEND_STOP_SESSION_REQ");
    if (WiFi.status() == WL_CONNECTED)
    { // Check WiFi connection status
      String URI = (String) "http://" + (String)server + (String) ":" + (String)serverPort + (String)startSession;
      Serial.println("Connecting to " + URI);
      http.begin(URI); // Specify destination for HTTP request
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(postMessage);
      if (httpCode > 0)
      { // Check for the returning code
        sessionResponse = http.getString();
        Serial.println(httpCode);
        Serial.println(sessionResponse);
        *session_step = SESSION_CHECK_RESPONSE;
      }
      else
      {
        Serial.println("Error on HTTP request");
        Serial.println(httpCode);
        *session_step = _ERROR;
      }
    }
    else
    {
      Serial.println("Lost connection");
      *session_step = _ERROR;
    }
    return false;
    break;
  }
  case _ERROR:
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFi.disconnect();
    }
    Serial.println("Error, try again");
    *session_step = SESSION_START_CONNECTION;
    delay(TRY_BACK_TIME * 1000);
    break;
  }

  case SESSION_CHECK_RESPONSE: // Status used when response received from the server that collects measurements is OK
  {
    Serial.println("STATUS: RESPONSE_OK");
    Serial.println(sessionResponse);
    if (!client.connected())
    {
      Serial.println();
      Serial.println("disconnecting http client...");
      client.stop();
    }
    if (startStop)
    {
      deserializeJson(sessionApiResponse, sessionResponse);
      globalSession = sessionApiResponse["SessionKey"].as<String>();
      Serial.print("Session started : ");
      Serial.println(globalSession);
      _SESSION_ACTIVE = true;
    }
    else
    {
      _SESSION_ACTIVE = false;
      globalSession = "";
      Serial.println("Session stopped");
    }

    *session_step = SESSION_CLOSE;
    return false;
    break;
  }

  case (SESSION_CLOSE): // Status used when disconnecting from the Wi-Fi network
  {
    Serial.println("STATUS: SESSION CLOSE");
    WiFi.disconnect();
    *session_step = 0;
    return true;
  }
  }
}
