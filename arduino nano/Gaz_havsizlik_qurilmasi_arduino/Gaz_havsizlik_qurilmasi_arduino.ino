// Kutubxonalar
#include <Wire.h>  // I2C kommunikatsiyasi uchun kutubxona
#include <LiquidCrystal_I2C.h>  // I2C orqali LCD ekranini boshqarish uchun kutubxona
#include <SoftwareSerial.h>  // SoftwareSerial kutubxonasi, bu kutubxona orqali bir nechta serial portlarni ishlatish mumkin
#include "DHT.h"  // DHT11/DHT22 sensorlarini boshqarish uchun kutubxona

// O'zgaruvchilar va konstantalar
#define EMERGENCY_PHONE_NUMBER "+998977477616"  // Havfsizlik xabarini yuborish uchun telefon raqami
#define minValue 0  // Analog sensorning minimal qiymati
#define maxValue 1023  // Analog sensorning maksimal qiymati
#define threshold 50  // To'xtatish chegarasi


//analog pinlar
#define analogSensor A0  // Analog sensor uchun pin
#define IRSensor A1  // IR sensor uchun pin
#define vibrationSensorPin A2// Vibratsiya sensori uchun pin
//bosh analog pinlar
#define bosh_A3 A3 // bosh A3 pin
#define bosh_A4 A4 // bosh A4 pin
#define bosh_A5 A5 // bosh A5 pin
#define bosh_A6 A6 // bosh A6 pin
#define bosh_A7 A7 // bosh A7 pin



// digital pinlar
#define DHT11_PIN 2  // DHT11 sensorini ulash uchun Arduino pin
#define relay1 3  // Relay 1 boshqaruv pin
#define relay2 4  // Relay 2 boshqaruv pin
#define relay3 5  // Relay 3 boshqaruv pin
#define relay4 6  // Relay 4 boshqaruv pin
#define rx_esp 7  // esp rx pin
#define tx_esp 8  // esp tx pin
#define rx_A9G 9  // A9G rx pin
#define tx_A9G 10 //A9G tx pin
#define buzzer 11 // Buzzer uchun pin
// bosh digital pinlar
#define pin12 12  // bosh pin 12
#define pin13 13  // bosh pin 13

// Obyektlar
DHT dht11(DHT11_PIN, DHT11);  // DHT11 obyektini e'lon qilish
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD obyekti
SoftwareSerial mySerial(rx_A9G, 10);  // GSM moduli uchun SoftwareSerial obyekti
SoftwareSerial espSerial(rx_esp, tx_esp);  // ESP moduli uchun SoftwareSerial obyekti

// O'zgaruvchilar
float temp, humidity, gasConcentration, irIntensity;  // Harorat, namlik, gastrafik konsentratsiyasi, infrakras intensivligi
int vibrationValue;  // Vibratsiya qiymati

void setup() {
  pinMode(analogSensor, INPUT);
  pinMode(IRSensor, INPUT);
  pinMode(vibrationSensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  Serial.begin(115200);  // Serial kommunikatsiyani boshlash
  lcd.init();  // LCD ekranini ishga tushirish
  lcd.backlight();  // LCD ekranini yoritish
  mySerial.begin(115200);  // mySerial kommunikatsiyani boshlash
  espSerial.begin(115200);  // espSerial kommunikatsiyani boshlash
  mySerial.println("AT+GPS=1");  // GPS ni yoqish
  delay(1000);

  dht11.begin();  // DHT11 sensorini boshlash
}

void loop() {

  
 digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);


  float humi = dht11.readHumidity();  // Namlikni o'qish
  float tempC = dht11.readTemperature();  // Haroratni o'qish (Selsiy)
  delay(2000);

  lcd.setCursor(0, 0);  // LCD ekranida kursorning joylashuvini belgilash
  lcd.print("Temp: ");  // Haroratni chiqarish
  lcd.print(tempC);
  lcd.print(" *C");

  int sensorValue = analogRead(analogSensor);  // Analog sensor qiymatini o'qish
  gasConcentration = map(sensorValue, minValue, maxValue, 0, 100);  // Gastrafik konsentratsiyani hisoblash
  lcd.setCursor(16, 1);  // LCD ekranida kursorning joylashuvini belgilash
  lcd.print("Gaz.: ");  // Gastrafik konsentratsiyani chiqarish
  lcd.print(gasConcentration);
  lcd.print("%");

  vibrationValue = analogRead(vibrationSensorPin);  // Vibratsiya sensori qiymatini o'qish
  lcd.setCursor(-4, 2);  // LCD ekranida kursorning joylashuvini belgilash
  lcd.print("Vibratsiya: ");  // Vibratsiyani chiqarish
  lcd.print(vibrationValue);

  int irValue = analogRead(IRSensor);  // IR sensor qiymatini o'qish
  irIntensity = map(irValue, minValue, maxValue, 100, 0);  // Infrakras intensivligini hisoblash
  lcd.setCursor(0, 3);  // LCD ekranida kursorning joylashuvini belgilash
  lcd.print("IR: ");  // Infrakras intensivligini chiqarish
  lcd.print(irIntensity);
  lcd.print("%");

  Serial.print("Harorat: ");  // Haroratni chiqarish
  Serial.print(tempC);
   Serial.println(" *C");
  Serial.print("Namlik: ");  // Namlikni chiqarish
  Serial.print(humi);
  Serial.println("%");
  Serial.print("Gaz Kons.: ");  // Gastrafik konsentratsiyani chiqarish
  Serial.print(gasConcentration);
  Serial.println("%");
  Serial.print("IR Intens.: ");  // Infrakras intensivligini chiqarish
  Serial.println(irIntensity);
  Serial.print("Vibratsiya: ");  // Vibratsiyani chiqarish
  Serial.println((vibrationValue > threshold) ? "Aniq" : "Aniq emas");

  sendSensorDataToESP();  // Sensor ma'lumotlarini ESP ga yuborish

  delay(100);

  if (espSerial.available() > 0) {  // Agar ESP dan ma'lumot kelgan bo'lsa
    String espCommand = espSerial.readStringUntil('\n');  // ESP dan kelgan buyruqni o'qish
    Serial.println("Command from ESP: " + espCommand);  // ESP dan kelgan buyruqni chiqarish

    if (espCommand.startsWith("TURN_ON_RELAY1")) {  // Agar buyruq "TURN_ON_RELAY1" bo'lsa
      digitalWrite(relay1, LOW);  // Relay 1 ni yoqish
    } else if (espCommand.startsWith("TURN_OFF_RELAY1")) {  // Agar buyruq "TURN_OFF_RELAY1" bo'lsa
      digitalWrite(relay1, HIGH);  // Relay 1 ni o'chirish
    }
    espSerial.flush();  // ESP serial bufferini tozalash
  }

  if (gasConcentration > threshold) {  // Agar gastrafik konsentratsiya chegaradan oshsa
    handleGasConcentration();  // Gastrafik konsentratsiyani boshqarish
    turnOffRelays();  // Barcha relaylarni o'chirish
  } else if (irIntensity > threshold) {  // Agar infrakras intensivligi chegaradan oshsa
    handleIRIntensity();  // Infrakras intensivligini boshqarish
  } else if (vibrationValue > threshold) {  // Agar vibratsiya chegaradan oshsa
    handleVibration();  // Vibratsiyani boshqarish
  } else {
  }
}

