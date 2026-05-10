#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define HEADER_BOTTOM_Y 13
#define FOOTER_TOP_Y 52

// Credenziali WiFi
#define WIFI_SSID "gianiaz24"
#define WIFI_PASSWORD "dalgabon76"
#define WIFI_CONNECT_TIMEOUT_MS 60000
#define WIFI_CONNECT_LOG_INTERVAL_MS 2000
#define WIFI_CONNECT_ANIMATION_MS 250

// Broker MQTT
#define MQTT_HOST "192.168.1.50"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_user"
#define MQTT_PASSWORD "dalgabon"
#define MQTT_CLIENT_ID "eeg-monitor-esp32c3"
#define MQTT_TOPIC "brain/data"
#define MQTT_CONNECT_TIMEOUT_MS 10000
#define MQTT_RECONNECT_INTERVAL_MS 10000
#define MQTT_PUBLISH_INTERVAL_MS 2000

// Configurazione pin I2C per ESP32-C3 Super Mini
#define I2C_SDA 8
#define I2C_SCL 9

// Joystick analogico
#define JOY_X_PIN 0
#define JOY_Y_PIN 1

// Il joystick viene calibrato all'avvio: lascialo fermo al centro mentre accendi.
// Se un verso risulta invertito, scambia LOW/HIGH nella readJoystick().
#define ADC_MAX_VALUE 4095
#define JOY_MOVE_PERCENT 55
#define JOY_MIN_MOVE_DELTA 250
#define JOY_MAX_MOVE_DELTA 900
#define JOY_RELEASE_DELTA 180
#define JOY_DEBUG_INTERVAL_MS 300

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

enum Direction {
  DIR_NONE,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_UP
};

enum Screen {
  SCREEN_MAIN_MENU,
  SCREEN_MODE_MENU,
  SCREEN_RUNNING
};

enum WifiUiState {
  WIFI_UI_DISCONNECTED,
  WIFI_UI_CONNECTING,
  WIFI_UI_CONNECTED
};

const char *mainMenuItems[] = {
  "",
  "Modalita"
};

const char *modeMenuItems[] = {
  "Meditazione",
  "Lettura",
  "Videogame"
};

const size_t mainMenuCount = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);
const size_t modeMenuCount = sizeof(modeMenuItems) / sizeof(modeMenuItems[0]);

Screen currentScreen = SCREEN_MAIN_MENU;
WifiUiState wifiUiState = WIFI_UI_DISCONNECTED;
size_t mainMenuIndex = 0;
size_t modeMenuIndex = 0;
const char *selectedActivity = "Meditazione";
bool activityRunning = false;
volatile int lastWifiDisconnectReason = -1;

Direction lastDirection = DIR_NONE;
bool waitingForJoystickRelease = false;
int joystickCenterX = 2048;
int joystickCenterY = 2048;
int joystickLeftDelta = 900;
int joystickRightDelta = 900;
int joystickUpDelta = 900;
int joystickDownDelta = 900;
unsigned long lastJoystickDebugAt = 0;
unsigned long lastMqttPublishAt = 0;
unsigned long lastMqttConnectAttemptAt = 0;
int curAtt = 50;
int curMed = 50;
float currentWaves[8] = {
  200000.0,
  90000.0,
  45000.0,
  38000.0,
  22000.0,
  16000.0,
  9000.0,
  7000.0
};

const char *directionName(Direction direction);
const char *currentItemName();

void drawCenteredText(const char *text, int16_t y, uint8_t textSize) {
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;

  display.setTextSize(textSize);
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}

void drawWifiDisconnectedIcon(int16_t x, int16_t y) {
  display.drawRect(x, y + 7, 3, 3, SSD1306_WHITE);
  display.drawRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  display.drawRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
  display.drawLine(x - 1, y, x + 15, y + 10, SSD1306_WHITE);
}

void drawWifiConnectedIcon(int16_t x, int16_t y) {
  display.fillRect(x, y + 7, 3, 3, SSD1306_WHITE);
  display.fillRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  display.fillRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
}

