#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal.h>

//////// LCD pins: (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

/////// function to center text on a given row
void printCentered(const String &text, int row) {
  int len = text.length();
  int col = (16 - len) / 2;
  if (col < 0) col = 0;  // safety
  lcd.setCursor(col, row);
  lcd.print(text);
}

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, use SoftwareSerial
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // pin #2 is RX, #3 is TX
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// 🟢 Add these missing declarations:
String storedNames[20] = {"Joey"};  // you can add more names later if needed
uint8_t nameCount = 1;              // currently we have 1 stored name

void setup()
{
  lcd.begin(16, 2);
  ////////////////Startup 
  lcd.clear();
  printCentered("Fingerprint", 0);
  printCentered("Initializing...", 1);
  delay(2000);
  lcd.clear();

  Serial.begin(9600);
  while (!Serial);  
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // Initialize fingerprint sensor
  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.clear();
    printCentered("Sensor not", 0);
    printCentered("found!", 1);
    while (1) { delay(1); }
  }

  // Display sensor info on serial
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.println("No fingerprint templates found.");
  } else {
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }

  ////////// Display welcome message
  lcd.clear();
  printCentered("Welcome", 0);
  printCentered("Waiting for", 1);
  delay(1500);
  lcd.clear();
  printCentered("Waiting for", 0);
  printCentered("fingerprint", 1);
}

void loop() {
  getFingerprintID();
  delay(50);
}

// --- Fingerprint reading function ---
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      return p;
    default:
      return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    uint8_t matchID = finger.fingerID;
    String name = (matchID <= nameCount) ? storedNames[matchID - 1] : "Unknown";

    Serial.print("Recognized: "); Serial.println(name);

    lcd.clear();
    printCentered("Authorized", 0);
    printCentered("--Unlocked--", 1);
    delay(2000);

    lcd.clear();
    printCentered("Welcome,", 0);
    printCentered(name, 1);
    delay(3500);

  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No match found.");
    lcd.clear();
    printCentered("Unauthorized", 0);
    printCentered("--LOCKED--", 1);
    delay(3500);
  }

  lcd.clear();
  printCentered("Waiting for", 0);
  printCentered("fingerprint", 1);
  return p;
}
