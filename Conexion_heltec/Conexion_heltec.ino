#include "LoRaWan_APP.h"
#include "Arduino.h"

#define DEBUG Serial0


LoRaMacRegion_t loraWanRegion = LORAMAC_REGION_AU915;
DeviceClass_t loraWanClass = CLASS_A;

bool overTheAirActivation = true;
bool loraWanAdr = true;
bool isTxConfirmed = false;

uint32_t appTxDutyCycle = 10000;
uint8_t appPort = 1; 
uint8_t confirmedNbTrials = 4;

uint32_t devAddr = 0x00000000;
uint8_t nwkSKey[16] = {0};
uint8_t appSKey[16] = {0};

uint16_t userChannelsMask[6] = {
  0x00FF, 0, 0, 0, 0, 0  // FSB1 canales 0-7 (902.3-903.7 MHz)
};

uint8_t devEui[] = { 0x00,0x00,0x40,0x3B,0x3C,0x43,0xCA,0x48 };
uint8_t appEui[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
uint8_t appKey[] = {
  0xE2,0xB2,0x9C,0x40,0x36,0x49,0x74,0x3B,
  0x05,0xE8,0x73,0xC6,0xD5,0xBD,0xA9,0xFC
};

void setup() {
  DEBUG.begin(115200);
  delay(2000);
  DEBUG.println("HELTEC S3 START");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
}

void loop() {
  switch (deviceState) {

    case DEVICE_STATE_INIT:
      DEBUG.println("STATE: INIT");
      LoRaWAN.init(loraWanClass, loraWanRegion);
      LoRaWAN.setDefaultDR(0);
      DEBUG.println("RSSI test...");
      deviceState = DEVICE_STATE_JOIN;
      break;

    case DEVICE_STATE_JOIN:
      DEBUG.println("STATE: JOIN");
      LoRaWAN.join();
      break;

    case DEVICE_STATE_SEND:
      DEBUG.println("STATE: SEND");
      appDataSize = sprintf((char*)appData, "queso.");
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;

    case DEVICE_STATE_CYCLE:
      DEBUG.println("STATE: CYCLE");
      LoRaWAN.cycle(appTxDutyCycle);
      deviceState = DEVICE_STATE_SLEEP;
      break;

    case DEVICE_STATE_SLEEP:
      DEBUG.println("STATE: SLEEP");
      LoRaWAN.sleep(loraWanClass);
      break;
  }
}

