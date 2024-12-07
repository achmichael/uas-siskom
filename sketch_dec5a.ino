#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Servo.h>


#define DS18B20_PIN 2
#define DHT22_PIN 3
#define BUTTON_START 5 
#define BUTTON_POWER 6
#define BUZZER_PIN 10
#define SERVO_PIN 9
#define POT_PIN A0

// Sensor dan perangkat (tetap sama)
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
DHT dht(DHT22_PIN, DHT22);
Servo teaServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variabel global
bool systemOn = false;
bool brewing = false;
int teaType = 0;
int boilingTime = 0;           // Waktu perebusan yang sudah ditentukan
float targetTemp = 0;
int oldButtonStart = 0;
float durationInMinutes = 0;
int potValue = 0;
int previousPotValue = -1;

float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void accurateDelay(float minutes) {
  unsigned long startTime = millis();
  unsigned long delayMillis = minutes * 60000;  // Konversi menit ke milidetik

  Serial.print("Durasi brewing: ");
  Serial.print(minutes);
  Serial.println(" menit");

  // Loop untuk mempertahankan delay tanpa memblokir
  while (millis() - startTime < delayMillis) {
    // Opsional: Tambahkan update LCD atau fungsi lain di sini jika diperlukan
    lcd.setCursor(0, 1);
    lcd.print("Brewing: ");
    lcd.print((delayMillis - (millis() - startTime)) / 1000);
    lcd.print(" detik  ");
    delay(1000);  // Update setiap detik
  }

  Serial.println("Brewing selesai");
}

void setup() {
  // Inisialisasi perangkat
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_POWER, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, HIGH);

  Serial.begin(9600);
  teaServo.attach(SERVO_PIN);

  lcd.init();
  lcd.backlight();

  dht.begin();
  sensors.begin();

  teaServo.write(0);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tekan Start");
}

void loop() {
  // Pembacaan tombol start
  int buttonStart = digitalRead(BUTTON_POWER);

  // Toggle system on/off
  if (buttonStart == LOW && oldButtonStart == HIGH) {
    systemOn = !systemOn;

    if (systemOn) {
      teaServo.write(80);
      delay(2000);

       teaServo.write(120);
      delay(2000);

       teaServo.write(180);
      delay(2000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sistem Aktif");
      Serial.println("Sistem Aktif");
      digitalWrite(BUZZER_PIN, LOW);
      tone(BUZZER_PIN, 5, 100);
      delay(1000);
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      // Reset semua variabel saat sistem dimatikan
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tekan Start");
      Serial.println("Sistem Nonaktif");
      digitalWrite(BUZZER_PIN, LOW);
      tone(BUZZER_PIN, 5, 100);
      delay(1000);
      digitalWrite(BUZZER_PIN, HIGH);
      teaType = 0;
      brewing = false;
    }
  }

  oldButtonStart = buttonStart;
  // Proses hanya jika sistem aktif
  if (systemOn) {
    // Pembacaan nilai potentiometer untuk menentukan waktu perebusan
    potValue = analogRead(POT_PIN);
    
    // Mengecek jika ada perubahan signifikan pada nilai potentiometer
    // if (abs(potValue - previousPotValue) > 5 && !brewing) {  // Jika perubahan lebih besar dari 5 (threshold)
      previousPotValue = potValue;  // Menyimpan nilai potentiometer terbaru
    
      // Mengonversi nilai dari potentiometer menjadi waktu (misal 0-1023 -> 0-300 detik)
      boilingTime = map(potValue, 0, 1023, 0, 300);  // Mengubah range potentiometer menjadi 0 - 300 detik (1 - 5 menit)
    
      // Debugging nilai waktu perebusan dalam menit dan detik
      // int minutes = boilingTime / 60;        // Menghitung menit
      // int seconds = boilingTime % 60;        // Menghitung sisa detik
      // Serial.print("Minutes: ");
      // Serial.print(minutes);
      // Serial.print(", Seconds: ");
      // Serial.println(seconds);
      Serial.println("Boiling time " + String(boilingTime));

      // Menampilkan waktu pada LCD
      // lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Waktu: ");
      // lcd.print(minutes);
      lcd.print("m ");
      // lcd.print(seconds);
      lcd.print("s");
    // }

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (temperature == DEVICE_DISCONNECTED_C) {
      Serial.println("Sensor gagal membaca suhu!");
    } else if (!isnan(temperature) && !isnan(humidity)) {
      Serial.println("Suhu sekitar: " + String(temperature));
      Serial.println("Kelembapan: " + String(humidity));
    } else {
      Serial.println("Nilai sensor tidak valid");
    }

    delay(2000);

    // Pembacaan tombol start untuk memulai proses perebusan
    int startButton = digitalRead(BUTTON_START);
    if (startButton == LOW && !brewing) {
      brewing = true;  
      Serial.println("Proses rebus dimulai");
      for (int i = 0; i <= 180; i ++){
        teaServo.write(i);
        delay(15);
      }

      for (int i = 180; i >= 0; i --){
        teaServo.write(i);
        delay(15);
      }

      Serial.print("Durasi: ");
      Serial.println(boilingTime);
      accurateDelay(boilingTime / 60.0);  // Memulai proses perebusan berdasarkan waktu yang ditentukan
      digitalWrite(BUZZER_PIN, LOW);
      tone(BUZZER_PIN, 5, 100);
      delay(1000);
      digitalWrite(BUZZER_PIN, HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Brewing selesai");
      brewing = false;
      boilingTime = 0;
    }
  }
}