void drawWifiConnectingIcon(int16_t x, int16_t y) {
  const uint8_t step = (millis() / WIFI_CONNECT_ANIMATION_MS) % 4;

  display.drawRect(x, y + 7, 3, 3, SSD1306_WHITE);
  display.drawRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  display.drawRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);

  if (step >= 1) {
    display.fillRect(x, y + 7, 3, 3, SSD1306_WHITE);
  }
  if (step >= 2) {
    display.fillRect(x + 5, y + 4, 3, 6, SSD1306_WHITE);
  }
  if (step >= 3) {
    display.fillRect(x + 10, y + 1, 3, 9, SSD1306_WHITE);
  }
}

void drawWifiIcon(int16_t x, int16_t y) {
  switch (wifiUiState) {
    case WIFI_UI_CONNECTED:
      drawWifiConnectedIcon(x, y);
      break;
    case WIFI_UI_CONNECTING:
      drawWifiConnectingIcon(x, y);
      break;
    case WIFI_UI_DISCONNECTED:
      drawWifiDisconnectedIcon(x, y);
      break;
  }
}

void drawHeader() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 1);
  display.print("EEG Monitor");
  display.drawLine(74, 0, 74, 9, SSD1306_WHITE);
  drawWifiIcon(88, 1);
}

void drawFrame() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawHeader();
  display.drawLine(0, HEADER_BOTTOM_Y, SCREEN_WIDTH - 1, HEADER_BOTTOM_Y, SSD1306_WHITE);
  display.drawLine(0, FOOTER_TOP_Y, SCREEN_WIDTH - 1, FOOTER_TOP_Y, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Mode: ");
  display.print(selectedActivity);
}

void drawMainMenu() {
  drawFrame();

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print(activityRunning ? "Avviata:" : "Attivita:");
  display.setCursor(58, 16);
  display.print(selectedActivity);

  drawCenteredText(currentItemName(), 31, 2);
  display.display();
}

void drawModeMenu() {
  drawFrame();

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("Modalita");

  drawCenteredText(modeMenuItems[modeMenuIndex], 31, 2);
  display.display();
}

void drawRunning() {
  drawFrame();

  display.setTextSize(1);
  display.setCursor(0, 16);
  display.print("Routine avviata");

  drawCenteredText(selectedActivity, 31, 2);
  display.display();
}

void drawCurrentScreen() {
  switch (currentScreen) {
    case SCREEN_MAIN_MENU:
      drawMainMenu();
      break;
    case SCREEN_MODE_MENU:
      drawModeMenu();
      break;
    case SCREEN_RUNNING:
      drawRunning();
      break;
  }
}

const char *wifiStatusName(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "WL_STATUS_SCONOSCIUTO";
  }
}

const char *wifiDisconnectReasonName(int reason) {
  switch (reason) {
    case 2:
      return "AUTH_EXPIRE";
    case 4:
      return "ASSOC_EXPIRE";
    case 8:
      return "ASSOC_LEAVE";
    case 15:
      return "4WAY_HANDSHAKE_TIMEOUT";
    case 202:
      return "AUTH_FAIL";
    case 201:
      return "NO_AP_FOUND";
    case 204:
      return "HANDSHAKE_TIMEOUT";
    default:
      return "SCONOSCIUTO";
  }
}

void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println(F("WiFi event: STA_START"));
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println(F("WiFi event: STA_CONNECTED"));
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print(F("WiFi event: GOT_IP | IP: "));
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      lastWifiDisconnectReason = info.wifi_sta_disconnected.reason;
      Serial.print(F("WiFi event: STA_DISCONNECTED | reason: "));
      Serial.print(lastWifiDisconnectReason);
      Serial.print(F(" ("));
      Serial.print(wifiDisconnectReasonName(lastWifiDisconnectReason));
      Serial.println(F(")"));
      break;
    default:
      break;
  }
}

