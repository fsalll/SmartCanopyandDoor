#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Alamat 0x27 atau 0x3F, ukuran 16x2

// Inisialisasi Servo
Servo myServo;

// Definisikan Keypad
const byte ROWS = 4; // Ada 4 baris pada keypad
const byte COLS = 3; // Ada 3 kolom pada keypad
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Pins untuk baris
byte colPins[COLS] = {5, 4, 3};    // Pins untuk kolom
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variabel password
const String correctPassword = "6754";
String enteredPassword       = "";
const int maxPasswordLength  = 4;

// Pin
const int relayPin       = 11; // Pin untuk relay
const int waterSensorPin = A0; // Pin untuk sensor hujan
const int LDRSensorPin   = A1; // Pin untuk sensor LDR

// Variabel untuk status relay
bool relayStatus = false; // Status awal relay: OFF

// Variabel untuk timing
unsigned long relayOnTime = 0;
const unsigned long relayDuration = 3000; // Durasi relay ON dalam milidetik
const unsigned long incorrectPasswordDuration = 1000; // Durasi tampilan password salah dalam milidetik

void setup() {
  // Setup LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

  // Setup Relay Pin
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Mulai dengan relay OFF (Solenoid Tertutup)

  // Setup Servo
  myServo.attach(10); // Pin servo

  // Setup Sensor Hujan
  pinMode(waterSensorPin, INPUT);

  // Setup Sensor LDR
  pinMode(LDRSensorPin, INPUT);
}

void loop() {
  // Baca sensor hujan
  int waterSensorValue = analogRead(waterSensorPin);
  bool isRaining = waterSensorValue > 612; // Nilai threshold bisa disesuaikan

  // Baca sensor LDR
  int LDRSensorValue = analogRead(LDRSensorPin);
  bool isDay = LDRSensorValue > 512; // Nilai threshold bisa disesuaikan

  // Gerakkan servo berdasarkan status sensor
  if (isDay && !isRaining) {
    myServo.write(110); // Terang dan tidak hujan: servo bergerak ke kiri (Terbuka)
  } else {
    myServo.write(0); // Selain kondisi di atas: servo bergerak ke kanan (Tertutup)
  }

  // Cek apakah relay aktif
  bool isRelayActive = relayStatus && (millis() - relayOnTime < relayDuration);

  // Proses Keypad hanya jika relay tidak aktif
  if (!isRelayActive) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') {
        if (enteredPassword == correctPassword) {
          digitalWrite(relayPin, LOW); // Password benar, relay ON (Solenoid Terbuka)
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Granted");
          relayStatus = true;
          relayOnTime = millis(); // Catat waktu ketika relay diaktifkan
        } else {
          // Password salah, tampilkan pesan kesalahan
          digitalWrite(relayPin, HIGH); // Password salah, relay OFF (Solenoid Tertutup)
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Denied");
          relayStatus = false;
          delay(incorrectPasswordDuration); // Tampilkan pesan selama 1 detik
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

  // Matikan relay setelah 3 detik
  if (relayStatus && (millis() - relayOnTime >= relayDuration)) {
    digitalWrite(relayPin, HIGH); // Matikan relay (Solenoid Tertutup)
    relayStatus = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:"); // Kembali ke tampilan awal
  }
}
