/*############################################################################ INITS ############################################################################*/

#include <SPI.h>
#include <MFRC522.h>
#include <math.h>


/*############################################################################ MACROS ############################################################################*/


#define RA 2
#define CREDIT 6
#define BLOCKED 10
#define PINZ A1
#define PINGR A2
#define SS_PIN 10
#define RST_PIN 9


/*############################################################################ SETS ############################################################################*/


MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;


/*############################################################################ GLOBALS ############################################################################*/

byte blockLuch[16] = {"1"};
byte blockBack[18];
byte raBack[18]; // RA array with buffer to transmit to the card
byte creditBack[18]; // check if card is blocked
byte newRA[16] = {"RA:1858769"}; // Sample
byte cardUID[8]; // UID
byte zerar[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte setCredit[16] = {"13350"};


/*############################################################################ SETUP ############################################################################*/


void setup()
{
    Serial.begin(9600);   // Initiate a serial communication
    pinMode(PINZ, INPUT); // set both pins
    pinMode(PINGR, INPUT);
    SPI.begin();      // Initiate  SPI bus
    mfrc522.PCD_Init();   // Initiate RFID module
    Serial.println("Aproxime o Cartão\n"); // Init Message
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF; // factory key
    }
}


/*############################################################################ MAIN ############################################################################*/

void loop()  // default : reads the card and show UID and whats in the block 2 (for now)
{
    //////////////////////////////// INITS ////////////////////////////////////

    byte* finalCred = new byte[16];

    
    for (int j = 0 ; j < 16 ; j++) //print the block contents
    {
        finalCred[j]=0;//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
    }
    bool modo_grava = digitalRead(PINGR); //sets pins
    bool modo_unblock = digitalRead(PINZ);

    //////////////////////////////// CHECK PRESSED BUTTONS /////////////////////////////////

    if (modo_unblock != 0)
    {
        Serial.println("Modo zerar selecionado");
        while (digitalRead(PINZ) == 1) {}
        delay(3000);
        unblock();
        resetAuth();
        return;

    }
    if (modo_grava != 0)
    {
        Serial.println("Modo gravar selecionado");
        while (digitalRead(PINGR) == 1) {}
        delay(3000);
        grava();
        resetAuth();
        return;

    }

    //////////////////////////////// Look for new cards /////////////////////////////////////
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
        //Serial.print("nnc:");
        delay(2000);
        return;

    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    //////////////////////////////// Show UID on serial monitor ////////////////////////////////
    Serial.print("Modo Leitura\n\nUID tag :\n");
    for (int i = 0; i < mfrc522.uid.size; i++)
    {
        cardUID[i] = mfrc522.uid.uidByte[i];
        Serial.print(cardUID [i] < 0x10 ? " 0" : " ");
        Serial.print(cardUID[i], HEX);
    }
    
    ///////////////////////////////////////// block check //////////////////////////////////////////////

    if(isBlocked())
    {
        Serial.print("Blocked");
        resetAuth();
        return;
    }
    
    ////////////////////////////////////////// Debit //////////////////////////////////////////////////

    readBlock(RA, raBack); // shows RA of the card
    
    readBlock(CREDIT,creditBack);

    Serial.print("\nRA:\n ");
    
    printBlock(raBack);
    
    Serial.print("\nCréditos:\n ");
    
    printBlock(creditBack);
    
    int intCred = getCredit();

    Serial.println("\n");
    
    Serial.println(intCred);
    
    Serial.println("\n");

    itoa(intCred,finalCred,10);

    for (int j = 0 ; j < 16 ; j++) //print the block contents
    {
        Serial.write (finalCred[j]);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
    }

    writeBlock(CREDIT,finalCred);
    
    readBlock(CREDIT,creditBack);
    
    printBlock(creditBack);

    writeBlock(BLOCKED,blockLuch);
    
    resetAuth();
    
    delete [] finalCred;

}


/*################# CARD MANAGMENT AUX FUNCTS. ################*/


void printBlock(byte ra[])
{
    for (int j = 0 ; j < 16 ; j++) //print the block contents
    {
        Serial.write (ra[j]);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
    }

}




/* ------------------ GRAVA ------------------*/
void grava()
{
    while ( ! mfrc522.PICC_IsNewCardPresent())
    {
        //Serial.println("\nncl");

    }
    if ( ! mfrc522.PICC_ReadCardSerial())    return;

    writeBlock(RA, newRA);
    Serial.println("\nGravado!");

}

/*------------------ZERA------------------*/
void unblock()
{
    while ( ! mfrc522.PICC_IsNewCardPresent())
    {
        //Serial.println("\nncl");

    }
    if ( ! mfrc522.PICC_ReadCardSerial())    return;

    writeBlock(BLOCKED, zerar);
    Serial.println("\nDesbloqueado\n");

}

/*------------------DEBIT------------------*/

int getCredit()
{
    int intCredit =0;
    intCredit = atoi(creditBack);
    intCredit -=350;
    return intCredit;



}

/*------------------DEBIT------------------*/

void debit() {}


/*------------------FLAG : BLOCKED------------------*/
bool isBlocked()
{
    readBlock(BLOCKED, blockBack);
    for (int i=0; i<16; i++)
    {
        if(blockBack[i] != 0)
        {
            return true;
        }

    }
    return false;




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


