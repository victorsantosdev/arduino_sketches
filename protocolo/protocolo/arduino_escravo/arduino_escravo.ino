#include "SoftwareSerial.h" //para falar com os radios Xbee
#include <Wire.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) //returns the number os elements in byte array

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

SoftwareSerial Xbee(2,3);
uint16_t bytes_read = 0 ;

/* protocolo */
uint8_t KNOCK[] = { 0xAF, 0x00, 0x00, 0x00, 0x01,0x0A, 0x0B, 0x0C, 0x0D, 0xAE };
uint8_t ACK[] = { 0xAF, 0x00, 0x00, 0x00, 0x01, 0x0E, 0x0F, 0x00, 0x00, 0x00 };

const uint8_t MESTRE_ID = 0x01;
const uint8_t THIS_SLAVE_ID = 0x02;

const uint8_t MAX_PROTOCOLO_LENGTH = 10;
uint8_t rx_data_xbee[10];

void setup() {
    Serial.begin(115200);
    Xbee.begin(9600); //RX: Arduino pin 2, XBee pin DOUT. TX: Arduino pin 3, XBee pin DIN;
}

void loop()
{
 if (Xbee.available() > 0)
    {
       //read a byte and store it ....
       rx_data_xbee[bytes_read] = Xbee.read(); // Then: Get them.
       Serial.write(rx_data_xbee[bytes_read]);
       bytes_read ++;
    
if (bytes_read >= 10) {
 //dados enviados pelo mestre
 if ((rx_data_xbee[0] == KNOCK[0]) && (rx_data_xbee[1] == KNOCK[1]) && (rx_data_xbee[2] == KNOCK[2]) && (rx_data_xbee[3] == KNOCK[3])
      && (rx_data_xbee[4] == KNOCK[4]) && (rx_data_xbee[5] == KNOCK[5]) && (rx_data_xbee[6] == KNOCK[6])
      && (rx_data_xbee[7] == KNOCK[7])  && (rx_data_xbee[8] == KNOCK[8]) && (rx_data_xbee[9] == KNOCK[9])    ) {

      //Serial.println("knock recebido");
      ACK[7] = MESTRE_ID;
      ACK[8] = THIS_SLAVE_ID;

      //checksum xor 8
      int checksum = 0;
      for (int i = 0; i < (MAX_PROTOCOLO_LENGTH - 1); i++)
        checksum ^= ACK[i];
      
      ACK[MAX_PROTOCOLO_LENGTH - 1] = checksum;
      
       Xbee.write(ACK, sizeof(ACK));
      memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
      bytes_read = 0;
    }
    else{ 
      bytes_read = 0;
      memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
    }
      
    
 }
 
 }}
 
