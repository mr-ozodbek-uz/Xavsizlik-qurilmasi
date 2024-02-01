#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>

float temp; // Harorat
float humidity; // Namlik
float gasConcentration; // Gastrafik konsentratsiyasi
float irIntensity; // Infrakras intensivligi
int vibrationValue; // Vibratsiya qiymati

#define analogSensor A0
#define minValue 0
#define maxValue 1023
#define threshold 50
#define DHTPIN 2
#define DHTTYPE DHT22
#define IRSensor A1
#define vibrationSensorPin A3
#define buzzer 8
#define relay1 3  // Relay 1 boshqaruv pin
#define relay2 4  // Relay 2 boshqaruv pin
#define relay3 8  // Relay 3 boshqaruv pin
#define relay4 7  // Relay 4 boshqaruv pin

// Konstantalar
#define EMERGENCY_PHONE_NUMBER "+998xxxxxxxxx"

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD obyekti
SoftwareSerial mySerial(10, 11); // GSM moduli uchun SoftwareSerial obyekti
SoftwareSerial espSerial(5, 6); // ESP moduli uchun SoftwareSerial obyekti
DHT dht(DHTPIN, DHTTYPE); // DHT sensori obyekti

void setup() {
  // Pinlarni modlash
  pinMode(analogSensor, INPUT);
  pinMode(IRSensor, INPUT);
  pinMode(vibrationSensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  Serial.begin(115200); // Serial aloqani boshlash
  lcd.init(); // LCD ni boshlash
  lcd.backlight(); // LCD ga orqa yorug'ini yoqish
  mySerial.begin(115200); // GSM modul Serial aloqasini boshlash
  espSerial.begin(115200); // ESP modul Serial aloqasini boshlash
  mySerial.println("AT+GPS=1"); // GSM modulda GPS ni yoqish
  delay(1000);
  dht.begin(); // DHT sensorini boshlash
}

void loop() {
  delay(2000);

  // DHT sensoridan harorat o'qish
  temp = dht.readTemperature();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" *C");

  // Analog sensor orqali gastrafik konsentratsiyasini o'qish
  int sensorValue = analogRead(analogSensor);
  gasConcentration = map(sensorValue, minValue, maxValue, 0, 100);
  lcd.setCursor(0, 1);
  lcd.print("Gaz kons.: ");
  lcd.print(gasConcentration);
  lcd.print("%");

  // Vibratsiya sensoridan qiymat o'qish
  vibrationValue = analogRead(vibrationSensorPin);
  lcd.setCursor(0, 2);
  lcd.print("Vibratsiya: ");
  lcd.print(vibrationValue);

  // Infrakras intensivligini analog sensor orqali o'qish
  int irValue = analogRead(IRSensor);
  irIntensity = map(irValue, minValue, maxValue, 100, 0);
  lcd.setCursor(-4, 3);
  lcd.print("IR intens.: ");
  lcd.print(irIntensity);
  lcd.print("%");

  // Sensor o'qishlarni tekshirib, kerakli harakatni bajarish
  if (gasConcentration > threshold) {
    turnOffRelays();
    handleGasConcentration();
  } else if (irIntensity > threshold) {
    turnOffRelays();
    handleIRIntensity();
  } else if (vibrationValue > threshold) {
    turnOffRelays();
    handleVibration();
  } else {
    turnOffRelays();
  }

  // Sensor o'qishlarini Serial monitor ga chiqarish
  Serial.print("Harorat: ");
  Serial.print(temp);
  Serial.println(" *C");
  Serial.print("Namlik: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Gaz Kons.: ");
  Serial.print(gasConcentration);
  Serial.println("%");
  Serial.print("IR Intens.: ");
  Serial.println(irIntensity);
  Serial.print("Vibratsiya: ");
  Serial.println((vibrationValue > threshold) ? "Aniq" : "Aniq emas");

  // Sensor ma'lumotlarini ESP modulga yuborish
  sendSensorDataToESP();

  delay(100);
}

// Sensor ma'lumotlarini ESP modulga yuborish uchun funksiya
void sendSensorDataToESP() {
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

// Relaysni o'chirish uchun funksiya
void turnOffRelays() {
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

// SMS xabar yuborish uchun funksiya
void sendSMS(String message) {
  mySerial.println("AT+CMGF=1"); // SMS matn rejimini sozlash
  delay(1000);
  mySerial.println("AT+CMGS=\"" + EMERGENCY_PHONE_NUMBER + "\""); // Qabul qiluvchi telefon raqamini sozlash
  delay(1000);
  mySerial.println(message); // SMS xabarni yuborish
  delay(100);
  mySerial.println((char)26); // Xabar tugallanganini bildirish uchun Ctrl+Z yuborish
  delay(1000);
}

// Qo'ng'iroq qilish uchun funksiya
void makeCall() {
  mySerial.println("ATD" + EMERGENCY_PHONE_NUMBER + ";"); // Telefon raqamini tanlash va qo'ng'iroq qilish
  delay(10000); // 10 soniya kutiladi (kerak bo'lsa o'zgartirilishi mumkin)
  mySerial.println("ATH"); // Qo'ng'iroqni to'xtatish
}

// Lokatsiyani SMS orqali yuborish uchun funksiya
void sendLocation() {
  mySerial.println("AT+LOCATION=2"); // GSM moduldan lokatsiya ma'lumotini so'raganlik
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

// Buzzer ni faollashtirish uchun funksiya
void activateBuzzer() {
  for (int i = 400; i <= 1500; i++) {
    tone(buzzer, i);
    delay(5);
  }
  for (int i = 1500; i >= 400; i--) {
    tone(buzzer, i);
    delay(5);
  }
}

// Gaz konsentratsiyasiga asosan harakatni boshqarish uchun funksiya
void handleGasConcentration() {
  sendSMS("Gaz konsentratsiyasi belgilangan chegaradan oshiq!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}

// IR intensivligiga asosan harakatni boshqarish uchun funksiya
void handleIRIntensity() {
  sendSMS("IR sensori yonmanganda o'tqan!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}

// Vibratsiya aniqlanganida harakatni boshqarish uchun funksiya
void handleVibration() {
  sendSMS("Vibratsiya aniqlandi!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}
