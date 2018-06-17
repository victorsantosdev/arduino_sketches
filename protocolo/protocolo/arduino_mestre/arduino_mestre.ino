#include "SoftwareSerial.h" //para falar com os radios Xbee
#include <Wire.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) //returns the number os elements in byte array

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

SoftwareSerial Xbee(2,3);

/* protocolo */
uint8_t KNOCK[] = { 0xAF, 0x00, 0x00, 0x00, 0x01,0x0A, 0x0B, 0x0C, 0x0D, 0xAE };
uint8_t ACK[] = { 0xAF, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x0F, 0x00, 0x00, 0x00 };

const uint8_t MAX_PROTOCOLO_LENGTH = 10;
uint8_t rx_data_pc[10];
uint8_t rx_data_xbee[10];

void send_knock(uint8_t addr);
void send_pc(uint8_t * cmd);


void send_pc(uint8_t * cmd) {
  Serial.write(cmd, sizeof(cmd));
}

void setup() {
    Xbee.begin(9600); //RX: Arduino pin 2, XBee pin DOUT. TX: Arduino pin 3, XBee pin DIN;
   Serial.begin(115200); //hardware uart
}

void loop()
{
uint16_t bytes_read = 0 ;
for (uint8_t i = 0; i < MAX_PROTOCOLO_LENGTH; i++) {
      rx_data_pc[i] = 0;
 } 
 while (bytes_read < MAX_PROTOCOLO_LENGTH)
 {
    if (Serial.available() > 0)
    {
       //read a byte and store it ....
       rx_data_pc[bytes_read] = Serial.read(); // Then: Get them.
       Serial.write(rx_data_pc[bytes_read]); //debug
       bytes_read++;
    }
 }

 //dados do PC->Arduino mestre
 if ((rx_data_pc[0] == KNOCK[0]) && (rx_data_pc[1] == KNOCK[1]) && (rx_data_pc[2] == KNOCK[2]) && (rx_data_pc[3] == KNOCK[3])
      && (rx_data_pc[4] == KNOCK[4]) && (rx_data_pc[5] == KNOCK[5]) && (rx_data_pc[6] == KNOCK[6])
      && (rx_data_pc[7] == KNOCK[7]) && (rx_data_pc[8] == KNOCK[8])&& (rx_data_pc[9] == KNOCK[9])
      ) {
      //Serial.println("send knock");
      Xbee.write(KNOCK, sizeof(KNOCK)); //debug
      memset(rx_data_pc, 0, sizeof(rx_data_pc));
    } else memset(rx_data_pc, 0, sizeof(rx_data_pc));
      
   if (Xbee.available() > 0)   {

  for (uint8_t i = 0; i < MAX_PROTOCOLO_LENGTH; i++) {
      rx_data_xbee[i] = 0;
  } 
  bytes_read = 0;
  while (bytes_read < MAX_PROTOCOLO_LENGTH)
 {
       //read a byte and store it ....
       rx_data_xbee[bytes_read] = Xbee.read(); // Then: Get them.
       Serial.write(rx_data_xbee[bytes_read]);
       bytes_read ++;
 }
 
 if ((rx_data_xbee[0] == ACK[0]) && (rx_data_xbee[1] == ACK[1]) && (rx_data_xbee[2] == ACK[2]) && (rx_data_xbee[3] == ACK[3])
      && (rx_data_xbee[4] == ACK[4]) && (rx_data_xbee[5] == ACK[5]) && (rx_data_xbee[6] == ACK[6])
  
      ) {
        uint16_t addr_recebido = rx_data_xbee[8] && 0xFF;
        //addr_recebido = ((rx_data_xbee[7] << 8) && 0xFF00) | (rx_data_xbee[8] && 0xFF); 
        Serial.println("Recebido ACK: ");
        Serial.write(addr_recebido);

      memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
    } else memset(rx_data_xbee, 0, sizeof(rx_data_xbee));

 }
}
