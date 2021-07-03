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

WiFiClient client;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

String orario;

enum stato {
  INIT,
  MEASURING_SHT_20,
  REQUEST_MH_Z19B,
  RECEIVING_MH_Z19B,
  RECEIVED_MH_Z19B,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  NTP_GET_TIME,
  NTP_GOT_TIME,
  SERVER_CONNECTING,
  SERVER_CONNECTED,
  SENDING_GET_REQUEST,
  SENT_GET_REQUEST,
  WAITING_RESPONSE,
  RESPONSE_OK,
  WIFI_DISCONNECT,
  STANDBY,
  ERRORE
};
const unsigned long HTTP_TIMEOUT = 10000;  // max response time from server
char response[800]; // this fixed sized buffers works well for this project using the NodeMCU.

int stato_macchina = INIT;
const char* ssid     = "JamiroOspite";
const char* password = "ciaociao";

#define RXD2 16
#define TXD2 17
#define BL 9
#define NSEC 60

// Set of commands for MH-Z19B
// last byte is checksum, calculated as negation of the sum of all bytes except FF
#define CMD_READCO2 {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79} // Read CO2 concentration
#define CMD_ZEROCAL {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78} // ZERO POINT CALIBRATION
#define CMD_SPANCAL {0xFF, 0x01, 0x88, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x9F} // SPAN POINT CALIBRATION (2000 ppm)
#define CMD_ABC_ON  {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE5} // ABC logic on (Automatic Baseline Correction)
#define CMD_ABC_OFF {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85} // ABC logic off (Automatic Baseline Correction)
#define CMD_RNGSET2 {0xFF, 0x01, 0x99, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x8E} // Sensor detection range setting to 2000 ppm
#define CMD_RNGSET5 {0xFF, 0x01, 0x99, 0x13, 0x88, 0x00, 0x00, 0x00, 0xCA} // Sensor detection range setting to 5000 ppm

// Set of parameters for SHT20
#define SHT20_I2C              0x40
#define SHT20_TEMP             0xF3
#define SHT20_HUMID            0xF5
#define SHT20_WRITE_USER_REG   0xE6
#define SHT20_READ_USER_REG    0xE7
#define SHT20_RESET            0xFE
#define _DISABLE_ONCHIP_HEATER 0b00000000
#define _ENABLE_OTP_RELOAD     0b00000000
#define _DISABLE_OTP_RELOAD    0b00000010
#define _RESERVED_BITMASK      0b00111000
#define SOFT_RESET_DELAY       20
#define TEMPERATURE_DELAY      100
#define HUMIDITY_DELAY         40
#define SHT20_RESOLUTION_12BITS      0b00000000
#define SHT20_RESOLUTION_11BITS      0b10000001
#define SHT20_RESOLUTION_10BITS      0b10000000
#define SHT20_RESOLUTION_8BITS       0b00000001
uint8_t _resolution;
uint8_t _onchip_heater;
uint8_t _otp_reload;
uint8_t _address;
TwoWire *_i2cPort;
float tempC;
float vpd_kPa;
float dew_pointC;
float RH;


int i = 0;
byte resp[BL];

char getCheckSum(char *);
bool cmdSent = false;
long start = millis();
bool standby = false;

void sendCmdToMhz19(char *);
char getCheckSum(char *);

void setup() {
  Serial.begin(115200);                         // Seriale standard per la comunicazione di debug
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Seriale 2 usata per connettersi al sensore MH-Z19B

  Wire.begin();                                 // Inizializzazione I2C usata per SHT20
  shtBegin();                                   // Inizializzazione sensore SHT20

  byte buf[BL] = CMD_ABC_OFF;                   // We are setting Automatic Baseline Correction to OFF
  sendCmdToMhz19(buf);
  cmdSent = false;                              // ready to process new commands on MH-Z19B (to be changed, absorbed by state machine)
}

