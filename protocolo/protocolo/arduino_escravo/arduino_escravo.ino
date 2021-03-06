#include "SoftwareSerial.h" //para falar com os radios Xbee
#include <Wire.h>

#define DEBUG
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) //returns the number os elements in byte array

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

SoftwareSerial Xbee(2, 3);
uint16_t bytes_read = 0;

/* LMPT RF Protocol */
/* Preamble - High|Preamble - Low|Version|Type|Size|Master ID|Slave ID|Data1|Data2|Data3|Data4|Checksum(XOR - 8) */
uint8_t KNOCK[] = { 0xAF, 0xB2, 0x01, 0x01, 0x0C, 0x01, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x10 };
uint8_t ACK[] = { 0xAF, 0xB2, 0x01, 0x01, 0x0C, 0x01, 0x00,0x0E, 0x0F, 0x00, 0x00, 0x00 };
uint8_t A_READ[] = { 0xAF, 0xB2, 0x01, 0x02, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t A_WRITE[] = { 0xAF, 0xB2, 0x01, 0x03, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t D_READ[] = { 0xAF, 0xB2, 0x01, 0x04, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t D_WRITE[] = { 0xAF, 0xB2, 0x01, 0x05, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t ADS_READ[] = { 0xAF, 0xB2, 0x01, 0x06, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t MESTRE_ID = 0x01;
const uint8_t THIS_SLAVE_ID = 0x03;

const uint8_t MAX_PROTOCOL_LENGTH = 12;
uint8_t rx_data_xbee[12];
int checksum = 0;

void setup() {
    Serial.begin(115200);
    Xbee.begin(9600); //RX: Arduino pin 2, XBee pin DOUT. TX: Arduino pin 3, XBee pin DIN;
}

void loop()
{
    memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
    bytes_read = 0;
    while (bytes_read < MAX_PROTOCOL_LENGTH)
    {
        if (Xbee.available() > 0)

        {
            if (bytes_read > 0) {
                rx_data_xbee[bytes_read] = Xbee.read();
                Serial.write(rx_data_xbee[bytes_read]);
                bytes_read++;
            }

            if (Xbee.read() == 0xAF && bytes_read == 0) {
                rx_data_xbee[0] = 0xAF;
                Serial.write(rx_data_xbee[bytes_read]);
                bytes_read = 1;
            }
        }
    }
    checksum = 0;
#ifdef DEBUG
    for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++) {
        Serial.write(rx_data_xbee[i]);
    }
#endif

    for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++) {
        checksum ^= rx_data_xbee[i];
    }
    if (checksum == rx_data_xbee[MAX_PROTOCOL_LENGTH - 1]) {
#ifdef DEBUG
        Serial.println("DEBUG:VALID_PKG_RCV");
#endif

            
            //dados enviados pelo mestre
            if (memcmp(rx_data_xbee, KNOCK, MAX_PROTOCOL_LENGTH) == 0) {
                Serial.println("DEBUG:RECEIVED KNOCK");
                ACK[9] = MESTRE_ID;
                ACK[10] = THIS_SLAVE_ID;

                //checksum xor 8
                int local_checksum = 0;
                for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++)
                    local_checksum ^= ACK[i];

                ACK[MAX_PROTOCOL_LENGTH - 1] = local_checksum;
                Xbee.write(ACK, sizeof(ACK));
                bytes_read = 0;
                memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
            }
            else {
                if (rx_data_xbee[6] == THIS_SLAVE_ID) {
            
                    bytes_read = 0;
                    memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
                }
                else {
                    Serial.print("PKG_DISCARDED: PACKAGE TO OTHER SLAVE:");
                    Serial.write(rx_data_xbee[6]);
                    Serial.println();
                    Xbee.flush();
                    bytes_read = 0;
                    memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
                }
            
            }

    }
    else {
        Serial.println("DEBUG: INVALID_CHECKSUM");
        Xbee.flush();
        bytes_read = 0;
        memset(rx_data_xbee, 0, sizeof(rx_data_xbee));
    }
}


