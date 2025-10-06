#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // pin #2 = RX, #3 = TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

////////// Store up to 20 names, each assigned a numeric ID (1–127)
String storedNames[20];
uint8_t nameCount = 0;

uint8_t id;
String currentName;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment (Name-based)");

  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
}

String readName() {
  String input = "";
  while (input.length() == 0) {
    while (!Serial.available());
    input = Serial.readStringUntil('\n');
    input.trim();
  }
  return input;
}

void loop() {
  Serial.println("\nReady to enroll a fingerprint!");
  Serial.println("Please type in the NAME you want to save this finger as:");
  currentName = readName();

  // Check if name already exists in our list
  for (uint8_t i = 0; i < nameCount; i++) {
    if (storedNames[i].equalsIgnoreCase(currentName)) {
      Serial.print("This name already exists as ID #");
      Serial.println(i + 1);
      return;
    }
  }

  // Assign next available numeric ID
  id = nameCount + 1;
  if (id > 127) {
    Serial.println("Error: maximum ID limit reached!");
    return;
  }

  storedNames[nameCount] = currentName;
  nameCount++;

  Serial.print("Enrolling fingerprint for '");
  Serial.print(currentName);
  Serial.print("' as ID #");
  Serial.println(id);

  while (!getFingerprintEnroll());
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger for "); Serial.println(currentName);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.print(".");
    } else if (p == FINGERPRINT_OK) {
      Serial.println("\nImage taken");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Imaging error");
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return p;

  Serial.println("Remove finger");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place same finger again");
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOFINGER) Serial.print(".");
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return p;

  Serial.println("Creating model...");
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.print("Stored fingerprint for ");
    Serial.print(currentName);
    Serial.print(" (ID #");
    Serial.print(id);
    Serial.println(")");
  } else {
    Serial.println("Error storing fingerprint");
  }

  return true;
}