void scanWifiForDebug() {
  Serial.println(F("WiFi scan: avvio"));
  const int networkCount = WiFi.scanNetworks(false, true);

  if (networkCount < 0) {
    Serial.print(F("WiFi scan: errore: "));
    Serial.println(networkCount);
    return;
  }

  Serial.print(F("WiFi scan: reti trovate: "));
  Serial.println(networkCount);

  bool foundConfiguredSsid = false;
  for (int i = 0; i < networkCount; i++) {
    const String ssid = WiFi.SSID(i);
    const bool isConfiguredSsid = ssid == WIFI_SSID;

    if (isConfiguredSsid) {
      foundConfiguredSsid = true;
    }

    Serial.print(F("WiFi scan: "));
    Serial.print(isConfiguredSsid ? F("* ") : F("  "));
    Serial.print(ssid);
    Serial.print(F(" | RSSI: "));
    Serial.print(WiFi.RSSI(i));
    Serial.print(F(" | channel: "));
    Serial.print(WiFi.channel(i));
    Serial.print(F(" | encryption: "));
    Serial.println(WiFi.encryptionType(i));
  }

  Serial.print(F("WiFi scan: SSID configurato trovato: "));
  Serial.println(foundConfiguredSsid ? F("si") : F("no"));
  WiFi.scanDelete();
}

void drawWifiStartupScreen(unsigned long elapsedMs) {
  drawFrame();

  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print("Connessione WiFi");

  display.setCursor(0, 30);
  display.print(WIFI_SSID);

  display.setCursor(0, 42);
  display.print("Timeout ");
  display.print((WIFI_CONNECT_TIMEOUT_MS - elapsedMs) / 1000);
  display.print("s");

  display.display();
}

void connectToWifi() {
  wifiUiState = WIFI_UI_CONNECTING;
  lastWifiDisconnectReason = -1;

  Serial.println(F("WiFi: avvio connessione"));
  Serial.print(F("WiFi: SSID: "));
  Serial.println(WIFI_SSID);
  Serial.print(F("WiFi: timeout ms: "));
  Serial.println(WIFI_CONNECT_TIMEOUT_MS);

  WiFi.onEvent(onWifiEvent);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.disconnect(true, true);
  delay(300);

  Serial.print(F("WiFi: MAC STA: "));
  Serial.println(WiFi.macAddress());

  scanWifiForDebug();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long startedAt = millis();
  unsigned long lastLogAt = 0;
  unsigned long lastDrawAt = 0;
  wl_status_t lastStatus = WL_IDLE_STATUS;

  while (millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    const unsigned long now = millis();
    const wl_status_t status = WiFi.status();

    if (status != lastStatus || now - lastLogAt >= WIFI_CONNECT_LOG_INTERVAL_MS) {
      Serial.print(F("WiFi: stato: "));
      Serial.print(wifiStatusName(status));
      Serial.print(F(" | elapsed ms: "));
      Serial.println(now - startedAt);
      lastStatus = status;
      lastLogAt = now;
    }

    if (status == WL_CONNECTED) {
      wifiUiState = WIFI_UI_CONNECTED;
      Serial.println(F("WiFi: connesso"));
      Serial.print(F("WiFi: IP: "));
      Serial.println(WiFi.localIP());
      Serial.print(F("WiFi: RSSI: "));
      Serial.println(WiFi.RSSI());
      drawCurrentScreen();
      return;
    }

    if (now - lastDrawAt >= WIFI_CONNECT_ANIMATION_MS) {
      drawWifiStartupScreen(now - startedAt);
      lastDrawAt = now;
    }

    delay(50);
  }

  wifiUiState = WIFI_UI_DISCONNECTED;
  Serial.println(F("WiFi: timeout, connessione non riuscita"));
  Serial.print(F("WiFi: ultimo stato: "));
  Serial.println(wifiStatusName(WiFi.status()));
  Serial.print(F("WiFi: ultimo motivo disconnessione: "));
  Serial.print(lastWifiDisconnectReason);
  Serial.print(F(" ("));
  Serial.print(wifiDisconnectReasonName(lastWifiDisconnectReason));
  Serial.println(F(")"));
  WiFi.disconnect(false);
  drawCurrentScreen();
}

