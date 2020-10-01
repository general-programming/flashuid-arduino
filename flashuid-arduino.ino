/*
 * --------------------------------------------------------------------------------------------------------------------
 * flashuid-arduino with screen support! I KNOW I SHOULD'VE USED memcpy BUT I SUCK AT DOING THAT
 * --------------------------------------------------------------------------------------------------------------------
 * 
 * @author linuxgemini
 * @license MIT
 * 
 * SHOUTOUT TO:
 *     Deviant Ollam
 *     Ave
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
 
// screen related libraries, we use liquidcrystal_i2c here
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);    // set the LCD address to 0x27 for a 16 chars and 2 line display

// MRFC522 related libraries and their configurations
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN   9                  // Configurable, see typical pin layout above
#define SS_PIN    10                 // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);    // Create MFRC522 instance
MFRC522::MIFARE_Key key;             // appearently MRFC522 library needs it???

byte TARGET_UID[4];                  // the card's serial is buffered at this variable
bool DID_MAGIC_HAPPEN = false;       // magic means the 1st stage of cloning: getting the serial 

void setup()
{
  lcd.init();
  lcd.backlight();
  
  // we don't really need serial at this point
  //// Serial.begin(9600);                // Initialize serial communications with the PC
  //// while (!Serial);                   // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
      
  SPI.begin();                       // Init SPI bus
  mfrc522.PCD_Init();                // Init MFRC522 card

  //// Serial.println(F("LET'S GOOOOOOO"));
  
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory. See: line 37
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop()
{
  delay(500);
  
  // return to square one if no cards are present, saving processing time?
  if ( ! mfrc522.PICC_IsNewCardPresent() ) return;

  /*
   * If your Chinese MIFARE Classic gets bricked, you can unbrick it!
   * Just comment out the extended checks below and reflash.
   */

  ////// START EXTENDED CHECKS
  // return to square one if "the card serial is unavaiable and no serial is gathered"
  if ( ! mfrc522.PICC_ReadCardSerial() && !DID_MAGIC_HAPPEN ) return;
  
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); // basically gathering the card type

  // return to square one if "the card is not a mifare classic and no serial is gathered"
  if ( piccType != MFRC522::PICC_TYPE_MIFARE_1K && !DID_MAGIC_HAPPEN ) {
      lcd.print("Only MIFARE 1K!");
      delay(2500);

      lcd.clear();
      
      return;
  }
  ////// END EXTENDED CHECKS

  // if serial gathered
  if ( DID_MAGIC_HAPPEN ) {
    // write to the cUID card and check for errors, also resetting the clone check
    if ( MIFARE_BackdooredSetUID(TARGET_UID, 4, false) ) {
      lcd.print("UID Cloned!");
      lcd.setCursor(0,1);
      lcd.print("Have fun!");
      
      DID_MAGIC_HAPPEN = false;
  
      delay(2500);
    } else {
      lcd.print("Error!");
      lcd.setCursor(0,1);
      lcd.print("Sequence reset!");
      
      DID_MAGIC_HAPPEN = false;
  
      delay(2500);
    }
  } else {
    // if serial hasn't been gathered, gather it and set the clone check to true
    memcpy(TARGET_UID, mfrc522.uid.uidByte, 4);
    
    lcd.print("UID now in RAM!");
    lcd.setCursor(0,1);
    lcd.print("Tap UIDc card!");
    
    DID_MAGIC_HAPPEN = true;

    delay(2500);
  }
  
  lcd.home();
  lcd.clear();
}

bool MIFARE_BackdooredSetUID(byte *uidbuff, byte bufsize, bool logErrors) {
  // if uid buffer is a null pointer and the buffer size isn't 4, return false
  if (uidbuff == nullptr || bufsize != 4) {
    return false;
  }

  byte bcc_value = uidbuff[0] ^ uidbuff[1] ^ uidbuff[2] ^ uidbuff[3]; // calculate the bcc by xor'ing the uid bytes
  
  mfrc522.MIFARE_OpenUidBackdoor(logErrors);                          // we utilise the library function here, basically opens the backdoor to write to the uid sector of the cUID card

  byte end_buffer[] = {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // the last 11 bytes of the uid sector is pre-determined; ATQA and SAK is set to their nominal values. TODO: fetch the entire uid sector of the cloned card and use that info

  byte final_buf[16];               // we define our uid sector

  memcpy(final_buf, uidbuff, 4);
  final_buf[4] = bcc_value;
  memcpy(&final_buf[5], end_buffer, 11);

  byte stat = mfrc522.MIFARE_Write((byte)0, final_buf, (byte)16); // write modified block 0 back to card, get its result as a custom byte
  if (stat != MFRC522::STATUS_OK) {
    if (logErrors) {
      //// Serial.print(F("mfrc522.MIFARE_Write() failed: "));
      //// Serial.println(mfrc522.GetStatusCodeName(stat));
    }
    return false;
  }
  return true;
}
