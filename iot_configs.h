#ifndef IOTCONFIGS
#define IOTCONFIGS
// Wifi
#define IOT_CONFIG_WIFI_SSID ""
#define IOT_CONFIG_WIFI_PASSWORD ""

// Azure IoT
#define IOT_CONFIG_IOTHUB_FQDN ""
#define IOT_CONFIG_DEVICE_ID ""
#define IOT_CONFIG_DEVICE_KEY ""

// Publish 1 message every 2 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 2000

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;
static uint8_t telemetry_payload[100];

void establishConnection();
static WiFiClientSecure wifi_client;
static PubSubClient mqtt_client(wifi_client); 
#endif
