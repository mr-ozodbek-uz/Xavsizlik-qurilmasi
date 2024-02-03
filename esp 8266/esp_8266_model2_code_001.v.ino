#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "Home_1";       // WiFi nomi
const char *password = "977477616"; // WiFi paroli

ESP8266WebServer server(80);

String receivedIR = "";
String receivedMQ9 = "";
String receivedHarorat = "";
String receivedBosim = "";

void setup() {
  Serial.begin(115200);

  // WiFi ulanish
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi ulanmagan...");
  }
  Serial.println("WiFi muvaffaqiyatli ulandi.");

  // IP manzilini olish
  Serial.println("WiFi modulning IP manzili: " + WiFi.localIP().toString());

  // Web serverni boshlash
  server.on("/", HTTP_GET, handleRoot);
  server.on("/pressed", HTTP_GET, handleButtonPress);
  server.begin();
  Serial.println("HTTP server ishga tushirildi.");
}

void loop() {
  server.handleClient();
  if (Serial.available()) {
    String message = Serial.readString();
    Serial.println("Qabul qilingan xabar: " + message);
    parseData(message);
  }
}

void handleRoot() {
  String html = "<html><head>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f0f0f0; animation: fadeIn 2s ease-in-out; }";
  html += ".container { width: 90%; margin: auto; overflow: hidden; animation: slideIn 1.5s ease-out; }";
  html += "h1 { padding: 20px 0; background-color: #333; color: #fff; text-align: center; animation: fadeInUp 1.5s ease-in-out; }";
  html += "h2 { background: #ddd; padding: 10px; margin: 10px 0; border-radius: 5px; animation: scaleIn 1s ease-in-out; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>Uyingiz malumotlari</h1>";
  html += "<h2 id='sensorValue'>" + receivedIR + "</h2>";
  html += "<button onclick=\"sendData()\">Bosish</button>";
  html += "<script>function sendData() { var xhr = new XMLHttpRequest(); xhr.open('GET', '/pressed', true); xhr.send(); }</script>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

void handleButtonPress() {
  Serial.println("Tugma bosildi");
  server.send(200, "text/plain", "OK");
}

void parseData(String message) {
  int firstCommaIndex = message.indexOf(',');
  receivedIR = message.substring(0, firstCommaIndex);
  message = message.substring(firstCommaIndex + 1);

  int secondCommaIndex = message.indexOf(',');
  receivedMQ9 = message.substring(0, secondCommaIndex);
  message = message.substring(secondCommaIndex + 1);

  int thirdCommaIndex = message.indexOf(',');
  receivedHarorat = message.substring(0, thirdCommaIndex);
  receivedBosim = message.substring(thirdCommaIndex + 1);
}
