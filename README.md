
# Xavsizlik-qurilmasi  IoT Loyihasi

Bu kod `Hoshimov Ozodbek Huanivch` tomonidan yaratilgan va patentlangan manba

Bu kodalarda Arduino va ESP8266 modullarini birlashtirgan bir IoT loyihasi mavjud. Ushbu loyiha uyg'unlik sensorlardan malumot olish, ularni ESP8266 moduli orqali bir serverga yuborish, serverdan esa web brauzer orqali bu malumotlarni ko'rish imkoniyatini beradi.

## Arduino kodi:

Arduino kodi, har bir sensorning malumotini o'qib oladi va uni ESP8266 moduliga yuboradi. Sensorlar orqali olingan malumotlar serverga yuboriladi va shunday qilib, uyingizdagi holat haqida ma'lumotni saqlaygan serverga aloqa o'rnatiladi.

### Sensorlar va ulardan olingan ma'lumotlar:

- DHT22 sensor orqali havo harorati va namlik.
- Analog sensor orqali gaz konsentratsiyasi.
- IR sensor orqali infrachiziqlar intensivligi.
- Vibration sensor orqali vibratsiya intensivligi.

### Funksiyalar:

- `sendSensorDataToESP()`: ESP8266 moduliga sensor ma'lumotlarini yuborish.
- `sendSMS()`: SMS yuborish.
- `makeCall()`: Qo'ng'iroq qilish.
- `sendLocation()`: Lokatsiyani yuborish.
- `activateBuzzer()`: Buzzer ni yoqish.
- `turnOnRelays()` va `turnOffRelays()`: Relaysni yoqish va o'chirish.
- `handleGasConcentration()`, `handleIRIntensity()`, `handleVibration()`: Har bir holat uchun ma'lumot yuborish va qo'ng'iroq qilish.

### ESP8266 kodi:

ESP8266 kodi, Arduino modulidan olingan ma'lumotlarni qabul qilib, ularni web server orqali brauzerga ko'rsatadi.

### Web server va malumotlarni olish:

- ESP8266WebServer bilan server yaratilgan.
- `/` va `/update` manzillariga kelgan so'rovlarni qayta ishlaydigan funksiyalar mavjud.
- Arduino modulidan kelgan ma'lumotlar `/update` manziliga o'tkaziladi.
- `parseData()` funksiyasi orqali ma'lumotlar ajratiladi.

### Web sayt:

- `handleRoot()` funksiyasi orqali server brauzerga HTML sahifasini yuboradi.
- Brauzerda `/` manziliga so'rov kelganida, ESP8266 serveri olingan ma'lumotlarni HTML sahifasida ko'rsatadi.

### Ma'lumotlar almashinuvi:

- Arduino modulidan olingan ma'lumotlar ESP8266 moduliga Serial bilan yuboriladi.
- ESP8266 moduli esa Serial bilan o'qib oladi va ma'lumotlarni qabul qilar.

### Sozlamalar:

- WiFi sozlamalari (`ssid` va `password`) serverni yaratishda ishlatilgan.

Bu kodlarni ishga tushirish uchun, Arduino va ESP8266 modullarini moslashtirilgan holda birlashtiring va serverga ulangan WiFi tarmog'i orqali bog'langan holda web brauzerda ko'rish uchun tayyor.