#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>

float temp;
float humidity;
float gasConcentration;
float irIntensity;
int vibrationValue;

#define analogSensor A0
#define minValue 0
#define maxValue 1023
#define threshold 50
#define DHTPIN 2
#define DHTTYPE DHT22
#define IRSensor A1
#define vibrationSensorPin A3
#define buzzer 8
#define relay1 3  // Rel1 IN1 -> Pin 3
#define relay2 4  // Rel2 IN2 -> Pin 4
#define relay3 8  // Rel3 IN3 -> Pin 5
#define relay4 7  // Rel4 IN4 -> Pin 9

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(10, 11);
SoftwareSerial espSerial(5, 6);
DHT dht(DHTPIN, DHTTYPE);

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
  delay(2000);

  temp = dht.readTemperature();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" *C");

  int sensorValue = analogRead(analogSensor);
  gasConcentration = map(sensorValue, minValue, maxValue, 0, 100);
  lcd.setCursor(0, 1);
  lcd.print("Gas hafi: ");
  lcd.print(gasConcentration);
  lcd.print("%");

  vibrationValue = analogRead(vibrationSensorPin);
  lcd.setCursor(0, 2);
  lcd.print("Vibration: ");
  lcd.print(vibrationValue);

  int irValue = analogRead(IRSensor);
  irIntensity = map(irValue, minValue, maxValue, 100, 0);
  lcd.setCursor(-4, 3);
  lcd.print("IR intensity: ");
  lcd.print(irIntensity);
  lcd.print("%");

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

  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" *C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Gas Concentration: ");
  Serial.print(gasConcentration);
  Serial.println("%");
  Serial.print("IR Intensity: ");
  Serial.println(irIntensity);
  Serial.print("Vibration: ");
  Serial.println((vibrationValue > threshold) ? "Detected" : "Not detected");

  sendSensorDataToESP();

  delay(100);
}




void sendSensorDataToESP() {
  espSerial.print("IR:      ");
  espSerial.println(irIntensity);
  espSerial.print("MQ9:     ");
  espSerial.println(gasConcentration);
  espSerial.print("Temperature: ");
  espSerial.println(temp);
  espSerial.print("Humidity: ");
  espSerial.println(humidity);
  espSerial.print("Vibration: ");
  espSerial.println((vibrationValue == HIGH) ? "Detected" : "Not detected");
}

void sendSMS(String message) {
  mySerial.println("AT+CMGF=1");
  delay(1000);
  mySerial.println("AT+CMGS=\"+998xxxxxxxxx\"");
  delay(1000);
  mySerial.println(message);
  delay(100);
  mySerial.println((char)26);
  delay(1000);
}

void makeCall() {
  mySerial.println("ATD+998xxxxxxxxx;");
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
      sendSMS("Location: " + latitude + ", " + longitude);
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


void turnOnRelays() {
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
}

void turnOffRelays() {
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void handleGasConcentration() {
  sendSMS("Gas concentration is above the threshold!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}

void handleIRIntensity() {
  sendSMS("IR sensor detected fire!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}

void handleVibration() {
  sendSMS("Vibration detected!");
  delay(3000);
  makeCall();
  delay(3000);
  sendLocation();
  activateBuzzer();
  turnOnRelays();
}