const char *mqttStateName(int state) {
  switch (state) {
    case MQTT_CONNECTION_TIMEOUT:
      return "MQTT_CONNECTION_TIMEOUT";
    case MQTT_CONNECTION_LOST:
      return "MQTT_CONNECTION_LOST";
    case MQTT_CONNECT_FAILED:
      return "MQTT_CONNECT_FAILED";
    case MQTT_DISCONNECTED:
      return "MQTT_DISCONNECTED";
    case MQTT_CONNECTED:
      return "MQTT_CONNECTED";
    case MQTT_CONNECT_BAD_PROTOCOL:
      return "MQTT_CONNECT_BAD_PROTOCOL";
    case MQTT_CONNECT_BAD_CLIENT_ID:
      return "MQTT_CONNECT_BAD_CLIENT_ID";
    case MQTT_CONNECT_UNAVAILABLE:
      return "MQTT_CONNECT_UNAVAILABLE";
    case MQTT_CONNECT_BAD_CREDENTIALS:
      return "MQTT_CONNECT_BAD_CREDENTIALS";
    case MQTT_CONNECT_UNAUTHORIZED:
      return "MQTT_CONNECT_UNAUTHORIZED";
    default:
      return "MQTT_STATE_SCONOSCIUTO";
  }
}

bool connectToMqtt() {
  if (mqttClient.connected()) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("MQTT: WiFi non connesso, impossibile connettere il broker"));
    return false;
  }

  const unsigned long now = millis();
  if (lastMqttConnectAttemptAt != 0 && now - lastMqttConnectAttemptAt < MQTT_RECONNECT_INTERVAL_MS) {
    Serial.print(F("MQTT: riconnessione in cooldown, ms rimanenti: "));
    Serial.println(MQTT_RECONNECT_INTERVAL_MS - (now - lastMqttConnectAttemptAt));
    return false;
  }

  lastMqttConnectAttemptAt = now;

  Serial.println(F("MQTT: avvio connessione"));
  Serial.print(F("MQTT: host: "));
  Serial.print(MQTT_HOST);
  Serial.print(F(":"));
  Serial.println(MQTT_PORT);
  Serial.print(F("MQTT: user: "));
  Serial.println(MQTT_USER);

  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  Serial.println(F("MQTT: tentativo singolo"));

  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println(F("MQTT: connesso"));
    return true;
  }

  const int state = mqttClient.state();
  Serial.print(F("MQTT: connessione non riuscita, ultimo state: "));
  Serial.print(state);
  Serial.print(F(" ("));
  Serial.print(mqttStateName(state));
  Serial.println(F(")"));

  if (state == MQTT_CONNECT_BAD_CREDENTIALS || state == MQTT_CONNECT_UNAUTHORIZED) {
    Serial.println(F("MQTT: errore autenticazione, controllo user/password o permessi broker necessario"));
  }

  return false;
}

const char *mqttActivityName() {
  if (strcmp(selectedActivity, "Meditazione") == 0) {
    return "meditazione";
  }
  if (strcmp(selectedActivity, "Lettura") == 0) {
    return "lettura";
  }
  if (strcmp(selectedActivity, "Videogame") == 0) {
    return "videogame";
  }

  return "default";
}

void chooseActivityTargets(const char *activity, int &targetAtt, int &targetMed) {
  if (strcmp(activity, "meditazione") == 0) {
    targetAtt = random(20, 50);
    targetMed = random(70, 95);
  } else if (strcmp(activity, "lettura") == 0) {
    targetAtt = random(65, 85);
    targetMed = random(40, 60);
  } else if (strcmp(activity, "videogame") == 0) {
    targetAtt = random(80, 100);
    targetMed = random(20, 45);
  } else {
    targetAtt = random(40, 60);
    targetMed = random(40, 60);
  }
}