void loop() { //Choose Serial1 or Serial2 as required
  switch (stato_macchina) {
    case (INIT):                                    // Initial status
      {
        Serial.println("STATUS: INIT");
      }
      stato_macchina = MEASURING_SHT_20;
      break;

    case (MEASURING_SHT_20):                    // Status used for collecting measurements from SHT20
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

    case (REQUEST_MH_Z19B):                     // Status used for requesting measurements from MH-Z19B
      {
        Serial.println("STATUS: REQUEST_MH_Z19B");
        byte buf[BL] = CMD_READCO2;
        sendCmdToMhz19(buf);
        stato_macchina = RECEIVING_MH_Z19B;
        Serial.println("STATUS: RECEIVING_MH_Z19B");
      }
      break;

    case (RECEIVING_MH_Z19B):                   // Status used for waiting measurements from MH-Z19B
      {
        while ( cmdSent && Serial2.available() > 0 ) {
          resp[i] = int(Serial2.read());
          i++;
          if (i > 8) {
            i = 0;
            cmdSent = false;
            start = millis();
            stato_macchina = RECEIVED_MH_Z19B;
          }
        }
      }
      break;

    case (RECEIVED_MH_Z19B):                    // Status used to print measurements from MH-Z19B
      {
        Serial.println("STATUS: RECEIVED_MH_Z19B");
        Serial.print("Concentrazione CO2: ");
        Serial.println(resp[2] * 256 + resp[3]);
        //        Serial.print("Temp (fake): ");
        //        Serial.println((resp[4] - 32) * 5 / 9);
        stato_macchina = WIFI_CONNECTING;
      }
      break;

    case (WIFI_CONNECTING):                     // Status used to connect to a known Wi-Fi network
      {
        Serial.println("STATUS: WIFI_CONNECTING");
        WiFi.begin(ssid, password);
        int counter = 0;
        Serial.print("Connecting to ");
        Serial.print(ssid);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
        Serial.println("");
        stato_macchina = WIFI_CONNECTED;
      }
      break;

    case (WIFI_CONNECTED):                     // Status used when connection has been established to a Wi-Fi network
      {
        Serial.println("STATUS: WIFI_CONNECTED");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        stato_macchina = NTP_GET_TIME;
      }
      break;

    case (NTP_GET_TIME):                  // Status used to init NTP client
      {
        Serial.println("STATUS: NTP_GET_TIME");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        stato_macchina = NTP_GOT_TIME;
      }
      break;


    case (NTP_GOT_TIME):                  // Status used to get time from NTP server
      {
        Serial.println("STATUS: NTP_GOT_TIME");
        // Init and get the time
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
          Serial.println("*** Failed to obtain time");
          stato_macchina = NTP_GET_TIME;
          return;
        } else {
          Serial.println("*** Got time");
          char buf[15];
          strftime(buf, 15, "%Y%m%d%H%M%S", &timeinfo);
          orario = (String) buf;
          Serial.println("Time:" + orario);
          stato_macchina = SERVER_CONNECTING;
        }
        stato_macchina = WIFI_DISCONNECT;
      }
      break;


    case (SERVER_CONNECTING):                  // Status used to start connecting to the server that collects measurements
      {
        Serial.println("STATUS: SERVER_CONNECTING");
        //connect(server);
        stato_macchina = WIFI_DISCONNECT;
      }

      break;

    case (SERVER_CONNECTED):                  // Status used when connection is done to the server that collects measurements
      {
        Serial.println("STATUS: SERVER_CONNECTED");
        stato_macchina = SENDING_GET_REQUEST;
        stato_macchina = WIFI_DISCONNECT;
      }
      break;

    case (SENDING_GET_REQUEST):               // Status used to send data to the server that collects measurements
      {
        Serial.println("STATUS: SENDING_GET_REQUEST");
        //sendGetRequest(server, resource);
        stato_macchina = WIFI_DISCONNECT;
      }
      break;

    case (SENT_GET_REQUEST):               // Status used when completed sending data to the server that collects measurements
      {
        Serial.println("STATUS: SENT_GET_REQUEST");
        //stato_macchina = WAITING_RESPONSE;
        stato_macchina = WIFI_DISCONNECT;
      }
      break;

    case (WAITING_RESPONSE):               // Status used when waiting for a response from the server that collects measurements
      {
        Serial.println("STATUS: WAITING_RESPONSE");
        //skipResponseHeaders();
        stato_macchina = WIFI_DISCONNECT;
      }
      break;

    case (RESPONSE_OK):               // Status used when response received from the server that collects measurements is OK
      {
        Serial.println("STATUS: RESPONSE_OK");
        //Serial.println(response);
        stato_macchina = WIFI_DISCONNECT;
      }
      break;

    case (WIFI_DISCONNECT):                     // Status used when disconnecting from the Wi-Fi network
      {
        Serial.println("STATUS: WIFI_DISCONNECT");
        WiFi.disconnect();
        stato_macchina = STANDBY;
      }
      break;

    case (STANDBY):                     // Status used when MCU goes to standby (actually this can be better, by using deep sleep and so on)
      {
        if (standby) {
          if (millis() - start > (1000 * NSEC) ) {
            standby = false;
            stato_macchina = MEASURING_SHT_20;
          }
        } else {
          Serial.println("STATUS: STANDBY");
          standby = true;
        }
      }
      break;

    case (ERRORE):                    // status used in case of error
      {
        Serial.println("STATUS: ERRORE");
        if (WiFi.status() == WL_CONNECTED) {
          stato_macchina = STANDBY;
        } else {
          Serial.println("Not connected to WiFi");
        }
      }
      break;

    default:
      break;
  }
}

