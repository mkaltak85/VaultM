#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>             // SPI hardware library
#include <MFRC522.h>         // RFID library 
#include "Vault.h"           // My Vault library 

#define DEBUG 0 

// MFRC522 READER
#define RST_PIN         11 // Configurable, see typical pin layout above
#define SS_PIN          12 // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance
// string to identify account name and password in block 
#define TERMINATION_STRING ' '
// key to open Vault and allow to scan Password card 
byte LenVaultKey=7;
byte VaultKey[7]  = { 0x04, 0xC6, 0xA0, 0x52, 0x83, 0x56, 0x80};
// key to access every sector of Mifare 1K card 
byte LenSectorKey=6;
byte SectorKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } ;
// define account names here, 16 are supported for Mifare Classic 1K
char* MyAccounts[] = {
    "Pass 1 ",
    "Pass 2 ",
    "Pass 3 ",
    "Pass 4 ",
    "Pass 5 ",
    "Pass 6 ",
    "Pass 7 ",
    "Pass 8 ",
    "Pass 9 ",
    "Pass 10",
    "Pass 11",
    "Pass 12",
    "Pass 13",
    "Pass 14",
    "Pass 15",
    "Pass 16"
    };

// time vault should be visible on idle 
unsigned long IDLE_TIME_MAX = 10000;

// random number 
uint8_t irand;

uint8_t successRead; 

// ======================= TFT =========================
// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     8
#define TFT_RST    7  
#define TFT_DC     6
// must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Color Buttons
#define R_BTN 4
#define B_BTN 3
#define Y_BTN 2

// Password vault user interface, needs a handle to the tft 
// also sets layout, i.e. how many items should be shown per
// page, and how many passwords there are in total 
Vault *vault = new Vault( 8, 16 );

//*************************************************************
// SETUP routine 
//*************************************************************
void setup(void) {
  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();   // Init MFRC522
  rfid.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details

  // initialize buttons as pull-up resistor
  pinMode( R_BTN, INPUT_PULLUP );
  pinMode( Y_BTN, INPUT_PULLUP );
  pinMode( B_BTN, INPUT_PULLUP );
  
  delay(500);
  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  // set rotation to 90 degrees
  tft.setRotation( 1 );
  tft.fillScreen(ST7735_BLACK);

  // large block of text
  tft.fillScreen(ST7735_BLACK);
  delay(100);
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
}
//*************************************************************
// Main Loop 
//*************************************************************
void loop() {
  // Look for new cards
  if ( rfid.PICC_IsNewCardPresent()) {
     DisplayReadingCard();
     // Select one of the cards
     if ( ! rfid.PICC_ReadCardSerial()) { return; }

     successRead = checkID( VaultKey, LenVaultKey ); 
     // if card is unlock card open vault user interface
     if ( successRead ) { 
        OpenVault( );
        DisplayRightCard();
     } else { 
        DisplayWrongCard();
     } 
     tft.fillScreen(ST7735_BLACK);
  } else {
     MyScreenSaver();
  }
}


