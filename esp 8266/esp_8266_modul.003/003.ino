#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> // SPIFFS kutubxonasi uchun

ESP8266WebServer server(80);

const c har* AP_ssid = "ESP8266_AP";
const char* AP_password = "12345678";

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFSni boshlashda xato");
    return;
  }

  WiFi.softAP(AP_ssid, AP_password);
  Serial.println("ESP8266 AP Mode Initiated");

  tryConnectWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/prompt", HTTP_GET, handlePrompt);
  server.on("/connect", HTTP_POST, handleConnect);
  server.on("/about", HTTP_GET, handleAbout);
  server.on("/statistics", HTTP_GET, handleStatistics);
  server.on("/settings", HTTP_GET, handleSettings);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<html><head><title>WiFi Scanner</title><style>body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 0; background: #f0f0f0; } ul { list-style-type: none; padding: 0; } li { background: #0082fc; margin: 20px auto; padding: 10px; border-radius: 5px; color: white; width: 50%; transition: background 0.5s, transform 0.5s; } li:hover { background: #0056b3; transform: scale(1.05); } a { color: white; text-decoration: none; font-weight: bold; } h1 { color: #333; } .navbar { overflow: hidden; background-color: #333; } .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; } .navbar a:hover { background-color: #ddd; color: black; }</style></head><body>";

  if (WiFi.status() == WL_CONNECTED) {
    html += "<div><strong>Connected to: </strong>" + WiFi.SSID() + "<br><strong>IP Address: </strong>" + WiFi.localIP().toString() + "</div>";
  } else {
    html += "<div>Not connected to any WiFi network</div>";
  }

  html += "<div class='navbar'><a href='/'>Home</a><a href='/about'>About</a><a href='/statistics'>Statistics</a><a href='/settings'>Settings</a></div><h1>Available WiFi Networks</h1><ul>";

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    String ssidEncoded = urlencode(ssid);
    html += "<li><a href=\"/prompt?ssid=" + ssidEncoded + "\">" + ssid + "</a></li>\n";
  }

  html += "</ul></body></html>";
  server.send(200, "text/html", html);
}

void handlePrompt() {
  String ssid = urldecode(server.arg("ssid"));
  String html = "<html><body><h1>Connect to " + ssid + "</h1><form action='/connect' method='post'><input type='hidden' name='ssid' value='" + ssid + "'>Password: <input type='password' name='password'><br><br><input type='submit' value='Connect'></form></body></html>";
  server.send(200, "text/html", html);
}

void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Faylni ochishda xato");
  } else {
    configFile.println(ssid);
    configFile.println(password);
    configFile.close();
    Serial.println("WiFi ma'lumotlari saqlandi");
  }

  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.println("Connecting to WiFi...");
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 30) {
    delay(1000);
    Serial.print(".");
    counter++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");
    server.send(200, "text/html", "<html><body><h1>Connected to " + ssid + " successfully!</h1><p>IP Address: " + WiFi.localIP().toString() + "</p></body></html>");
  } else {
    Serial.println("Failed to connect.");
    server.send(200, "text/html", "<html><body><h1>Failed to connect.</h1></body></html>");
  }
}

void handleAbout() {
  String html = "<html><body><h1>About ESP8266 Web Server</h1><p>This is a simple web server example using ESP8266. It demonstrates basic web server functionality including serving web pages and handling different routes.</p></body></html>";
  server.send(200, "text/html", html);
}

void handleStatistics() {
  String html = "<html><body><h1>System Statistics</h1><p>System Uptime: 100 hours</p><p>Handled Requests: 500</p></body></html>";
  server.send(200, "text/html", html);
}

void handleSettings() {
  String html = "<html><body><h1>Settings</h1><p>Here you can configure system settings.</p></body></html>";
  server.send(200, "text/html", html);
}

void tryConnectWiFi() {
  if (SPIFFS.exists("/config.txt")) {
    File configFile = SPIFFS.open("/config.txt", "r");
    if (configFile) {
      String ssid = configFile.readStringUntil('\n').c_str();
      ssid.trim();
      String password = configFile.readStringUntil('\n').c_str();
      password.trim();

      WiFi.begin(ssid.c_str(), password.c_str());
      Serial.print("Oldingi WiFi ma'lumotlari bilan ulanish: ");
      Serial.println(ssid);

      int counter = 0;
      while (WiFi.status() != WL_CONNECTED && counter < 30) {
        delay(1000);
        Serial.print(".");
        counter++;x`
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("Failed to connect. Falling back to AP mode.");
        WiFi.softAP(AP_ssid, AP_password);
      }
    }
    configFile.close();
  } else {
    Serial.println("WiFi konfiguratsiyasi topilmadi. AP rejimiga o'tish.");
    WiFi.softAP(AP_ssid, AP_password);
  }
}

String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
          code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
          code0 = c - 10 + 'A';
      }
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

String urldecode(String str) {
  String decodedString = "";
  char temp[] = "0x00";
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == '+') {
      decodedString += ' ';
    } else if (str[i] == '%') {
      temp[2] = str[i + 1];
      temp[3] = str[i + 2];
      decodedString += (char) strtol(temp, NULL, 16);
      i += 2;
    } else {
      decodedString += str[i];
    }
  }
  return decodedString;
}
