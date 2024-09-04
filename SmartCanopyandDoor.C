#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// LCD INITIAL
LiquidCrystal_I2C lcd(0x27, 16, 2); // 
// servo INITIAL
Servo myServo;

// Keypad 
const byte ROWS = 4; 
const byte COLS = 3; 
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3};    
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String correctPassword = "6754";
String enteredPassword       = "";
const int maxPasswordLength  = 4;

// Pin
const int relayPin       = 11; 
const int waterSensorPin = A0; 
const int LDRSensorPin   = A1; 

bool relayStatus = false; 


unsigned long relayOnTime = 0;
const unsigned long relayDuration = 3000; // in milisec
const unsigned long incorrectPasswordDuration = 1000; //wrong pass in milisec

void setup() {
  // Setup LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

 //setup relay pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // start in relay OFF (lock)

  // Setup Servo
  myServo.attach(10);
  // Setup water sensor
  pinMode(waterSensorPin, INPUT);

  // Setup Sensor LDR
  pinMode(LDRSensorPin, INPUT);
}

void loop() {
  int waterSensorValue = analogRead(waterSensorPin);
  bool isRaining = waterSensorValue > 612; 

  // Baca sensor LDR
  int LDRSensorValue = analogRead(LDRSensorPin);
  bool isDay = LDRSensorValue > 512; 

  // move servo
  if (isDay && !isRaining) {
    myServo.write(110);
  } else {
    myServo.write(0); 
  }

  // iff relay on
  bool isRelayActive = relayStatus && (millis() - relayOnTime < relayDuration);

  //Keypad hanya jika relay tidak aktif
  if (!isRelayActive) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        if (enteredPassword == correctPassword) {
          digitalWrite(relayPin, LOW); // correct pass, relay ON
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Granted");
          relayStatus = true;
          relayOnTime = millis(); // Catat waktu ketika relay diaktifkan
        } else {
          //wrong pass, show information
          digitalWrite(relayPin, HIGH); // wrong pass, relay OFF
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Denied");
          relayStatus = false;
          delay(incorrectPasswordDuration); // show in 1 sec
        }
        // Reset password
        enteredPassword = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
      } else if (key == '*') {
        // Reset password
        enteredPassword = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Password:");
      } else {
        // Menambahkan digit ke password yang dimasukkan
        if (enteredPassword.length() < maxPasswordLength) {
          enteredPassword += key;
          lcd.setCursor(0, 1);
          lcd.print(enteredPassword);
        }
      }
    }
  } else {
    // Display relay status
    lcd.setCursor(0, 1);
    lcd.print("Door Unlocked");
  }

  // relay off in 3 sec
  if (relayStatus && (millis() - relayOnTime >= relayDuration)) {
    digitalWrite(relayPin, HIGH); // relay off
    relayStatus = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:"); // Kembali ke tampilan awal
  }
}
