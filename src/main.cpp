#include <Arduino.h>
#include <LoRa.h>
#include <ArduinoJson.h>

#define DEBUG_MODE false

#define LORA_FREQ 433E6 // LoRa module frequency (433 MHz)
#define LORA_SYNC_WORD 0xB4 // Sync word for this network
#define LORA_SS_PIN 10
#define LORA_RESET_PIN 9
#define LORA_DIO0_PIN 2
#define LED_PIN 8

typedef struct __attribute__((__packed__)) LoRa_Payload {
    /* Header */
    byte forwarder_id = 0;
    byte transmitter_id = 0;
    uint16_t message_id = 0;
    bool allow_forwarding = false;
    uint8_t ttl = 0;

    /* Body */
    uint16_t co2_ppm{};
    uint16_t pressure{};
    uint16_t bmp280_altitude{};
    uint16_t bmp280_temp{};
    uint16_t dht22_temp{};
    uint8_t humidity{};
    uint16_t gps_altitude{};
    float gps_lat{};
    float gps_lng{};
    uint16_t gps_satellites{};

    /* Power */
    uint16_t battery_temp{};
    uint16_t battery_voltage{};
    uint16_t charger_voltage{};

    /* System */
    uint16_t memory_usage{};
} LoRa_Payload;

void send_payload_json(const LoRa_Payload &payload);

void debug_print_payload(const LoRa_Payload &payload);

void setup() {
    pinMode(LED_PIN, OUTPUT);

    Serial.begin(9600);

    while (!Serial) {
    }

    LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(LORA_FREQ)) {
        if (DEBUG_MODE) {
            Serial.println("LoRa init failed.");
        }

        exit(EXIT_FAILURE);
    }

    LoRa.enableCrc();
    LoRa.setSyncWord(LORA_SYNC_WORD);

    if (DEBUG_MODE) {
        Serial.println("LoRa init succeeded.");
    }
}

void loop() {
    const int packet_size = LoRa.parsePacket();

    if (packet_size) {
        digitalWrite(LED_PIN, HIGH);

        LoRa_Payload data;
        LoRa.readBytes(reinterpret_cast<uint8_t *>(&data), packet_size);

        if (DEBUG_MODE) {
            debug_print_payload(data);
        } else {
            send_payload_json(data);
        }

        digitalWrite(LED_PIN, LOW);
    }
}

void send_payload_json(const LoRa_Payload &payload) {
    StaticJsonDocument<256> doc;

    doc["device_id"] = payload.transmitter_id;
    doc["forwarder_id"] = payload.forwarder_id;
    doc["message_id"] = payload.message_id;
    doc["allow_forwarding"] = payload.allow_forwarding;
    doc["ttl"] = payload.ttl;
    doc["co2_ppm"] = payload.co2_ppm;
    doc["pressure"] = payload.pressure;

    const JsonObject bmp280 = doc.createNestedObject("bmp280");
    bmp280["altitude"] = payload.bmp280_altitude;
    bmp280["temp"] = static_cast<float>(payload.bmp280_temp) / 10;

    const JsonObject dht22 = doc.createNestedObject("dht22");
    dht22["temp"] = static_cast<float>(payload.dht22_temp) / 10;
    dht22["humidity"] = payload.humidity;

    const JsonObject gps = doc.createNestedObject("gps");
    gps["altitude"] = payload.gps_altitude;
    gps["lat"] = String(payload.gps_lat, 6);
    gps["lng"] = String(payload.gps_lng, 6);
    gps["satellites"] = payload.gps_satellites;

    const JsonObject battery = doc.createNestedObject("battery");
    battery["temp"] = static_cast<float>(payload.battery_temp) / 10;
    battery["voltage"] = static_cast<float>(payload.battery_voltage) / 100;
    doc["charger"]["voltage"] = static_cast<float>(payload.charger_voltage) / 100;
    doc["sys"]["mem_usage"] = payload.memory_usage;

    serializeJson(doc, Serial);
    Serial.println(); // Newline to indicate the end of message.
}

void debug_print_payload(const LoRa_Payload &payload) {
    Serial.println("");
    Serial.println("____________________");

    // Header
    Serial.print("Forwarder ID: ");
    Serial.println(payload.forwarder_id);

    Serial.print("Transmitter ID: ");
    Serial.println(payload.transmitter_id);

    Serial.print("Message ID: ");
    Serial.println(payload.message_id);

    Serial.print("Allow forwarding: ");
    Serial.println(payload.allow_forwarding);

    Serial.print("TTL: ");
    Serial.println(payload.ttl);

    // Body
    Serial.print("CO2 PPM: ");
    Serial.println(payload.co2_ppm);

    Serial.print("Pressure: ");
    Serial.println(payload.pressure);

    Serial.print("BMP280 altitude: ");
    Serial.println(payload.bmp280_altitude);

    Serial.print("BMP280 temperature: ");
    Serial.println(static_cast<float>(payload.bmp280_temp) / 10, 1);

    Serial.print("DHT22 temperature: ");
    Serial.println(static_cast<float>(payload.dht22_temp) / 10, 1);

    Serial.print("Humidity: ");
    Serial.println(payload.humidity);

    Serial.print("GPS altitude: ");
    Serial.println(payload.gps_altitude);

    Serial.print("GPS lat: ");
    Serial.println(payload.gps_lat, 6);

    Serial.print("GPS lng: ");
    Serial.println(payload.gps_lng, 6);

    Serial.print("Map: ");
    Serial.print("https://www.google.com/maps/search/?api=1&query=");
    Serial.print(payload.gps_lat, 6);
    Serial.print(",");
    Serial.println(payload.gps_lng, 6);

    Serial.print("GPS satellites: ");
    Serial.println(payload.gps_satellites);

    // Power
    Serial.print("Battery temperature: ");
    Serial.println(static_cast<float>(payload.battery_temp) / 10, 1);

    Serial.print("Battery voltage: ");
    Serial.println(static_cast<float>(payload.battery_voltage) / 100, 2);

    Serial.print("Charger voltage: ");
    Serial.println(static_cast<float>(payload.charger_voltage) / 100, 2);

    // System
    Serial.print("Memory usage: ");
    Serial.println(payload.memory_usage);

    Serial.println("____________________");
    Serial.println("");
}

