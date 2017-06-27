#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define PIN_SETUP    D5   // this pin LOW causes it to enter WiFi config mode
#define PIN_LED      D3   // built in LED
#define PORT_SPEED   9600 // speed to use for the serial monitor

// Constants
// This is the access point to look for when hitting the config web server from your phone/PC
#define AP_NAME "Blynk_Project_Name"
// change salt to force settings to be re-written into EEPROM
#define EEPROM_SALT 0

// these are the defaults to try connecting with
typedef struct {
  char  bootState[4]      = "off";
  char  blynkToken[33]    = "e5ed22bfca2a499491e0bf26c17d0dc2"; // Tester token
  char  blynkServer[33]   = "blynk-cloud.com";
  char  blynkPort[6]      = "8442";
  int   salt              = EEPROM_SALT;
} WMSettings;

WMSettings settings;

bool shouldSaveConfig = false;

void saveConfigCallback () {
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager);
void blinkStatusLED(bool configMode);

void setup() {
  // App setup before WiFi goes here


  // End app setup before WiFi
  
  const char *apName = AP_NAME;
  WiFiManager wifiManager;

  Serial.begin(PORT_SPEED);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setMinimumSignalQuality(50);

  EEPROM.begin(512);
  EEPROM.get(0, settings);
  EEPROM.end();

  if (settings.salt != EEPROM_SALT) {
    Serial.println("Invalid settings in EEPROM, trying with defaults");
    WMSettings defaults;
    settings = defaults;
    shouldSaveConfig = true;
  }

  WiFiManagerParameter custom_blynk_token("blynk-token", "<br/>Blynk Token:<br/>", settings.blynkToken, 33);
  wifiManager.addParameter(&custom_blynk_token);

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  if (digitalRead(PIN_SETUP) == LOW) { // force setup web server
    Serial.println("PIN_SETUP is LOW, erasing previous WiFi info to force config mode");

    WiFi.disconnect();

    blinkStatusLED(true);

    delay(1000);
  }

  blinkStatusLED(false);

  if (!wifiManager.autoConnect(apName)) { //reset and try again, or maybe put it to deep sleep
    Serial.println("Auto connect failed, resetting and delaying to try again");
    ESP.reset();
    delay(5000);
  }

  Serial.print("Connected to access point: ");
  Serial.println(wifiManager.getSSID());

  // If you got here, you connected one way or another

  if (shouldSaveConfig) {
    // write out config to EEPROM
    Serial.println("Saving configuration to EEPROM");
    strcpy(settings.blynkToken, custom_blynk_token.getValue());

    EEPROM.begin(512);
    EEPROM.put(0, settings);
    EEPROM.end();
  }

// Start up Blynk connection
  Serial.println("Starting Blynk config");
  Blynk.config(settings.blynkToken, settings.blynkServer, atoi(settings.blynkPort));

  // App setup after WiFi goes here


  // End app setup after WiFi

  Serial.println("Setup complete. Entering main loop");
}

void loop() {
  Blynk.run();
}

void blinkStatusLED(bool configMode) {
  int on = 500;
  int off = 100;

  if (!configMode) {
    on = 100;
    off = 500;
  }

  for (int i = 0; i < 4; i++) {
    digitalWrite(PIN_LED, HIGH);
    delay(500);
    digitalWrite(PIN_LED, LOW);
    delay(100);
  }
}