void sendCmdToMhz19(byte *buf) {
  //  Serial.print("Bytes available for write in Serial2: "),
  //               Serial.println(Serial2.availableForWrite());
  Serial2.write(buf, BL);
  cmdSent = true;
}


char getCheckSum(char *packet)
{
  char i, checksum;
  for ( i = 1; i < 8; i++)
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

  if (config != 0xFF) {
    return true;
  }
  else {
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
  tempC = shtTemperature();
  RH = shtHumidity();

  float es = 0.6108 * exp(17.27 * tempC / (tempC + 237.3));
  float ae = RH / 100 * es;
  vpd_kPa = es - ae;

  return vpd_kPa;
}

float shtDew_point()
{
  tempC = shtTemperature();
  RH = shtHumidity();

  float tem = -1.0 * tempC;
  float esdp = 6.112 * exp(-1.0 * 17.67 * tem / (243.5 - tem));
  float ed = RH / 100.0 * esdp;
  float eln = log(ed / 6.112);
  dew_pointC = -243.5 * eln / (eln - 17.67 );

  return dew_pointC;
}

bool shtBegin()
{
  _address = SHT20_I2C;
  _resolution = SHT20_RESOLUTION_12BITS;
  _i2cPort = &Wire;


  return shtConnected();
}

void shtMeasure_all()
{
  // also measures temp/humidity
  shtVpd();
  shtDew_point();
}

bool connectToServer(const char* hostName) {
  stato_macchina = SERVER_CONNECTING;
  Serial.print("Connecting to ");
  Serial.println(hostName);

  bool ok = client.connect(hostName, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  stato_macchina = ok ? SERVER_CONNECTED : ERRORE;
  return ok;
}

bool sendGetRequest(const char* host, const char* resource) {
  stato_macchina = SENDING_GET_REQUEST;
  Serial.print("GET ");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.0");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();

  stato_macchina = SENT_GET_REQUEST;

  return true;
}

bool skipResponseHeaders() {

  stato_macchina = WAITING_RESPONSE;

  //HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (ok) {
    client.readBytes(response, 800);
    stato_macchina = RESPONSE_OK;
  } else {
    Serial.println("No response or invalid response!");
    stato_macchina = ERRORE;
  }
  return ok;
}
