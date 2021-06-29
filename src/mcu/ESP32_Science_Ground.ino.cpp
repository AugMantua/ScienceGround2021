/*
   There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.

   U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
   U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
   U2UXD is unused and can be used for your projects.

*/

#define RXD2 16
#define TXD2 17
#define BL 9
#define NSEC 60000

// Set of commands for MH-Z19B
// last byte is checksum, calculated as negation of the sum of all bytes except FF
#define CMD_READCO2 {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79} // Read CO2 concentration
#define CMD_ZEROCAL {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78} // ZERO POINT CALIBRATION
#define CMD_SPANCAL {0xFF, 0x01, 0x88, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x9F} // SPAN POINT CALIBRATION (2000 ppm)
#define CMD_ABC_ON  {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE5} // ABC logic on (Automatic Baseline Correction)
#define CMD_ABC_OFF {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85} // ABC logic off (Automatic Baseline Correction)
#define CMD_RNGSET2 {0xFF, 0x01, 0x99, 0x07, 0xD0, 0x00, 0x00, 0x00, 0x8E} // Sensor detection range setting to 2000 ppm
#define CMD_RNGSET5 {0xFF, 0x01, 0x99, 0x13, 0x88, 0x00, 0x00, 0x00, 0xCA} // Sensor detection range setting to 5000 ppm



int i = 0;
byte resp[BL];

char getCheckSum(char *);
bool cmdSent = false;
long lastCommandSent = NSEC;

void sendCmdToMhz19(char *);
char getCheckSum(char *);

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));
  
  byte buf[BL] = CMD_ABC_OFF;
  sendCmdToMhz19(buf);
  cmdSent = false;
}

void loop() { //Choose Serial1 or Serial2 as required
  if ( !cmdSent && millis() - lastCommandSent >= NSEC ) {
    byte buf[BL] = CMD_READCO2;
    sendCmdToMhz19(buf);
  }
  while ( cmdSent && Serial2.available() > 0 ) {
    resp[i] = int(Serial2.read());
    Serial.print("Resp[ ");
    Serial.print(i);
    Serial.print(" ] = ");

    Serial.print(resp[i]);
    Serial.println(" ");
    if (i == 8) {
      Serial.print("concentrazione CO2: ");
      Serial.println(resp[2] * 256 + resp[3]);
      Serial.print("Temp (fake): ");
      Serial.println((resp[4] - 32) * 5 / 9);
    }
    i++;
    if (i > 8) {
      i = 0;
      cmdSent = false;
      lastCommandSent = millis();
    }
  }
}

void sendCmdToMhz19(byte *buf){
    Serial.print("Bytes available for write in Serial2: "),
                 Serial.println(Serial2.availableForWrite());
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
