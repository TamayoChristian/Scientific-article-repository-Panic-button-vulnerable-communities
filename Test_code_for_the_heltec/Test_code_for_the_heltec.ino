/*#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_TinyGPS++.h"

// ===================== GPS =====================
#define GPS_RX 33
#define GPS_TX 34
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial GPS(1);

// ===================== LORAWAN =====================
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_US915;
DeviceClass_t loraWanClass = CLASS_A;

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = false;

uint32_t appTxDutyCycle = 10000;
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;
uint32_t devAddr = 0;

uint8_t nwkSKey[16] = { 0 };
uint8_t appSKey[16] = { 0 };

uint16_t userChannelsMask[6] = {
  0x00FF, 0, 0, 0, 0, 0
};

// ======== TTN KEYS ========
uint8_t devEui[] = { 0x00, 0x00, 0x40, 0x3B, 0x3C, 0x43, 0xCA, 0x48 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = {
  0x3C, 0x1F, 0x23, 0x1A, 0xBA, 0xDB, 0xB4, 0x87,
  0xED, 0x92, 0x95, 0xC0, 0xF8, 0xCD, 0x69, 0xBE
};

// ===================== VARIABLES =====================
float latitude = 0.0;
float longitude = 0.0;
uint8_t sats = 0;
bool gpsFix = false;
// ====================================================
void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println("🚀 HELTEC V3 + NEO-7M GPS → TTN");

  GPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  deviceState = DEVICE_STATE_INIT;  // ⭐ ESTA LÍNEA FALTABA
}

// ====================================================
void loop() {

  // Leer GPS continuamente
  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  // Verificar FIX real
  if (gps.location.isValid() && gps.satellites.value() >= 4) {
    gpsFix = true;

    latitude = gps.location.lat();
    longitude = gps.location.lng();
    sats = gps.satellites.value();

    Serial.printf("✅ GPS FIX: %.6f , %.6f | sats=%d\n",
                  latitude, longitude, sats);
  }

  switch (deviceState) {

    case DEVICE_STATE_INIT:
      Serial.println("STATE: INIT");
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:
      Serial.println("STATE: JOIN");
      LoRaWAN.join();
      deviceState = DEVICE_STATE_SEND;
      break;


    case DEVICE_STATE_SEND:

      Serial.println("STATE: SEND");

      if (gpsFix) {

        appDataSize = sprintf(
          (char*)appData,
          "{\"lat\":%.6f,\"lon\":%.6f,\"sats\":%d}",
          latitude,
          longitude,
          sats);

      } else {

        appDataSize = sprintf((char*)appData, "NO_FIX");
      }

      Serial.print("Payload: ");
      Serial.println((char*)appData);

      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;

    case DEVICE_STATE_CYCLE:
      LoRaWAN.cycle(appTxDutyCycle);
      deviceState = DEVICE_STATE_SLEEP;
      break;

    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep(loraWanClass);
      break;
  }
}
*/

















/*
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_TinyGPS++.h"
#include <EEPROM.h>
// ===================== GPS =====================
#define GPS_RX 33
#define GPS_TX 34
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial GPS(1);

// ===================== LORAWAN =====================
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_US915;
DeviceClass_t loraWanClass = CLASS_A;

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = false;

uint32_t appTxDutyCycle = 10000;  // 10 s
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;
uint32_t devAddr = 0;

uint8_t nwkSKey[16] = { 0 };
uint8_t appSKey[16] = { 0 };

uint16_t userChannelsMask[6] = { 0x00FF, 0, 0, 0, 0, 0 };

// ======== TTN KEYS ========
uint8_t devEui[] = { 0x00, 0x00, 0x40, 0x3B, 0x3C, 0x43, 0xCA, 0x48 };
uint8_t appEui[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t appKey[] = {
  0x3C, 0x1F, 0x23, 0x1A, 0xBA, 0xDB, 0xB4, 0x87,
  0xED, 0x92, 0x95, 0xC0, 0xF8, 0xCD, 0x69, 0xBE
};

// ===================== VARIABLES =====================
float latitude = 0.0;
float longitude = 0.0;
uint8_t sats = 0;
bool gpsFix = false;

unsigned long t0 = 0;  // inicio de la prueba
uint32_t msgId = 0;    // contador de mensajes enviados con fix

bool joinComplete = false;  // NUEVO: flag para controlar join

// ====================================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("🚀 HELTEC V3 + NEO-7M GPS → TTN");

  GPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  deviceState = DEVICE_STATE_INIT;

  t0 = millis();  // tiempo relativo inicial
  EEPROM.begin(512);
  EEPROM.get(0, msgId);  // Recupera último msgId
  Serial.printf("📟 Último msgId cargado: %lu\n", msgId);
}

// ====================================================
void loop() {

  // ======== LEER GPS ========
  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  if (gps.location.isValid() && gps.satellites.value() >= 4) {
    gpsFix = true;
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    sats = gps.satellites.value();

    Serial.printf("✅ GPS FIX: %.6f , %.6f | sats=%d\n", latitude, longitude, sats);
  } else {
    gpsFix = false;
  }

  unsigned long currentMillis = millis();
  unsigned long ts_rel = currentMillis - t0;

  // ======== ESTADOS LORAWAN ========
  switch (deviceState) {

    case DEVICE_STATE_INIT:
      Serial.println("STATE: INIT");
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:
      if (!joinComplete) {
        Serial.println("STATE: JOIN - intentando unirse...");
        LoRaWAN.join();
        joinComplete = true;  // una vez llamado, ya no intentará join otra vez
      } else {
        Serial.println("✅ JOIN considerado completo");
        deviceState = DEVICE_STATE_SEND;
      }
      break;

    case DEVICE_STATE_SEND:
      if (!joinComplete) {
        // Si por alguna razón no se unió, vuelve a JOIN
        deviceState = DEVICE_STATE_JOIN;
        break;
      }

      Serial.println("STATE: SEND");
      msgId++;
      EEPROM.put(0, msgId);
      EEPROM.commit();


      char payload[128];
      if (gpsFix) {
        snprintf(payload, sizeof(payload),
                 "{\"id\":%lu,\"lat\":%.6f,\"lon\":%.6f,\"sats\":%d,\"ts_rel\":%lu}",
                 msgId, latitude, longitude, sats, millis() - t0);
      } else {
        snprintf(payload, sizeof(payload),
                 "{\"message\":\"NO_FIX\",\"ts_rel\":%lu}",
                 millis() - t0);
      }

      appDataSize = strlen(payload);
      memcpy(appData, payload, appDataSize);

      Serial.print("Payload: ");
      Serial.println(payload);

      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;


    case DEVICE_STATE_CYCLE:
      LoRaWAN.cycle(appTxDutyCycle);
      deviceState = DEVICE_STATE_SLEEP;
      break;

    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep(loraWanClass);
      break;
  }
}
*/



