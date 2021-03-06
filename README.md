# VaultM
This project describes how to build your own RFID reader
into an IBM Model M keyboard using an Arduino Micro, a 
MFRC522 module and a 1.8" TFT screen. 

This project is based on following freely available libraries:

RFID library:
https://github.com/miguelbalboa/rfid

Adafruits TFT library:
https://github.com/adafruit/Adafruit-ST7735-Library

Schematic
-----------------------------
Connect the parts as follows: 

MFRC522 to Arduino
  * MOSI    ->  MOSI
  * MISO    ->  MISO
  * SCK     ->  SCK
  * 3.3V    ->  3.3V
  * GND     ->  GND
  * SDA     ->  12 
  * RST     ->  11 

TFT to Arduino
  * LED     ->  1K Ohm -> 3.3V
  * SCK     ->  SCK
  * SDA     ->  MOSI
  * A0/DC   ->  6
  * RST     ->  7 
  * CS      ->  8
  * GND     ->  GND
  * VCC     ->  3.3V
  
3 buttons: 
  * RED+    ->  4
  * RED-    ->  GND
  * YELLOW+ ->  3
  * YELLOW- ->  GND
  * BLACK+  ->  2
  * BLACK-  ->  GND

Usage
-----------------------------
* Password vault is unlocked by scanning the key card with the ID defined in "VaultKey". This can be a Mifare UL or Mifare 1K card. 
* Select password by pressing yellow button. Confirm selection with black button or abort action with the red button. 
* Scan key card containing the right account info (account name+password). 

Notes
------------------------------
To access the sector on the Mifare card Key A defined as "SectorKey" is used. 

It is assumed that the account information (account name + password)
is stored on the first block in every sector. 
For account #5 this is the 5th sector and block #21. 
