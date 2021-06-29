/*
 * There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.
 * 
 * U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
 * U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
 * U2UXD is unused and can be used for your projects.
 * 
*/

#define RXD2 16
#define TXD2 17
#define BL 9

int i=0;
byte resp[BL];

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: "+String(TX));
  Serial.println("Serial Rxd is on pin: "+String(RX));
  byte buf[BL]={0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  Serial.print("Bytes available for write in Serial2: "), 
  Serial.println(Serial2.availableForWrite());
  Serial2.write(buf, BL);  
}

void loop() { //Choose Serial1 or Serial2 as required
  while (Serial2.available()) {
    resp[i]=int(Serial2.read());
    Serial.print("Resp[ ");
    Serial.print(i);
    Serial.print(" ] = ");
    
    Serial.print(resp[i]);
    Serial.println(" ");
    if (i == 8){
      Serial.print("concentrazione CO2: ");
      Serial.println(resp[2]*256 + resp[3]);
      Serial.print("Temp: ");
      Serial.println((resp[4]-32)*5/9);
    }
    i++;
    if (i>9) i=0;
  }
}
