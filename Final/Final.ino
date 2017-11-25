/*############################################################################ INITS ############################################################################*/

#include <SPI.h>

#include <MFRC522.h>

#include <LiquidCrystal.h>

#include "ESP8266.h"
#include <SoftwareSerial.h>


/*############################################################################ MACROS ############################################################################*/



////-- for card mang--////
#define RA 2
#define CREDIT 6
#define BLOCKED 10

////-- for esp8266 setup --////

#define SSID        "Redmi"
#define PASSWORD    "leolindo"
#define HOST_NAME   "192.168.43.178"
#define HOST_PORT   (6969)


/*############################################################################ SETS ############################################################################*/

LiquidCrystal lcd(8, 15, 7, 6, 5, 4);

MFRC522 mfrc522(10, 9);
MFRC522::MIFARE_Key key;

SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);


/*############################################################################ GLOBALS ############################################################################*/

byte blockLuch[2] = {"1"};
byte blockBack[18];
byte raBack[18]; // RA array with buffer to transmit to the card
byte creditBack[18]; // check if card is blocked
byte zerar[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte finalCred[16] = "";
/*############################################################################ SETUP ############################################################################*/



void setup() {

    SPI.begin();
    mfrc522.PCD_Init();
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF; // factory key
    }
    ////-- esp8266 setup --////
    wifi.joinAP(SSID, PASSWORD);
    wifi.disableMUX();
    lcd.clear();
    lcd.print(" Aprox. o Card");


}


void loop() {

    //////////////////////////////// Look for new cards /////////////////////////////////////
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
        return;

    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    readBlock(RA, raBack);

    char* hello = new char[150];

    char strIni[] = "GET /arduino/getdata.php?ra=";
    
    char strFinal[] =  " HTTP/1.1\r\nHost: 192.168.43.178\r\n\r\n";
    
    strcat(hello, strIni);
    
    strcat(hello, raBack);
    
    strcat(hello, strFinal);
    
    uint8_t buffer[300] = {0};
    
    wifi.createTCP(HOST_NAME, HOST_PORT);
    
    wifi.send((const uint8_t*)hello, strlen(hello));
    
    delete [] hello;

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);

    String strBuff = (char*)buffer;
    
    byte found = strBuff.indexOf('@');
    
    found++;
    
    String cred = strBuff.substring(found, found + 5);
    
    int recharge = cred.toInt();
    
    strBuff = "";
    
    cred = "";
    
    wifi.releaseTCP();
    
    if (recharge) {
    
        readBlock(CREDIT, creditBack);
    
        int intCredit = atoi(creditBack);
    
        intCredit += recharge;
    
        itoa(intCredit, finalCred, 10);
    
        writeBlock(CREDIT, finalCred);

        readBlock(CREDIT, creditBack);

        resetAuth();

        lcd.clear();

        lcd.print("Recarga: ");

        lcd.print((float)recharge / 100);

        delay (1000);

        lcd.clear();

        lcd.print(" Aprox. o Card");

        return;
    }

    readBlock(BLOCKED, blockBack);

    byte isBlock = atoi(blockBack);

    if (isBlock) {

        lcd.clear();

        lcd.print("   BLOCKED ");

        delay(1000);

        lcd.clear();

        lcd.print(" Aprox. o Card");

        resetAuth();

        return;
    }

    readBlock(CREDIT, creditBack);

    int intCredit = atoi(creditBack);

    int checkCred = intCredit;

    if ((checkCred -= 350) > 0) {

        intCredit -= 350;

        itoa(intCredit, finalCred, 10);

        writeBlock(CREDIT, finalCred);
        lcd.clear();
        lcd.print("Cred:");
        lcd.print((float)intCredit / 100);

    }
    else {
        lcd.clear();
        lcd.print("  Sem Saldo");
        delay(1000);
        lcd.clear();
        lcd.print(" Aprox. o Card");
        resetAuth();
        return;
    }


    writeBlock(BLOCKED, blockLuch);
    delay(1000);
    lcd.clear();
    lcd.print(" Aprox. o Card");
    resetAuth();

}



void unblock()
{
    writeBlock(BLOCKED, zerar);

}



/*################# CARD MANAGMENT AUX FUNCTS. ################*/


/*################# CARD MANAGMENTE MAIN FUNCTS. ################*/


/* ------------------ GRAVA ------------------*/

int writeBlock(int blockNumber, byte arrayAddress[])
{
    int largestModulo4Number = blockNumber / 4 * 4;
    int trailerBlock = largestModulo4Number + 3; 
    if (blockNumber > 2 && (blockNumber + 1) % 4 == 0)
    {
        return 2;
    }

    /*****************************************authentication of the desired block for access***********************************************************/
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

    if (status != MFRC522::STATUS_OK)
    {
        return 3;
    }

    /*****************************************writing the block***********************************************************/

    status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
    if (status != MFRC522::STATUS_OK)
    {
        return 4;
    }
    return;
}


/* ------------------ ZERA ------------------*/


int readBlock(int blockNumber, byte arrayAddress[])
{
    int largestModulo4Number = blockNumber / 4 * 4;
    int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector

    /*****************************************authentication of the desired block for access***********************************************************/
    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    
    if (status != MFRC522::STATUS_OK)
    {
        return 3;//return "3" as error message
    }


    /*****************************************reading a block***********************************************************/

    byte buffersize = 18;
    status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
    if (status != MFRC522::STATUS_OK)
    {
        return 4;
    }
    return;
}

void resetAuth()
{
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
}





