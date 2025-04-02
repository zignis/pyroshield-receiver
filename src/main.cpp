#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define LORA_FREQ 433E6 // LoRa module frequency (433 MHz)
#define LORA_SYNC_WORD 0xB4 // Sync word for this network
#define LORA_SS_PIN 10
#define LORA_RESET_PIN 9
#define LORA_DIO0_PIN 2

typedef struct LoRa_Payload {
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
    uint8_t bmp280_temp{};
    uint8_t dht22_temp{};
    uint8_t humidity{};
    uint16_t gps_altitude{};
    float gps_lat{};
    float gps_lng{};
    uint16_t gps_satellites{};

    /* Power */
    uint8_t battery_temp{};
    uint16_t battery_voltage{};
    uint16_t charger_voltage{};

    /* System */
    uint16_t memory_usage{};
} LoRa_Payload;


void print_payload(const LoRa_Payload &payload);

void setup() {
    Serial.begin(9600);

    while (!Serial) {
    }

    LoRa.setPins(LORA_SS_PIN, LORA_RESET_PIN, LORA_DIO0_PIN);
    LoRa.setSyncWord(LORA_SYNC_WORD);

    if (!LoRa.begin(LORA_FREQ)) {
        Serial.println("LoRa init failed.");
        exit(EXIT_FAILURE);
    }

    Serial.println("LoRa init succeeded.");
}

void loop() {
    const int packet_size = LoRa.parsePacket();

    if (packet_size) {
        LoRa_Payload data;
        LoRa.readBytes(reinterpret_cast<uint8_t *>(&data), packet_size);
        print_payload(data);
    }
}

void print_payload(const LoRa_Payload &payload) {
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
    Serial.println(payload.bmp280_temp);

    Serial.print("DHT22 temperature: ");
    Serial.println(payload.dht22_temp);

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
    Serial.println(payload.battery_temp);

    Serial.print("Battery voltage: ");
    Serial.println(payload.battery_voltage);

    Serial.print("Charger voltage: ");
    Serial.println(payload.charger_voltage);

    // System
    Serial.print("Memory usage: ");
    Serial.println(payload.memory_usage);

    Serial.println("____________________");
    Serial.println("");
}
