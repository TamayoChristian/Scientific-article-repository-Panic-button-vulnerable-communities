# Early Warning System for Vulnerable Communities Based on IoT

This repository contains the implementation of an IoT-based early warning system 
that integrates a Heltec device, MQTT communication, and a WhatsApp bot for emergency notifications.

## System Architecture

The system consists of:

- Heltec WiFi LoRa 32 device (panic button sender)
- Sensecap M2 GATEWAY
- THE THINGS NETWORK APPLICATION with MQTT broker for message transmission
- WhatsApp Bot for emergency notifications

## Repository Structure

- BotWhatsApp/ → WhatsApp bot implementation
- libraries/ → Required libraries for Heltec
- Test_code_for_the_heltec/ → Device testing scripts
- How_to_find_the_devEUI_of_the_Heltec_device/ → Utility scripts

## Environment Variables

This project requires a .env file with the following variables:

MQTT_HOST=
MQTT_USER=
MQTT_PASSWORD=


## Running the WhatsApp Bot

cd BotWhatsApp
npm install
node index.js

On first run, you must scan the QR code.

## Security Notice

Authentication files (auth/, session/, .env) are excluded via .gitignore.
Credentials are not included in this repository.