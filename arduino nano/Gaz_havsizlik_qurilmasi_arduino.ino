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
#define buzzer 13
#define relay1 3  // Relay 1 boshqaruv pin
#define relay2 4  // Relay 2 boshqaruv pin
#define relay3 8  // Relay 3 boshqaruv pin
#define relay4 7  // Relay 4 boshqaruv pin

// Konstantalar
#define EMERGENCY_PHONE_NUMBER "+998977477616"

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD obyekti
SoftwareSerial mySerial(10, 11); // GSM moduli uchun SoftwareSerial obyekti
SoftwareSerial espSerial(5, 6); // ESP moduli uchun SoftwareSerial obyekti
DHT dht(DHTPIN, DHTTYPE); // DHT sensori obyekti

void setup() {
  pinMode(analogSensor, INPUT);
  pinMode(IRSensor, INPUT);
  pinMode(vibrationSensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  mySerial.begin(115200);
  espSerial.begin(115200);
  mySerial.println("AT+GPS=1");
  delay(1000);
  dht.begin();
}

void loop() {


   digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  delay(2000);

  temp = dht.readTemperature();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" *C");


  int sensorValue = analogRead(analogSensor);
  gasConcentration = map(sensorValue, minValue, maxValue, 0, 100);
  lcd.setCursor(16, 1);
  lcd.print("Gaz.: ");
  lcd.print(gasConcentration);
  lcd.print("%");

  vibrationValue = analogRead(vibrationSensorPin);
  lcd.setCursor(-4, 2);
  lcd.print("Vibratsiya: ");
  lcd.print(vibrationValue);

  int irValue = analogRead(IRSensor);
  irIntensity = map(irValue, minValue, maxValue, 100, 0);
  lcd.setCursor(0, 3);
  lcd.print("IR: ");
  lcd.print(irIntensity);
  lcd.print("%");

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




  sendSensorDataToESP();

  delay(100);

  if (espSerial.available() > 0) {
    String espCommand = espSerial.readStringUntil('\n');
    Serial.println("Command from ESP: " + espCommand);

    if (espCommand.startsWith("TURN_ON_RELAY1")) {
      digitalWrite(relay1, LOW);
    } else if (espCommand.startsWith("TURN_OFF_RELAY1")) {
      digitalWrite(relay1, HIGH);
    }
    // Add more commands as needed

    espSerial.flush();
  }
 // favqulota jaroyon

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

}

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

void turnOffRelays() {
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void sendSMS(String message) {
  String smsCommand = "AT+CMGF=1";
  delay(1000);
  mySerial.println(smsCommand);
  String phoneNumberCommand = "AT+CMGS=\"";
  phoneNumberCommand.concat(EMERGENCY_PHONE_NUMBER);
  phoneNumberCommand.concat("\"");
  mySerial.println(phoneNumberCommand);
  delay(1000);
  mySerial.println(message);
  delay(100);
  mySerial.println((char)26);
  delay(1000);
}

void makeCall() {
  String callCommand = "ATD";
  callCommand.concat(EMERGENCY_PHONE_NUMBER);
  callCommand.concat(";");
  mySerial.println(callCommand);
  delay(10000);
  mySerial.println("ATH");
}

void sendLocation() {
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

void handleGasConcentration() {
   lcd.clear();
   lcdhelp();
  sendSMS("Gaz konsentratsiyasi belgilangan chegaradan oshiq!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOffRelays();

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
    lcd.clear();
}

void handleIRIntensity() {
   lcd.clear();
   lcdhelp();
  sendSMS("IR sensori yonmanganda o'tqan!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOffRelays();

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
    lcd.clear();
}

void handleVibration() {
   lcd.clear();
   lcdhelp();
  sendSMS("Vibratsiya aniqlandi!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOffRelays();

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

    lcd.clear();
}

void  lcdhelp() {
lcd.setCursor(0, 0);
  lcd.print("Siz Xaf ostidasiz");
  lcd.setCursor(-4, 1);
  lcd.print("Uyni tark eting");
  lcd.setCursor(-4, 2);
  lcd.print("Haf Haf Haf");
    lcd.setCursor(16, 3);
  lcd.print("Habar berildi");
}
