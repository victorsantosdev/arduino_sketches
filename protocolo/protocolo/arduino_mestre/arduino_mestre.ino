#include "SoftwareSerial.h" //para falar com os radios Xbee
#include <Wire.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0])) //returns the number os elements in byte array

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

SoftwareSerial Xbee(2, 3);

/* LMPT RF Protocol */
/* Preamble - High|Preamble - Low|Version|Type|Size|Master ID|Slave ID|Data1|Data2|Data3|Data4|Checksum(XOR - 8) */
uint8_t KNOCK[] = { 0xAF, 0xB2, 0x01, 0x01, 0x0C, 0x01, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x10 };
uint8_t ACK[] = { 0xAF, 0xB2, 0x01, 0x01, 0x0C, 0x01, 0x00, 0x0E, 0x0F, 0x00, 0x00, 0x00 };
uint8_t A_READ[] = { 0xAF, 0xB2, 0x01, 0x02, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t A_WRITE[] = { 0xAF, 0xB2, 0x01, 0x03, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t D_READ[] = { 0xAF, 0xB2, 0x01, 0x04, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t D_WRITE[] = { 0xAF, 0xB2, 0x01, 0x05, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t ADS_READ[] = { 0xAF, 0xB2, 0x01, 0x06, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t THIS_MASTER_ID = 1;
const uint8_t MAX_PROTOCOL_LENGTH = 12;
uint8_t rx_data_pc[50];
uint8_t rx_data_xbee[MAX_PROTOCOL_LENGTH];
int checksum = 0;
uint8_t bytes_read = 0;

/* function prototypes*/
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
    /* this section handles incoming messages from PC */
    bytes_read = 0;
    memset(rx_data_pc, 0, sizeof(rx_data_pc));

    while (bytes_read < MAX_PROTOCOL_LENGTH)
    {
        if (Serial.available() > 0)
        {
            if (bytes_read > 0) {
                rx_data_pc[bytes_read] = Serial.read();
                Serial.write(rx_data_pc[bytes_read]); //debug
                bytes_read++;
            }

            if (Serial.read() == 0xAF && bytes_read == 0) {
                rx_data_pc[0] = 0xAF;
                Serial.write(rx_data_pc[bytes_read]); //debug
                bytes_read = 1;
            }
        }
    }
    /* after collect an entire packet, lets evaluate the checksum*/
    checksum = 0;
#ifdef DEBUG
    for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++) {
        Serial.write(rx_data_pc[i]);
    }
#endif

    for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++) {
        checksum ^= rx_data_pc[i];
    }

    if (checksum == rx_data_pc[MAX_PROTOCOL_LENGTH - 1]) {
#ifdef DEBUG
        Serial.println("DEBUG:VALID_PKG_RCV");
#endif

        /* disables the Serial from receiving garbage during the handling of a received package */
        Serial.flush();
        Serial.end();

        //dados do PC->Arduino mestre
        if (memcmp(rx_data_pc, KNOCK, MAX_PROTOCOL_LENGTH) == 0)
        {
#ifdef DEBUG
            Serial.println("SEND KNOCK");
#endif
            Xbee.write(KNOCK, sizeof(KNOCK));
        }
        else if (memcmp(rx_data_pc, A_READ, 6) == 0)
        {

#ifdef DEBUG
            Serial.println("SEND A_READ");
#endif

            Xbee.write(A_READ, sizeof(A_READ));
        }
        else if (memcmp(rx_data_pc, A_WRITE, 6) == 0)
        {
#ifdef DEBUG
            Serial.println("SEND A_WRITE");
#endif
            Xbee.write(A_WRITE, sizeof(A_WRITE));
        }
        else if (memcmp(rx_data_pc, D_READ, 6) == 0)
        {
#ifdef DEBUG
            Serial.println("SEND D_READ");
#endif
            Xbee.write(D_READ, sizeof(D_READ));
        }
        else if (memcmp(rx_data_pc, D_WRITE, 6) == 0)
        {
#ifdef DEBUG
            Serial.println("SEND D_WRITE");
#endif
            Xbee.write(ADS_READ, sizeof(ADS_READ));
        }
        else if (memcmp(rx_data_pc, ADS_READ, 6) == 0)
        {
#ifdef DEBUG
            Serial.println("SEND ADS_AREAD");
#endif
            Xbee.write(ADS_READ, sizeof(ADS_READ));
        }
        /* re-enables the serial to use in the xbee messages handling section*/
        Serial.begin(115200);

    }
    else {
        /* re-enables the serial to use in the xbee messages handling section*/
        Serial.begin(115200);
        Serial.println("DEBUG: INVALID CHECKSUM");
    }

    /* this section handles incoming messages from Xbee network*/
    bytes_read = 0;

    if (Xbee.available() > 0) {

        while (bytes_read < MAX_PROTOCOL_LENGTH)
        {
            rx_data_xbee[bytes_read] = Xbee.read();
#ifdef DEBUG
            Serial.write(rx_data_xbee[bytes_read]);
#endif
            bytes_read++;
        }

        /* disables the Xbee serial from receiving garbage during the handling of a received package */
        Xbee.end();
        checksum = 0;
        for (int i = 0; i < (MAX_PROTOCOL_LENGTH - 1); i++)
            checksum ^= rx_data_xbee[i];

        if (checksum == rx_data_xbee[MAX_PROTOCOL_LENGTH - 1]) {

            if (rx_data_xbee[5] == THIS_MASTER_ID) {

                if (memcmp(rx_data_xbee, ACK, 8) == 0)
                {
                    for (int i = 0; i < (MAX_PROTOCOL_LENGTH); i++)
                        Serial.write(rx_data_xbee[i]);
                }
            }
            else {
                Serial.println("PKG_DISCARDED: PACKAGE TO OTHER MASTER");
            }
        }
        else {
            Serial.println("PKG_DICARDED: CHECKSUM ERROR");
        }
    }
    //re-enables the radio after handling the received package
    Xbee.begin(9600);
    Xbee.flush();
}
