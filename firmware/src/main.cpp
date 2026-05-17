#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_BMP280.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Load local config if present, otherwise fall back to template
#if __has_include("config.local.h")
#include "config.local.h"
#else
#include "config.h"
#endif

Adafruit_SHT4x sht4;
Adafruit_BMP280 bmp;
bool sht4Ok = false;
bool bmpOk  = false;

InfluxDBClient influx(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point envPoint("environment");

// ── WiFi ──────────────────────────────────────────────────────────────────────

void connectWifi() {
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nWiFi connected: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nWiFi failed — will retry next cycle");
    }
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(I2C_SDA, I2C_SCL);

    // SHT40
    sht4Ok = sht4.begin();
    if (sht4Ok) {
        sht4.setPrecision(SHT4X_HIGH_PRECISION);
        sht4.setHeater(SHT4X_NO_HEATER);
        Serial.println("SHT40 ready");
    } else {
        Serial.println("SHT40 not found — check wiring");
    }

    // BMP280 — ENV.IV uses address 0x76
    bmpOk = bmp.begin(BMP280_ADDRESS_ALT);
    if (!bmpOk) bmpOk = bmp.begin();  // fallback to 0x77
    if (bmpOk) {
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                        Adafruit_BMP280::SAMPLING_X2,
                        Adafruit_BMP280::SAMPLING_X16,
                        Adafruit_BMP280::FILTER_X16,
                        Adafruit_BMP280::STANDBY_MS_500);
        Serial.println("BMP280 ready");
    } else {
        Serial.println("BMP280 not found — check wiring");
    }

    connectWifi();

    // NTP sync — InfluxDB needs accurate time for timestamps
    timeSync(TZ_INFO, "pool.ntp.org", "time.nist.gov");

    // Constant tags applied to every point
    envPoint.addTag("device",   DEVICE_ID);
    envPoint.addTag("location", DEVICE_LOCATION);

    influx.setWriteOptions(WriteOptions().writePrecision(WritePrecision::S));
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWifi();
        return;
    }

    envPoint.clearFields();

    if (sht4Ok) {
        sensors_event_t humEvent, tempEvent;
        if (sht4.getEvent(&humEvent, &tempEvent)) {
            envPoint.addField("temperature_c", tempEvent.temperature);
            envPoint.addField("humidity_rh",   humEvent.relative_humidity);
        }
    }

    if (bmpOk) {
        float pressure = bmp.readPressure();
        float bmpTemp  = bmp.readTemperature();
        if (!isnan(pressure) && pressure > 0) {
            envPoint.addField("pressure_hpa", pressure / 100.0F);
        }
        if (!isnan(bmpTemp)) {
            envPoint.addField("bmp_temperature_c", bmpTemp);
        }
    }

    if (envPoint.hasFields()) {
        if (!influx.writePoint(envPoint)) {
            Serial.printf("InfluxDB write error: %s\n",
                          influx.getLastErrorMessage().c_str());
        } else {
            Serial.printf("[%s] wrote point\n", DEVICE_LOCATION);
        }
    }

    delay(SEND_INTERVAL_MS);
}