//*************************************************************
// user inter face
//*************************************************************
// shows vault on TFT 
void OpenVault( ) {
  unsigned long startMillis ;

  // initialize counters
  startMillis=millis(); // for loop

  // infinite loop
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  for(;;){
     byte Button_EXIT = digitalRead( R_BTN );
     byte Button_NEXT = digitalRead( Y_BTN );
     byte Button_SELECT = digitalRead( B_BTN );

     // next item 
     if ( Button_NEXT == LOW ){
        vault->nextAccount();
        // also reset counter
        startMillis=millis();
     }

     showVaultUI( );
     showAccountNumber( );
     scrollSelected( );

     // selects item todo: scan proper key card with passord
     if ( Button_SELECT == LOW ){
        showSelected( );
     }

     // if time exceeded or Red button pressed exit vault
     if ( millis() - startMillis >  IDLE_TIME_MAX || Button_EXIT == LOW ){
        break;
     }
     delay(100);
  }
  tft.setTextSize(1);
}
void showVaultUI( ) { 
   uint8_t w=68 , h=24; 
   int16_t x1,y1;
   uint16_t wi,he;

   tft.setTextColor(ST7735_WHITE);
   tft.setTextSize(1);
   // draw rectangles and names of accounts 
   byte j=0;
   for ( uint8_t i=4 ; i<tft.height()-h/2 ; i+=(h+4) ) {
     //           x  y  w, h
     tft.drawRect(4 , i, w+4, h, ST7735_WHITE);
     if ( vault->getAccount()<8 ) { 
        tft.setCursor(8, i+4);
        if ( vault->getPageFold() ) {
          tft.getTextBounds(MyAccounts[j], 8, i+4, &x1, &y1, &wi, &he);
          tft.fillRect(x1, y1, wi, he, ST7735_BLACK);
        }
        tft.print( MyAccounts[j] ) ;
     } else { 
        tft.setCursor(8, i+4);
        if ( vault->getPageFold() ) {
          tft.getTextBounds(MyAccounts[j], 8, i+4, &x1, &y1, &wi, &he);
          tft.fillRect(x1, y1, wi, he, ST7735_BLACK);
        }
        tft.print( MyAccounts[j+8] ) ;
     } 

     tft.drawRect(80, i, w+4, h, ST7735_WHITE);
     if ( vault->getAccount()<8 ) { 
        tft.setCursor(84, i+4);
        if ( vault->getPageFold() ) {
          tft.getTextBounds(MyAccounts[j+4], 84, i+4, &x1, &y1, &wi, &he);
          tft.fillRect(x1, y1, wi, he, ST7735_BLACK);
        }
        tft.print( MyAccounts[j+4] ) ;
     } else { 
        tft.setCursor(84, i+4);
        if ( vault->getPageFold() ) {
          tft.getTextBounds(MyAccounts[j+4], 84, i+4, &x1, &y1, &wi, &he);
          tft.fillRect(x1, y1, wi, he, ST7735_BLACK);
        }
        tft.print( MyAccounts[j+12] ) ;
     } 
     j++;
   }
   if ( vault->getPageFold() ) {
      vault->setPageFold( false );
   }
}
void showAccountNumber( ) { 
   tft.fillRect(6, 120, 12, 8, ST7735_BLACK);
   tft.setTextColor(ST7735_WHITE);
   tft.setCursor(6, 120);
   tft.setTextSize(1);
   tft.print(vault->getAccount()+1);
}
// draws a rectangle arround selected item
void scrollSelected( ) { 
   uint8_t row, col;
   // column
   col = vault->getAccount()/4 ;
   col = col%2;
   // row
   row = vault->getAccount()%4 ;
   tft.drawRect(4+col*76 , 4+row*28, 72, 24, ST7735_RED);

}
// prints selected Password
void showSelected( ) { 
  unsigned long startMillis ;
  // initialize counters
  startMillis=millis(); // for loop
  // infinite loop
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_CYAN);
  tft.setTextSize(2);
  tft.setCursor(0,50);
  tft.println("Scan key card");

  for(;;){
     byte Button_EXIT = digitalRead( R_BTN );

     // Look for new cards
     if ( rfid.PICC_IsNewCardPresent()) {
        DisplayReadingCard();
        // Select one of the cards
        if ( rfid.PICC_ReadCardSerial()) { 
           startMillis=millis(); // reset counter
           // read block from card
           ReadSector( vault->getAccount() ); 
        }
     }
     // if time exceeded or Red button pressed exit vault
     if ( millis() - startMillis >  IDLE_TIME_MAX || Button_EXIT == LOW ){
        break;
     }
  }
  tft.fillScreen(ST7735_BLACK);
}

//*************************************************************
// Reader specific functions 
//*************************************************************
// compares card to passed ID
boolean checkID( byte *key , byte length) {
  boolean match ;
  match = false;
  // indicate on tft that a card is read
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // prepare key from unlock card 
  byte nuidPICC[rfid.uid.size];
  // only two different card types are supported
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL && 
      piccType != MFRC522::PICC_TYPE_MIFARE_1K ) {
      match = false;
      for ( byte i = 0; i < rfid.uid.size; i++) {
        nuidPICC[i] = 0xFF;
      }
  } else {
     // Store NUID into nuidPICC array
     for ( byte i = 0; i < rfid.uid.size; i++) {
       nuidPICC[i] = rfid.uid.uidByte[i];
     }
  }
