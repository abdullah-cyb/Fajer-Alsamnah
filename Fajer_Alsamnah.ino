/*
   ============================================================
   FAJER ALSAMNAH
   Smart Prayer Alarm - ESP32
   ============================================================

   Hardware:
   - ESP32
   - Small Buzzer

   Features:
   - WiFi connection
   - Automatic prayer times from AlAdhan API
   - Fajr, Dhuhr, Asr, Maghrib, Isha
   - Alarm ON/OFF for every prayer
   - Alarm duration
   - Different buzzer tones
   - Lightweight web interface
   - Automatic daily update
   - Yemen / Aden default location

   IMPORTANT:
   This project uses the ESP32 internal web server.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ============================================================
// WIFI SETTINGS
// ============================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ============================================================
// DEVICE SETTINGS
// ============================================================

// Buzzer GPIO
#define BUZZER_PIN 25

// Default location: Aden, Yemen
float latitude  = 12.785496;
float longitude = 45.018654;

// GMT +3 Yemen
const long GMT_OFFSET_SEC = 3 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

// ============================================================
// WEB SERVER
// ============================================================

WebServer server(80);

// ============================================================
// PRAYER DATA
// ============================================================

struct Prayer {
  String name;
  String time;
  bool enabled;
};

Prayer prayers[5] = {
  {"الفجر", "--:--", true},
  {"الظهر", "--:--", true},
  {"العصر", "--:--", true},
  {"المغرب", "--:--", true},
  {"العشاء", "--:--", true}
};

// ============================================================
// ALARM SETTINGS
// ============================================================

int alarmDuration = 30;

// Tone:
// 0 = Simple
// 1 = Fast
// 2 = Slow
// 3 = Triple
int alarmTone = 0;

bool alarmRunning = false;

unsigned long alarmStartMillis = 0;
unsigned long lastToneMillis = 0;

int currentAlarmIndex = -1;
int currentToneStep = 0;

// ============================================================
// API UPDATE
// ============================================================

unsigned long lastApiUpdate = 0;

// Update every 6 hours
const unsigned long API_UPDATE_INTERVAL = 6UL * 60UL * 60UL * 1000UL;

// ============================================================
// LAST TRIGGER
// ============================================================

String lastTriggeredPrayer = "";
String lastTriggeredDate = "";

// ============================================================
// URL ENCODING
// ============================================================

String urlEncode(String text) {

  String encoded = "";

  char c;
  char code0;
  char code1;

  for (int i = 0; i < text.length(); i++) {

    c = text.charAt(i);

    if (isalnum(c)) {
      encoded += c;
    }

    else {

      code1 = (c & 0xF) + '0';

      if ((c & 0xF) > 9)
        code1 = (c & 0xF) - 10 + 'A';

      c = (c >> 4) & 0xF;

      code0 = c + '0';

      if (c > 9)
        code0 = c - 10 + 'A';

      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }

  return encoded;
}

// ============================================================
// CLEAN PRAYER TIME
// ============================================================

String cleanPrayerTime(String value) {

  int spaceIndex = value.indexOf(' ');

  if (spaceIndex != -1) {
    value = value.substring(0, spaceIndex);
  }

  return value;
}

// ============================================================
// GET CURRENT DATE
// ============================================================

String getCurrentDate() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return "";
  }

  char dateBuffer[20];

  strftime(
    dateBuffer,
    sizeof(dateBuffer),
    "%Y-%m-%d",
    &timeinfo
  );

  return String(dateBuffer);
}

// ============================================================
// GET CURRENT TIME
// ============================================================

String getCurrentTime() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    return "--:--";
  }

  char timeBuffer[10];

  strftime(
    timeBuffer,
    sizeof(timeBuffer),
    "%H:%M",
    &timeinfo
  );

  return String(timeBuffer);
}

// ============================================================
// CONNECT WIFI
// ============================================================

void connectWiFi() {

  Serial.println();
  Serial.println("=================================");
  Serial.println("FAJER ALSAMNAH");
  Serial.println("Connecting to WiFi...");
  Serial.println("=================================");

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  int attempts = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 40
  ) {

    delay(500);

    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  }

  else {

    Serial.println("WiFi connection failed.");
    Serial.println("The device will continue running.");
  }
}

// ============================================================
// GET PRAYER TIMES FROM API
// ============================================================

bool updatePrayerTimes() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println(
      "WiFi not connected. Cannot update prayer times."
    );

    return false;
  }

  HTTPClient http;

  String url =
    "https://api.aladhan.com/v1/timings"
    "?latitude=" +
    String(latitude, 6) +
    "&longitude=" +
    String(longitude, 6) +
    "&method=4";

  Serial.println();
  Serial.println("Updating prayer times...");
  Serial.println(url);

  http.begin(url);

  http.setTimeout(15000);

  int httpCode = http.GET();

  if (httpCode <= 0) {

    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpCode));

    http.end();

    return false;
  }

  if (httpCode != 200) {

    Serial.print("API returned HTTP code: ");
    Serial.println(httpCode);

    http.end();

    return false;
  }

  String payload = http.getString();

  http.end();

  Serial.println("Prayer API response received.");

  DynamicJsonDocument doc(16000);

  DeserializationError error =
    deserializeJson(doc, payload);

  if (error) {

    Serial.print(
      "JSON parsing failed: "
    );

    Serial.println(error.c_str());

    return false;
  }

  JsonObject timings =
    doc["data"]["timings"];

  if (timings.isNull()) {

    Serial.println(
      "Prayer timings not found."
    );

    return false;
  }

  prayers[0].time =
    cleanPrayerTime(
      timings["Fajr"].as<String>()
    );

  prayers[1].time =
    cleanPrayerTime(
      timings["Dhuhr"].as<String>()
    );

  prayers[2].time =
    cleanPrayerTime(
      timings["Asr"].as<String>()
    );

  prayers[3].time =
    cleanPrayerTime(
      timings["Maghrib"].as<String>()
    );

  prayers[4].time =
    cleanPrayerTime(
      timings["Isha"].as<String>()
    );

  Serial.println();
  Serial.println("Today's Prayer Times:");

  for (int i = 0; i < 5; i++) {

    Serial.print(
      prayers[i].name
    );

    Serial.print(" : ");

    Serial.println(
      prayers[i].time
    );
  }

  lastApiUpdate = millis();

  return true;
}

// ============================================================
// START BUZZER
// ============================================================

void startAlarm(int prayerIndex) {

  if (prayerIndex < 0 || prayerIndex >= 5) {
    return;
  }

  if (!prayers[prayerIndex].enabled) {

    Serial.print(
      "Alarm disabled for: "
    );

    Serial.println(
      prayers[prayerIndex].name
    );

    return;
  }

  if (alarmRunning) {
    return;
  }

  alarmRunning = true;

  currentAlarmIndex = prayerIndex;

  currentToneStep = 0;

  alarmStartMillis = millis();

  lastToneMillis = 0;

  Serial.println();
  Serial.println("=================================");
  Serial.print("ALARM: ");
  Serial.println(
    prayers[prayerIndex].name
  );
  Serial.println("=================================");
}

// ============================================================
// STOP BUZZER
// ============================================================

void stopAlarm() {

  noTone(BUZZER_PIN);

  alarmRunning = false;

  currentAlarmIndex = -1;

  currentToneStep = 0;
}

// ============================================================
// ALARM TONE ENGINE
// ============================================================

void processAlarmTone() {

  if (!alarmRunning) {
    return;
  }

  unsigned long elapsed =
    millis() - alarmStartMillis;

  if (
    elapsed >=
    ((unsigned long)alarmDuration * 1000UL)
  ) {

    stopAlarm();

    Serial.println(
      "Alarm finished."
    );

    return;
  }

  unsigned long now =
    millis();

  if (
    now - lastToneMillis < 500
  ) {
    return;
  }

  lastToneMillis = now;

  switch (alarmTone) {

    case 0:

      tone(
        BUZZER_PIN,
        1000,
        350
      );

      break;

    case 1:

      tone(
        BUZZER_PIN,
        1800,
        200
      );

      break;

    case 2:

      tone(
        BUZZER_PIN,
        700,
        450
      );

      break;

    case 3:

      tone(
        BUZZER_PIN,
        1200,
        150
      );

      delay(180);

      tone(
        BUZZER_PIN,
        1600,
        150
      );

      delay(180);

      tone(
        BUZZER_PIN,
        2000,
        150
      );

      break;

    default:

      tone(
        BUZZER_PIN,
        1000,
        300
      );

      break;
  }
}

// ============================================================
// CHECK PRAYER TIME
// ============================================================

void checkPrayerAlarms() {

  String nowTime =
    getCurrentTime();

  String today =
    getCurrentDate();

  if (
    nowTime == "--:--" ||
    today == ""
  ) {
    return;
  }

  for (int i = 0; i < 5; i++) {

    if (!prayers[i].enabled) {
      continue;
    }

    if (
      prayers[i].time == nowTime
    ) {

      String triggerID =
        today + "_" + String(i);

      if (
        lastTriggeredDate !=
        triggerID
      ) {

        lastTriggeredDate =
          triggerID;

        lastTriggeredPrayer =
          prayers[i].name;

        startAlarm(i);
      }
    }
  }
}

// ============================================================
// HTML PAGE
// ============================================================

String buildWebPage() {

  String html;

  html.reserve(12000);

  html += R"rawliteral(
<!DOCTYPE html>
<html lang="ar" dir="rtl">

<head>

<meta charset="UTF-8">

<meta name="viewport"
content="width=device-width,
initial-scale=1.0">

<title>Fajer Alsamnah</title>

<style>

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #f5f7fa;
  color: #222;
}

.container {
  width: 94%;
  max-width: 600px;
  margin: auto;
  padding: 15px;
}

.header {
  background: #111827;
  color: white;
  padding: 20px;
  border-radius: 15px;
  text-align: center;
  margin-bottom: 15px;
}

.header h1 {
  margin: 0 0 8px 0;
  font-size: 25px;
}

.header p {
  margin: 0;
  opacity: 0.8;
}

.card {
  background: white;
  border-radius: 15px;
  padding: 15px;
  margin-bottom: 15px;
  box-shadow:
    0 2px 10px rgba(0,0,0,0.06);
}

.prayer {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 10px;
  padding: 14px;
  margin-bottom: 8px;
  border-radius: 12px;
  background: #f3f4f6;
}

.prayer:last-child {
  margin-bottom: 0;
}

.prayer-name {
  font-weight: bold;
  font-size: 17px;
}

.prayer-time {
  font-size: 18px;
  font-weight: bold;
}

button {
  border: none;
  border-radius: 10px;
  padding: 9px 13px;
  cursor: pointer;
  font-size: 14px;
}

.on {
  background: #16a34a;
  color: white;
}

.off {
  background: #dc2626;
  color: white;
}

select,
input {
  width: 100%;
  padding: 11px;
  border: 1px solid #ddd;
  border-radius: 10px;
  margin-top: 7px;
  background: white;
}

label {
  display: block;
  margin-top: 12px;
  font-weight: bold;
}

.refresh {
  width: 100%;
  background: #111827;
  color: white;
  margin-top: 15px;
  padding: 13px;
}

.status {
  text-align: center;
  font-size: 14px;
  color: #666;
}

.footer {
  text-align: center;
  color: #777;
  font-size: 12px;
  padding: 15px;
}

</style>

</head>

<body>

<div class="container">

<div class="header">

<h1>فجر الصمّانة</h1>

<p>منبه أوقات الصلاة الذكي</p>

</div>

<div class="card">

<h3>🕌 أوقات الصلاة</h3>

)rawliteral";

  for (int i = 0; i < 5; i++) {

    html +=
      "<div class='prayer'>";

    html +=
      "<div>";

    html +=
      "<div class='prayer-name'>" +
      prayers[i].name +
      "</div>";

    html +=
      "<div class='prayer-time'>" +
      prayers[i].time +
      "</div>";

    html +=
      "</div>";

    html +=
      "<button class='" +
      String(
        prayers[i].enabled
        ? "on"
        : "off"
      ) +
      "' onclick='toggleAlarm(" +
      String(i) +
      ")'>";

    html +=
      prayers[i].enabled
      ? "المنبه يعمل"
      : "المنبه متوقف";

    html +=
      "</button>";

    html += "</div>";
  }

  html += R"rawliteral(

</div>

<div class="card">

<h3>⏰ إعدادات المنبه</h3>

<label>
مدة المنبه بالثواني
</label>

<select id="duration"
onchange="setDuration(this.value)">

<option value="10">10 ثواني</option>
<option value="20">20 ثانية</option>
<option value="30">30 ثانية</option>
<option value="60">60 ثانية</option>
<option value="120">120 ثانية</option>

</select>

<label>
نوع النغمة
</label>

<select id="tone"
onchange="setTone(this.value)">

<option value="0">نغمة بسيطة</option>
<option value="1">نغمة سريعة</option>
<option value="2">نغمة هادئة</option>
<option value="3">نغمة متقطعة</option>

</select>

<button class="refresh"
onclick="refreshPage()">

🔄 تحديث أوقات الصلاة

</button>

</div>

<div class="card">

<div class="status">

<div>
الوقت الحالي:
<span id="clock">--:--</span>
</div>

<div>
آخر تحديث من API:
<span id="apiStatus">جارٍ التحقق...</span>
</div>

</div>

</div>

<div class="footer">

Fajer Alsamnah<br>
ESP32 Smart Prayer Alarm

</div>

</div>

<script>

function toggleAlarm(index) {

  fetch(
    '/toggle?index=' + index
  )
  .then(() => {
    location.reload();
  });

}

function setDuration(value) {

  fetch(
    '/duration?value=' + value
  );

}

function setTone(value) {

  fetch(
    '/tone?value=' + value
  );

}

function refreshPage() {

  fetch('/update')
    .then(() => {
      location.reload();
    });

}

function updateClock() {

  fetch('/time')
    .then(response => response.text())
    .then(time => {

      document.getElementById(
        'clock'
      ).innerText = time;

    });

}

setInterval(
  updateClock,
  1000
);

updateClock();

</script>

</body>

</html>

)rawliteral";

  return html;
}

// ============================================================
// WEB ROUTE: HOME
// ============================================================

void handleRoot() {

  server.send(
    200,
    "text/html; charset=UTF-8",
    buildWebPage()
  );
}

// ============================================================
// WEB ROUTE: TOGGLE ALARM
// ============================================================

void handleToggle() {

  if (!server.hasArg("index")) {

    server.send(
      400,
      "text/plain",
      "Missing index"
    );

    return;
  }

  int index =
    server.arg("index").toInt();

  if (
    index < 0 ||
    index >= 5
  ) {

    server.send(
      400,
      "text/plain",
      "Invalid index"
    );

    return;
  }

  prayers[index].enabled =
    !prayers[index].enabled;

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// ============================================================
// WEB ROUTE: DURATION
// ============================================================

void handleDuration() {

  if (!server.hasArg("value")) {

    server.send(
      400,
      "text/plain",
      "Missing value"
    );

    return;
  }

  int value =
    server.arg("value").toInt();

  if (value < 5) {
    value = 5;
  }

  if (value > 300) {
    value = 300;
  }

  alarmDuration = value;

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// ============================================================
// WEB ROUTE: TONE
// ============================================================

void handleTone() {

  if (!server.hasArg("value")) {

    server.send(
      400,
      "text/plain",
      "Missing value"
    );

    return;
  }

  int value =
    server.arg("value").toInt();

  if (value < 0 || value > 3) {
    value = 0;
  }

  alarmTone = value;

  server.send(
    200,
    "text/plain",
    "OK"
  );
}

// ============================================================
// WEB ROUTE: CURRENT TIME
// ============================================================

void handleTime() {

  server.send(
    200,
    "text/plain",
    getCurrentTime()
  );
}

// ============================================================
// WEB ROUTE: UPDATE API
// ============================================================

void handleUpdate() {

  bool success =
    updatePrayerTimes();

  if (success) {

    server.send(
      200,
      "text/plain",
      "Updated"
    );

  }

  else {

    server.send(
      500,
      "text/plain",
      "Update failed"
    );
  }
}

// ============================================================
// SETUP WEB SERVER
// ============================================================

void setupWebServer() {

  server.on(
    "/",
    HTTP_GET,
    handleRoot
  );

  server.on(
    "/toggle",
    HTTP_GET,
    handleToggle
  );

  server.on(
    "/duration",
    HTTP_GET,
    handleDuration
  );

  server.on(
    "/tone",
    HTTP_GET,
    handleTone
  );

  server.on(
    "/time",
    HTTP_GET,
    handleTime
  );

  server.on(
    "/update",
    HTTP_GET,
    handleUpdate
  );

  server.begin();

  Serial.println(
    "Web server started."
  );
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  noTone(
    BUZZER_PIN
  );

  connectWiFi();

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    "pool.ntp.org",
    "time.nist.gov"
  );

  Serial.println(
    "Synchronizing time..."
  );

  delay(2000);

  updatePrayerTimes();

  setupWebServer();

  Serial.println();
  Serial.println(
    "================================="
  );

  Serial.println(
    "FAJER ALSAMNAH READY"
  );

  if (
    WiFi.status() == WL_CONNECTED
  ) {

    Serial.print(
      "Open: http://"
    );

    Serial.println(
      WiFi.localIP()
    );
  }

  Serial.println(
    "================================="
  );
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  server.handleClient();

  processAlarmTone();

  checkPrayerAlarms();

  // Update prayer times periodically
  if (
    WiFi.status() == WL_CONNECTED &&
    millis() - lastApiUpdate >
    API_UPDATE_INTERVAL
  ) {

    updatePrayerTimes();
  }

  delay(10);
}
