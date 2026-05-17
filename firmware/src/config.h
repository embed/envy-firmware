#pragma once

// Copy this file to config.local.h and fill in your values.
// config.local.h is gitignored — never commit secrets.

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID        "your-ssid"
#define WIFI_PASSWORD    "your-password"

// ── InfluxDB 2.x ──────────────────────────────────────────────────────────────
#define INFLUXDB_URL     "http://192.168.1.100:8086"   // NAS IP
#define INFLUXDB_TOKEN   "your-influxdb-admin-token"
#define INFLUXDB_ORG     "home"
#define INFLUXDB_BUCKET  "environment"

// ── Device identity ───────────────────────────────────────────────────────────
// Unique per sensor node. Used as tags in InfluxDB.
#define DEVICE_ID        "nanoc6-01"          // e.g. nanoc6-01, nanoc6-02
#define DEVICE_LOCATION  "living_room"        // e.g. living_room, bedroom

// ── Timezone for NTP (used for accurate timestamps) ───────────────────────────
// See: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TZ_INFO          "CET-1CEST,M3.5.0,M10.5.0/3"   // Europe/Brussels

// ── I2C pins (M5NanoC6 Grove port) ───────────────────────────────────────────
#define I2C_SDA          2
#define I2C_SCL          1

// ── Sampling ──────────────────────────────────────────────────────────────────
#define SEND_INTERVAL_MS 30000    // 30 seconds
