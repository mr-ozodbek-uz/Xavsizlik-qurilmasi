#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "YourESP8266AP";
const char *password = "YourPassword";

ESP8266WebServer server(80);

String receivedIR = "";
String receivedMQ9 = "";
String receivedHarorat = "";
String receivedBosim = "";

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if (Serial.available()) {
    String message = Serial.readString();
    Serial.println("Qabul qilingan xabar: " + message);
    parseData(message);
  }
}

void handleUpdate() {
  // Update logic here
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


// Yangilanish Yes js





void handleRoot() {
  String html = R"(
    <html>
      <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>HQ Havsizlik qurilmasi</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f0f0f0;
          }
          .container {
            width: 90%;
            margin: auto;
            overflow: hidden;
          }
          h1 {
            padding: 20px 0;
            background-color: #333;
            color: #fff;
            text-align: center;
          }
          h2 {
            background: #ddd;
            padding: 10px;
            margin: 10px 0;
            border-radius: 5px;
          }
        </style>
        <script>
          function updateValues() {
            var irValue = document.getElementById('irValue');
            irValue.innerText = '" + receivedIR + "'; // Update with the receivedIR value
          }

          setInterval(updateValues, 1000); // Update every 1000 milliseconds (1 second)
        </script>
      </head>
      <body>
        <div class="container">
          <h1>Uyingiz malumotlari</h1>
          <h2 id="irValue">Initial IR Value</h2>
        </div>
      </body>
    </html>
  )";

  server.send(200, "text/html", html);
}


// Eski korinish no js



// void handleRoot() {
//   String html = R"(
//     <html>
//       <head>
//         <meta charset="UTF-8" />
//         <meta name="viewport" content="width=device-width, initial-scale=1.0" />
//         <title>HQ Havsizlik qurilmasi</title>
//         <style>
//           body {
//             font-family: Arial, sans-serif;
//             margin: 0;
//             padding: 0;
//             background-color: #f0f0f0;
//           }
//           .container {
//             width: 90%;
//             margin: auto;
//             overflow: hidden;
//           }
//           h1 {
//             padding: 20px 0;
//             background-color: #333;
//             color: #fff;
//             text-align: center;
//           }
//           h2 {
//             background: #ddd;
//             padding: 10px;
//             margin: 10px 0;
//             border-radius: 5px;
//           }
//         </style>
//       </head>
//       <body>
//         <div class="container">
//           <h1>Uyingiz malumotlari</h1>
//           <h2>)" + receivedIR + R"(</h2>
//         </div>
//       </body>
//     </html>
//   )";

//   server.send(200, "text/html", html);
// }