#define DEBUG 1
#if DEBUG
  for ( byte i = 0 ; i<rfid.uid.size; i++ ){
     Serial.println( nuidPICC[i],HEX );
  }
#endif
#define DEBUG 0
  match = compareTwoCards( nuidPICC, key, rfid.uid.size  );

  // Halt PICC
  rfid.PICC_HaltA();

  // if card is correct make pacman green
  if ( match ) { 
     DisplayRightCard();
  } else { 
     DisplayWrongCard();
  }
  // check ID card
  return match;
}
// compares two keys 
boolean compareTwoCards( byte *a, byte *b, byte length ){
  boolean match;

  // initialize to false in the beginning
  match = true; 
  // compare cards 
  for ( byte i=0; i<length; i++ ){
     if( b[i] != a[i] ) {
        match = false;
        break;
     } 
  }
  return match;
}
// reads a sector of Mifare card
void ReadSector( byte sector ) { 
  // Prepare key - for sector 
  MFRC522::MIFARE_Key key;

  //some variables we need
  MFRC522::StatusCode status;
  // this is the block where info is stored 
  // it is assumed that the account name is stored on first 
  // block of sector in the format "ACCOUNT PASSWORD " 
  byte block = sector*4+1;
  byte len=18;
  byte buffer1[len], buffer2[len], buffer[2*len]; 

  // set key to access sector of Mifare Card 
  // we assume Key A is used and is the same for all sectors
  for ( byte i = 0; i < 6 ; i++ ) { 
     key.keyByte[i] = SectorKey[i]; 
  }

#if DEBUG 
  Serial.print(F("**Card Detected:** Block: #"));
  Serial.println( block  );
#endif 
  // empty buffer
  for ( byte i=0 ; i<len; i++ ){
     buffer1[i]=0x20;
     buffer2[i]=0x20;
     buffer[i]=0x20;
     buffer[i+len]=0x20;
  }

#if DEBUG 
  rfid.PICC_DumpDetailsToSerial(&(rfid.uid)); //dump some details about the card
  rfid.PICC_DumpMifareClassicSectorToSerial(&(rfid.uid), &key, sector);
#endif

  // first use authentication key to access block
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                    block, 
                                    &key, 
                                    &(rfid.uid));
  // continue only if status is OK
  if (status != MFRC522::STATUS_OK) {
     DisplayWrongCard();
     return;
  }
  // access block and pass buffer for reading
  status = rfid.MIFARE_Read(block, buffer1, &len);
  // check status again and continue only if its OK, otherwise write error
  if (status != MFRC522::STATUS_OK) {
    return;
  }
#if DEBUG 
  //print block read to serial 
  Serial.print(" LENGTHA: ");
  Serial.print( len);
  Serial.println(" ");
#endif
  // first use authentication key to access block
  block+=1;
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                    block, 
                                    &key, 
                                    &(rfid.uid));
  // continue only if status is OK
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  // access block and pass buffer for reading
  status = rfid.MIFARE_Read(block, buffer2, &len);
  // check status again and continue only if its OK, otherwise write error
  if (status != MFRC522::STATUS_OK) {
    return;
  }
#if DEBUG 
  //print block read to serial 
  Serial.print(" LENGTHB: ");
  Serial.print( len);
  Serial.println(" ");