#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_TinyGPS++.h"
#include <EEPROM.h>
#include <TimeLib.h>  // Para makeTime() y timestamp UNIX

// ===================== GPS =====================
#define GPS_RX 33
#define GPS_TX 34
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial GPS(1);

// ===================== LORAWAN =====================
LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_US915;
DeviceClass_t loraWanClass = CLASS_A;

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = false;

uint32_t appTxDutyCycle = 10000;  // 10 s
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;
uint32_t devAddr = 0;

uint8_t nwkSKey[16] = { 0 };
uint8_t appSKey[16] = { 0 };

uint16_t userChannelsMask[6] = { 0x00FF, 0, 0, 0, 0, 0 };

// ======== TTN KEYS ========
uint8_t devEui[] = { 0x00, 0x00, 0x40, 0x3B, 0x3C, 0x43, 0xCA, 0x48 };
uint8_t appEui[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t appKey[] = {
  0x3C, 0x1F, 0x23, 0x1A, 0xBA, 0xDB, 0xB4, 0x87,
  0xED, 0x92, 0x95, 0xC0, 0xF8, 0xCD, 0x69, 0xBE
};

// ===================== VARIABLES =====================
float latitude = 0.0;
float longitude = 0.0;
uint8_t sats = 0;
bool gpsFix = false;

unsigned long t0 = 0;  
uint32_t msgId = 0;    
bool joinComplete = false;

// Función para obtener timestamp UTC desde GPS
unsigned long getGpsTimestamp() {
  if (gps.date.isValid() && gps.time.isValid()) {
    tmElements_t tm;
    tm.Year = gps.date.year() - 1970;
    tm.Month = gps.date.month();
    tm.Day = gps.date.day();
    tm.Hour = gps.time.hour();  // << sin ajustar
    tm.Minute = gps.time.minute();
    tm.Second = gps.time.second();
    return makeTime(tm);  // timestamp en UTC
  }
  return 0;
}



// ====================================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("🚀 HELTEC V3 + NEO-7M GPS → TTN");

  GPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
  deviceState = DEVICE_STATE_INIT;

  t0 = millis();  

  // Inicializar EEPROM y recuperar msgId
  EEPROM.begin(512);
  EEPROM.get(0, msgId);
  Serial.printf("📟 Último msgId cargado: %lu\n", msgId);
}

// ====================================================
void loop() {
  // ======== LEER GPS ========
  while (GPS.available()) gps.encode(GPS.read());

  if (gps.location.isValid() && gps.satellites.value() >= 4) {
    gpsFix = true;
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    sats = gps.satellites.value();
    Serial.printf("✅ GPS FIX: %.6f , %.6f | sats=%d\n", latitude, longitude, sats);
  } else {
    gpsFix = false;
  }

  // ======== ESTADOS LORAWAN ========
  unsigned long gps_ts = getGpsTimestamp();  // timestamp UTC

  switch (deviceState) {

    case DEVICE_STATE_INIT:
      Serial.println("STATE: INIT");
      LoRaWAN.init(loraWanClass, loraWanRegion);
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:
      if (!joinComplete) {
        Serial.println("STATE: JOIN - intentando unirse...");
        LoRaWAN.join();
        joinComplete = true;
      } else {
        Serial.println("✅ JOIN considerado completo");
        deviceState = DEVICE_STATE_SEND;
      }
      break;

    case DEVICE_STATE_SEND:
      if (!joinComplete) {
        deviceState = DEVICE_STATE_JOIN;
        break;
      }

      Serial.println("STATE: SEND");

      // Incrementar msgId y guardar en EEPROM
      msgId++;
      EEPROM.put(0, msgId);
      EEPROM.commit();

      // Preparar payload
      char payload[128];

      if (gpsFix && gps_ts > 0) {
        snprintf(payload, sizeof(payload),
                 "{\"id\":%lu,\"lat\":%.6f,\"lon\":%.6f,\"sats\":%d,\"ts\":%lu}",
                 msgId, latitude, longitude, sats, gps_ts);
      } else {
        snprintf(payload, sizeof(payload),
                 "{\"id\":%lu,\"message\":\"NO_FIX\"}",
                 msgId);
      }

      appDataSize = strlen(payload);
      memcpy(appData, payload, appDataSize);

      Serial.print("Payload: ");
      Serial.println(payload);

      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;

    case DEVICE_STATE_CYCLE:
      LoRaWAN.cycle(appTxDutyCycle);
      deviceState = DEVICE_STATE_SLEEP;
      break;

    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep(loraWanClass);
      break;
  }
}

