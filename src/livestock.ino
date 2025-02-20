#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Initialize LCD with I2C address 0x27, 16 columns, and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myservo;

// Define RFID module pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Store product information
char products[6] = {'X', 'X', 'X', 'X', 'X', 'X'};
int total = 0;

// Define buzzer pin
#define BUZZER_PIN 3

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    myservo.attach(3);
    
    Serial.begin(9600);
    SPI.begin();  // Initialize SPI bus
    mfrc522.PCD_Init(); // Initialize RFID module
    
    Serial.println("Approximate your card to the reader...");
    
    lcd.init();
    lcd.backlight();
    lcd.print("Animal monitoring");
    delay(2000);
    lcd.clear();
}

void loop() {
    rfidmodule();
    Serial.println(total);
    
    lcd.setCursor(0, 0);
    lcd.print("Total: ");
    lcd.print(total);
    delay(1000);
}

void rfidmodule() {
    // Check if a new RFID card is present
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }
    
    // Read the card serial number
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }
    
    // Display UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();
    
    // Check RFID tag and update the status
    processAnimal("D7 27 40 D7", 'A', "Animal 1");
    processAnimal("0C AB B3 2F", 'B', "Animal 2");
    processAnimal("D3 1C 0B 1E", 'C', "Animal 4");
    processAnimal("E7 26 36 D7", 'D', "Animal 3");
}

void processAnimal(String tagID, char identifier, String animalName) {
    String content;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    
    if (content.substring(1) == tagID) {
        int index = identifier - 'A';
        
        if (total > 0 && products[index] == identifier) {
            total -= 1;
            products[index] = 'X';
            myservo.write(180);
            lcd.setCursor(0, 1);
            lcd.print(animalName + " came out");
            delay(500);
            lcd.clear();
        } else {
            Serial.println(animalName + " inside");
            products[index] = identifier;
            myservo.write(0);
            lcd.setCursor(0, 1);
            lcd.print(animalName + " inside");
            digitalWrite(BUZZER_PIN, HIGH);
            total += 1;
            delay(500);
            lcd.clear();
            digitalWrite(BUZZER_PIN, LOW);
        }
        delay(500);
    } else {
        Serial.println("Invalid Product");
        lcd.setCursor(0, 1);
        lcd.print("Invalid");
        delay(1000);
        lcd.clear();
    }
}