void sendSensorDataToESP() {  // Sensor ma'lumotlarini ESP ga yuborish
  espSerial.print("IR:      ");
  espSerial.println(irIntensity);
  espSerial.print("MQ9:     ");
  espSerial.println(gasConcentration);
  espSerial.print("Harorat: ");
  espSerial.println(temp);
  espSerial.print("Namlik: ");
  espSerial.println(humidity);
  espSerial.print("Vibratsiya: ");
  espSerial.println((vibrationValue == HIGH) ? "Aniq" : "Aniq emas");
}

void turnOffRelays() {  // Barcha relaylarni o'chirish
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void sendSMS(String message) {  // SMS yuborish
  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.println((char)26);
  delay(1000);
}

void makeCall() {  // Qo'ng'iroq qilish
  mySerial.println("ATD" + String(EMERGENCY_PHONE_NUMBER) + ";");
  delay(10000);
  mySerial.println("ATH");
}

void sendLocation() {  // Lokatsiyani yuborish
  mySerial.println("AT+LOCATION=2");
  delay(1000);
  while (mySerial.available()) {
    String location = mySerial.readStringUntil('\n');
    if (location.indexOf("+LOCATION: ") >= 0) {
      location = location.substring(location.indexOf("+LOCATION: ") + 11);
      String longitude = location.substring(0, location.indexOf(","));
      String latitude = location.substring(location.indexOf(",") + 1);
      sendSMS("Lokatsiya: " + latitude + ", " + longitude);
    }
  }
}

void activateBuzzer() {  // Buzzer ishga tushirish
  for (int i = 400; i <= 1500; i++) {
    tone(buzzer, i);
    delay(5);
  }
  for (int i = 1500; i >= 400; i--) {
    tone(buzzer, i);
    delay(5);
  }
}

void handleGasConcentration() {  // Gastrafik konsentratsiyani boshqarish
  lcd.clear();
  lcdhelp();
  turnOffRelays();
  activateBuzzer();
  sendSMS("Gaz konsentratsiyasi belgilangan chegaradan oshiq!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();

  if (gasConcentration > threshold) {
    handleGasConcentration();
    turnOffRelays();
  } else if (irIntensity > threshold) {
    handleIRIntensity();
  } else if (vibrationValue > threshold) {
    handleVibration();
  } else {
  }
  lcd.clear();
}

void handleIRIntensity() {  // Infrakras intensivligini boshqarish
  lcd.clear();
  lcdhelp();
  turnOffRelays();
  activateBuzzer();
  sendSMS("IR sensori yonmanganda o'tqan!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();

  if (gasConcentration > threshold) {
    handleGasConcentration();
    turnOffRelays();
  } else if (irIntensity > threshold) {
    handleIRIntensity();
  } else if (vibrationValue > threshold) {
    handleVibration();
  } else {
  }
  lcd.clear();
}

void handleVibration() {  // Vibratsiyani boshqarish
  lcd.clear();
  lcdhelp();
  turnOffRelays();
  activateBuzzer();
  sendSMS("Vibratsiya aniqlandi!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();

  if (gasConcentration > threshold) {
    handleGasConcentration();
    turnOffRelays();
  } else if (irIntensity > threshold) {
    handleIRIntensity();
  } else if (vibrationValue > threshold) {
    handleVibration();
  } else {
  }

  lcd.clear();
}

void lcdhelp() {  // LCD ekraniga yordam matnini chiqarish
  lcd.setCursor(0, 0);
  lcd.print("Siz Xaf ostidasiz");
  lcd.setCursor(-4, 1);
  lcd.print("Uyni tark eting");
  lcd.setCursor(-4, 2);
  lcd.print("Haf Haf Haf");
  lcd.setCursor(16, 3);
  lcd.print("Habar berildi");
}