void sendSimulatedEEG() {
  if (!activityRunning) {
    Serial.println(F("MQTT: publish annullato, attivita non avviata"));
    return;
  }

  const char *activity = mqttActivityName();
  int targetAtt;
  int targetMed;
  chooseActivityTargets(activity, targetAtt, targetMed);

  if (curAtt < targetAtt) {
    curAtt += 2;
  } else if (curAtt > targetAtt) {
    curAtt -= 2;
  }

  if (curMed < targetMed) {
    curMed += 2;
  } else if (curMed > targetMed) {
    curMed -= 2;
  }

  curAtt = constrain(curAtt, 0, 100);
  curMed = constrain(curMed, 0, 100);

  long waves[8];
  for (uint8_t i = 0; i < 8; i++) {
    const float multiplier = random(95, 106) / 100.0;
    currentWaves[i] *= multiplier;
    currentWaves[i] = constrain(currentWaves[i], 1000.0, 500000.0);

    float activityBoost = 1.0;
    if (strcmp(activity, "meditazione") == 0 && (i == 2 || i == 3)) {
      activityBoost = 1.4;
    } else if (strcmp(activity, "lettura") == 0 && (i == 4 || i == 5)) {
      activityBoost = 1.2;
    } else if (strcmp(activity, "videogame") == 0 && (i == 4 || i == 5 || i == 6)) {
      activityBoost = 1.35;
    }

    waves[i] = (long)(currentWaves[i] * activityBoost);
  }

  JsonDocument doc;
  doc["activity"] = activity;
  doc["poorSignal"] = 0;
  doc["attention"] = curAtt;
  doc["meditation"] = curMed;
  doc["uptimeMs"] = millis();

  JsonObject w = doc["waves"].to<JsonObject>();
  w["delta"] = waves[0];
  w["theta"] = waves[1];
  w["loAlpha"] = waves[2];
  w["hiAlpha"] = waves[3];
  w["loBeta"] = waves[4];
  w["hiBeta"] = waves[5];
  w["loGamma"] = waves[6];
  w["midGamma"] = waves[7];

  char buffer[512];
  const size_t payloadSize = serializeJson(doc, buffer, sizeof(buffer));

  if (!connectToMqtt()) {
    Serial.println(F("MQTT: errore, broker non connesso"));
    return;
  }

  if (!activityRunning) {
    Serial.println(F("MQTT: publish annullato dopo connessione, attivita fermata"));
    return;
  }

  const bool published = mqttClient.publish(MQTT_TOPIC, buffer, payloadSize);
  Serial.print(F("MQTT: publish topic: "));
  Serial.print(MQTT_TOPIC);
  Serial.print(F(" | bytes: "));
  Serial.print(payloadSize);
  Serial.print(F(" | esito: "));
  Serial.println(published ? F("ok") : F("fallito"));
  Serial.println(buffer);
}

void handleMqttStreaming() {
  mqttClient.loop();

  if (!activityRunning) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastMqttPublishAt >= MQTT_PUBLISH_INTERVAL_MS) {
    lastMqttPublishAt = now;
    sendSimulatedEEG();
  }
}

int readAverageAnalog(uint8_t pin, uint8_t samples) {
  long total = 0;

  for (uint8_t i = 0; i < samples; i++) {
    total += analogRead(pin);
    delay(2);
  }

  return total / samples;
}

int calculateMoveDelta(int availableRange) {
  return constrain((availableRange * JOY_MOVE_PERCENT) / 100, JOY_MIN_MOVE_DELTA, JOY_MAX_MOVE_DELTA);
}

void calibrateJoystick() {
  delay(200);
  joystickCenterX = readAverageAnalog(JOY_X_PIN, 24);
  joystickCenterY = readAverageAnalog(JOY_Y_PIN, 24);

  joystickLeftDelta = calculateMoveDelta(joystickCenterX);
  joystickRightDelta = calculateMoveDelta(ADC_MAX_VALUE - joystickCenterX);
  joystickUpDelta = calculateMoveDelta(joystickCenterY);
  joystickDownDelta = calculateMoveDelta(ADC_MAX_VALUE - joystickCenterY);

  Serial.print(F("Joystick calibrato | X centro: "));
  Serial.print(joystickCenterX);
  Serial.print(F(" | Y centro: "));
  Serial.println(joystickCenterY);

  Serial.print(F("Soglie | left: -"));
  Serial.print(joystickLeftDelta);
  Serial.print(F(" | right: +"));
  Serial.print(joystickRightDelta);
  Serial.print(F(" | up: -"));
  Serial.print(joystickUpDelta);
  Serial.print(F(" | down: +"));
  Serial.println(joystickDownDelta);
}

