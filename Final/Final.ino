/*############################################################################ INITS ############################################################################*/

#include <SPI.h>
#include <MFRC522.h>
//#include <math.h>

#include <LiquidCrystal.h>

////-- for esp8266 setup --////

#include "ESP8266.h"
#include <SoftwareSerial.h>


/*############################################################################ MACROS ############################################################################*/

#define UNBLOCK 1
//#define UNBLOCK 0



////-- for card mang--////
#define RA 2
#define CREDIT 6
#define BLOCKED 10
#define PINZ A1
#define PINGR A2
#define SS_PIN 10
#define RST_PIN 9

////-- for esp8266 setup --////

#define SSID        "Redmi"
#define PASSWORD    "leolindo"
#define HOST_NAME   "192.168.43.178"
#define HOST_PORT   (6969)


/*############################################################################ SETS ############################################################################*/

LiquidCrystal lcd(8, 15, 7, 6, 5, 4);

MFRC522 mfrc522(SS_PIN, RST_PIN);
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
  if (wifi.joinAP(SSID, PASSWORD)) {
    //Serial.print("Join AP success\r\n");
    //Serial.print("IP:");
    //Serial.println( wifi.getLocalIP().c_str());
  } else {
    //Serial.print("Join AP failure\r\n");
  }
  if (wifi.disableMUX()) {
    //Serial.print("single ok\r\n");
  } else {
    //Serial.print("single err\r\n");
  }

  Serial.print("setup end\r\n");
  lcd.clear();
  lcd.print(" Aprox. o Card");


}


void loop() {

  //////////////////////////////// Look for new cards /////////////////////////////////////
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    //Serial.print("nnc:");
    //delay(1000);
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
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
    Serial.print("create tcp ok\r\n");
  } else {
    Serial.print("create tcp err\r\n");
  }
  wifi.send((const uint8_t*)hello, strlen(hello));
  delete [] hello;

  uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
    Serial.print("");
    for (uint32_t i = 0; i < len; i++) {
      Serial.print((char)buffer[i]);
      //            buff2[i] = (char)buffer[i];
    }
    Serial.print("\r\n");
  }
  String strBuff = (char*)buffer;
  Serial.print("\n\n\n\n");
  //Serial.print(strBuff);
  byte found = strBuff.indexOf('@');
  found++;
  //Serial.println(found);
  String cred = strBuff.substring(found, found + 5);
  int recharge = cred.toInt();
  //Serial.println(fncred);
  strBuff = "";
  cred = "";
  if (wifi.releaseTCP()) {
    Serial.print("release tcp ok\r\n");
  } else {
    Serial.print("release tcp err\r\n");
  }
  if (recharge) {
    readBlock(CREDIT, creditBack);
    int intCredit = atoi(creditBack);
    intCredit += recharge;
    itoa(intCredit, finalCred, 10);
    writeBlock(CREDIT, finalCred);

    readBlock(CREDIT, creditBack);

    printBlock(creditBack);
    resetAuth();
    lcd.clear();
    lcd.print("Recarga: ");
    lcd.print((float)recharge/100);
    delay (1000);
    lcd.clear();
    lcd.print(" Aprox. o Card");
    //lcd.print(recharge);

    return;
  }
  readBlock(BLOCKED,blockBack);
  byte isBlock = atoi(blockBack);
  if (isBlock){
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
    //float fltCred = (float)intCredit;
    //fltCred /=100;
    lcd.clear();
    lcd.print("Cred:");
    lcd.print((float)intCredit/100);

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
  Serial.println("\nDesbloqueado\n");

}



/*################# CARD MANAGMENT AUX FUNCTS. ################*/


void printBlock(byte blockPRT[])
{
  for (int j = 0 ; j < 16 ; j++) //print the block contents
  {
    Serial.write (blockPRT[j]);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
  }

}


/*################# CARD MANAGMENTE MAIN FUNCTS. ################*/


/* ------------------ GRAVA ------------------*/

int writeBlock(int blockNumber, byte arrayAddress[])
{
  //this makes sure that we only write into data blocks. Every 4th block is a trailer block for the access/security info.
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector
  if (blockNumber > 2 && (blockNumber + 1) % 4 == 0)
  {
    Serial.print(blockNumber);  //block number is a trailer block (modulo 4); quit and send error code 2
    Serial.println(" is a trailer block:");
    return 2;
  }
  Serial.print("\n");
  Serial.print(blockNumber);
  Serial.println(" is a data block:");

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("PCD_Authenticate() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;//return "3" as error message
  }
  //it appears the authentication needs to be made before every block read/write within a specific sector.
  //If a different sector is being authenticated access to the previous one is lost.


  /*****************************************writing the block***********************************************************/

  status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);//valueBlockA is the block number, MIFARE_Write(block number (0-15), byte array containing 16 values, number of bytes in block (=16))
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("MIFARE_Write() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("\n\nblock was written!\n");
  return;
}


/* ------------------ ZERA ------------------*/


int readBlock(int blockNumber, byte arrayAddress[])
{
  int largestModulo4Number = blockNumber / 4 * 4;
  int trailerBlock = largestModulo4Number + 3; //determine trailer block for the sector

  /*****************************************authentication of the desired block for access***********************************************************/
  byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  //byte PCD_Authenticate(byte command, byte blockAddr, MIFARE_Key *key, Uid *uid);
  //this method is used to authenticate a certain block for writing or reading
  //command: See enumerations above -> PICC_CMD_MF_AUTH_KEY_A = 0x60 (=1100000),    // this command performs authentication with Key A
  //blockAddr is the number of the block from 0 to 15.
  //MIFARE_Key *key is a pointer to the MIFARE_Key struct defined above, this struct needs to be defined for each block. New cards have all A/B= FF FF FF FF FF FF
  //Uid *uid is a pointer to the UID struct that contains the user ID of the card.
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("PCD_Authenticate() failed (read): ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 3;//return "3" as error message
  }
  //it appears the authentication needs to be made before every block read/write within a specific sector.
  //If a different sector is being authenticated access to the previous one is lost.


  /*****************************************reading a block***********************************************************/

  byte buffersize = 18;//we need to define a variable with the read buffer size, since the MIFARE_Read method below needs a pointer to the variable that contains the size...
  status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);//&buffersize is a pointer to the buffersize variable; MIFARE_Read requires a pointer instead of just a number
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("MIFARE_read() failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return 4;//return "4" as error message
  }
  Serial.println("");
  return;
}

void resetAuth()
{
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}





