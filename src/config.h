#pragma once
#include "Arduino.h"
#include "SoftwareSerial.h"

#define JSON_CONFIG_FILE "/config.json"
#define JSON_STATICIP_FILE "/staticip.json"

#define FIRMWARE_VERSION 0.3

IPAddress STATIC_IP = IPAddress(0,0,0,0);
IPAddress STATIC_SUB = IPAddress(0,0,0,0);
IPAddress STATIC_GW = IPAddress(0,0,0,0); 
IPAddress STATIC_DNS = IPAddress(0,0,0,0);

unsigned int RS485_BAUD = 115200;
Config RS485_OPTIONS = SWSERIAL_8N1;
unsigned int LISTEN_PORT = 3333;