bool joystickIsReleased() {
  const int xValue = analogRead(JOY_X_PIN);
  const int yValue = analogRead(JOY_Y_PIN);

  return abs(xValue - joystickCenterX) < JOY_RELEASE_DELTA &&
         abs(yValue - joystickCenterY) < JOY_RELEASE_DELTA;
}

Direction readJoystick() {
  const int xValue = analogRead(JOY_X_PIN);
  const int yValue = analogRead(JOY_Y_PIN);
  const int xDelta = xValue - joystickCenterX;
  const int yDelta = yValue - joystickCenterY;

  if (xDelta < -joystickLeftDelta) {
    return DIR_LEFT;
  }
  if (xDelta > joystickRightDelta) {
    return DIR_RIGHT;
  }
  if (yDelta > joystickDownDelta) {
    return DIR_DOWN;
  }
  if (yDelta < -joystickUpDelta) {
    return DIR_UP;
  }

  return DIR_NONE;
}

void printJoystickPositionDebug() {
  const unsigned long now = millis();

  if (now - lastJoystickDebugAt < JOY_DEBUG_INTERVAL_MS) {
    return;
  }

  lastJoystickDebugAt = now;

  const int xValue = analogRead(JOY_X_PIN);
  const int yValue = analogRead(JOY_Y_PIN);
  const int xDelta = xValue - joystickCenterX;
  const int yDelta = yValue - joystickCenterY;
  const Direction direction = readJoystick();

  Serial.print(F("JOY RAW | X: "));
  Serial.print(xValue);
  Serial.print(F(" dX: "));
  Serial.print(xDelta);
  Serial.print(F(" | Y: "));
  Serial.print(yValue);
  Serial.print(F(" dY: "));
  Serial.print(yDelta);
  Serial.print(F(" | dir: "));
  Serial.print(directionName(direction));
  Serial.print(F(" | wait release: "));
  Serial.println(waitingForJoystickRelease ? F("si") : F("no"));
}

Direction readJoystickEvent() {
  if (waitingForJoystickRelease) {
    if (joystickIsReleased()) {
      waitingForJoystickRelease = false;
      lastDirection = DIR_NONE;
    }

    return DIR_NONE;
  }

  const Direction direction = readJoystick();
  if (direction == DIR_NONE) {
    lastDirection = DIR_NONE;
    return DIR_NONE;
  }

  lastDirection = direction;
  waitingForJoystickRelease = true;
  return direction;
}

void nextMainMenuItem() {
  mainMenuIndex = (mainMenuIndex + 1) % mainMenuCount;
}

void nextModeMenuItem() {
  modeMenuIndex = (modeMenuIndex + 1) % modeMenuCount;
}

void confirmMainMenuItem() {
  if (mainMenuIndex == 0) {
    if (activityRunning) {
      activityRunning = false;
      lastMqttPublishAt = 0;
      Serial.println(F("EEG: streaming simulato fermato"));
    } else {
      activityRunning = true;
      lastMqttPublishAt = 0;
      Serial.print(F("EEG: streaming simulato avviato | activity: "));
      Serial.println(mqttActivityName());
      sendSimulatedEEG();
      lastMqttPublishAt = millis();
    }

    mainMenuIndex = 0;
    currentScreen = SCREEN_MAIN_MENU;
    return;
  }

  if (strcmp(mainMenuItems[mainMenuIndex], "Modalita") == 0) {
    currentScreen = SCREEN_MODE_MENU;
  }
}

void confirmModeMenuItem() {
  selectedActivity = modeMenuItems[modeMenuIndex];
  mainMenuIndex = 0;
  currentScreen = SCREEN_MAIN_MENU;
}

void goBack() {
  if (currentScreen == SCREEN_MODE_MENU || currentScreen == SCREEN_RUNNING) {
    currentScreen = SCREEN_MAIN_MENU;
  }
}

const char *directionName(Direction direction) {
  switch (direction) {
    case DIR_LEFT:
      return "sinistra";
    case DIR_RIGHT:
      return "destra";
    case DIR_DOWN:
      return "giu";
    case DIR_UP:
      return "su";
    case DIR_NONE:
      return "nessuno";
  }

  return "sconosciuto";
}