#endif

  for ( byte i =0 ; i<16 ; i++ ) { 
     buffer[i] = buffer1[ i ]; 
     buffer[i+16] = buffer2[ i ]; 
  }
 
  // print info to TFT 
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0,0);
  tft.fillRect(0, 0, 32, 16, ST7735_BLACK);
  tft.setTextColor(ST7735_GREEN);
  tft.print(MyAccounts[sector]); 
  // print header to TFT
  DisplayAccount( buffer, 32, 0, 60, 2, ST7735_CYAN ); 

  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1();  // Stop encryption on PCD
}
// Display account info on TFT
void DisplayAccount( char *buffer, byte *length, byte *x, byte *y, byte *fontSize, uint16_t *color ){
   int16_t x1,y1;
   uint16_t w,h;
   byte i,j;

   // set color, cursor and text size 
   tft.setTextColor(color);
   tft.setCursor(x,y);
   tft.setTextSize(fontSize);

   j=0;
   for ( i=0; i < length; i++ ){ 
#if DEBUG 
     Serial.print( i );
     Serial.print( " : " );
     Serial.println( buffer[i] );
#endif
     // termination string is TERMINATION_STRING
     if ( buffer[i] == TERMINATION_STRING ) { 
       j+=1;
       if ( j > 1 ) { 
          break ; 
       } else { 
          tft.setCursor(x,y+20);
       }
     } else {

       tft.getTextBounds("A", tft.getCursorX(), tft.getCursorY(), &x1, &y1, &w, &h);
       tft.fillRect(x1, y1, w, h, ST7735_BLACK);
       tft.print( buffer[i] ); 
    }
   }

#if DEBUG 
     Serial.println( " bytes found: " );
     Serial.print( i );
#endif
   // display no keys found 
   if ( i == 0 ) {
     tft.setCursor(x+10,y);
     tft.getTextBounds("block empty", tft.getCursorX(), tft.getCursorY(), &x1, &y1, &w, &h);
     tft.fillRect(x1, y1, w, h, ST7735_BLACK);
     tft.print( "block empty" ); 
   }
} 

//*************************************************************
// Animations
//*************************************************************
// this animation is show while reading card 
void DisplayReadingCardNEW() {
  //DisplayBusy( ST7735_BLUE, ST7735_MAGENTA) ;
  uint16_t color = random(0, 65535);
  tft.fillScreen(ST7735_BLACK);
  for ( byte i =1; i<60; i+=2 ) {
     tft.drawCircle(80, 60, i, color);
  }
  for ( byte i =1; i<60; i+=2 ) {
     tft.drawCircle(80, 60, 60-i, ST7735_BLACK);
  }
}
// this animation is show while reading card 
void DisplayReadingCard() {
  DisplayBusy( ST7735_BLUE, ST7735_MAGENTA) ;
}
// this animation is show when correct card was read
void DisplayRightCard() {
  DisplaySuccess();
  tft.setCursor(10,20);
  tft.println("Key detected");
}
// this animation is show when wrong card read
void DisplayWrongCard() {
  // dislay error 
  DisplayError();
  tft.setCursor(20,50);
  tft.println("Wrong Card");
  delay(1000);
}
// this animation is show with passed message to be shown
void DisplayWrongCard(char* message) {
  // dislay error 
  DisplayError();
  tft.setCursor(0,50);
  tft.println(message);
  // pause afterwards
  delay(1000);
}
// Busy Animation
void DisplayBusy(uint16_t *color1, uint16_t *color2){
  for (int16_t x=10; x < tft.width(); x+=20) {
    for (int16_t y=10; y < tft.height(); y+=20) {
      tft.fillCircle(x, y, 10, color1);
    }
  }
  for (int16_t x=15; x < tft.width(); x+=20) {
    for (int16_t y=15; y < tft.height(); y+=20) {
      tft.fillCircle(x, y, 10, color2);
    }
  }
}
// Error Animation
void DisplayError(){
  DisplayBusy( ST7735_BLUE, ST7735_RED ) ;
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(2);
}
// Success Animation
void DisplaySuccess(){
  DisplayBusy( ST7735_BLUE, ST7735_GREEN ) ;
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(2);
}
void MyScreenSaver(){
   byte x,y,r;
   uint16_t color;
   r = random(1,15);
   x = random(0,160);
   y = random(0,128);
   color = random(0, 65535);
   for ( byte i =0; i<10; i++ ) {
      tft.drawCircle(x, y, r+i, color);
   }
   for ( byte i =0; i<10; i++ ) {
      tft.drawCircle(x, y, r+9-i,ST7735_BLACK);
   }
}