const char *screenName(Screen screen) {
  switch (screen) {
    case SCREEN_MAIN_MENU:
      return "menu principale";
    case SCREEN_MODE_MENU:
      return "menu modalita";
    case SCREEN_RUNNING:
      return "routine avviata";
  }

  return "sconosciuta";
}

const char *currentItemName() {
  switch (currentScreen) {
    case SCREEN_MAIN_MENU:
      if (mainMenuIndex == 0) {
        return activityRunning ? "Stop" : "Start";
      }
      return mainMenuItems[mainMenuIndex];
    case SCREEN_MODE_MENU:
      return modeMenuItems[modeMenuIndex];
    case SCREEN_RUNNING:
      return selectedActivity;
  }

  return "";
}

void printActionResult(Direction direction, Screen previousScreen, const char *previousItem) {
  Serial.print(F("Joystick: "));
  Serial.println(directionName(direction));

  Serial.print(F("Azione: "));
  switch (direction) {
    case DIR_LEFT:
      if (previousScreen == currentScreen) {
        Serial.println(F("indietro ignorato"));
      } else {
        Serial.println(F("ritorno al menu principale"));
      }
      break;
    case DIR_RIGHT:
      if (previousScreen == SCREEN_MAIN_MENU || previousScreen == SCREEN_MODE_MENU) {
        Serial.print(F("voce successiva: "));
        Serial.println(currentItemName());
      } else {
        Serial.println(F("voce successiva ignorata"));
      }
      break;
    case DIR_DOWN:
      if (previousScreen == SCREEN_MAIN_MENU && strcmp(previousItem, "Start") == 0) {
        Serial.print(F("avvio routine: "));
        Serial.println(selectedActivity);
      } else if (previousScreen == SCREEN_MAIN_MENU && strcmp(previousItem, "Stop") == 0) {
        Serial.print(F("stop routine: "));
        Serial.println(selectedActivity);
      } else if (previousScreen == SCREEN_MAIN_MENU && strcmp(previousItem, "Modalita") == 0) {
        Serial.println(F("apertura sottomenu modalita"));
      } else if (previousScreen == SCREEN_MODE_MENU) {
        Serial.print(F("modalita selezionata: "));
        Serial.println(selectedActivity);
      } else {
        Serial.println(F("conferma ignorata"));
      }
      break;
    case DIR_UP:
      Serial.println(F("su ignorato"));
      break;
    case DIR_NONE:
      Serial.println(F("nessuna azione"));
      break;
  }

  Serial.print(F("Schermata: "));
  Serial.print(screenName(currentScreen));
  Serial.print(F(" | Voce: "));
  Serial.print(currentItemName());
  Serial.print(F(" | Mode: "));
  Serial.print(selectedActivity);
  Serial.print(F(" | Running: "));
  Serial.println(activityRunning ? F("si") : F("no"));
}

void handleDirection(Direction direction) {
  const Screen previousScreen = currentScreen;
  const char *previousItem = currentItemName();

  switch (direction) {
    case DIR_LEFT:
      goBack();
      break;
    case DIR_RIGHT:
      if (currentScreen == SCREEN_MAIN_MENU) {
        nextMainMenuItem();
      } else if (currentScreen == SCREEN_MODE_MENU) {
        nextModeMenuItem();
      }
      break;
    case DIR_DOWN:
      if (currentScreen == SCREEN_MAIN_MENU) {
        confirmMainMenuItem();
      } else if (currentScreen == SCREEN_MODE_MENU) {
        confirmModeMenuItem();
      }
      break;
    case DIR_UP:
    case DIR_NONE:
      break;
  }

  printActionResult(direction, previousScreen, previousItem);
}

void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());

  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("ERRORE: Display non trovato. Controlla le saldature!"));
    for (;;) {
    }
  }

  drawCurrentScreen();
  calibrateJoystick();
  connectToWifi();
  drawCurrentScreen();
}

void loop() {
  printJoystickPositionDebug();

  const Direction direction = readJoystickEvent();

  if (direction != DIR_NONE) {
    handleDirection(direction);
    drawCurrentScreen();
  }

  handleMqttStreaming();
}
