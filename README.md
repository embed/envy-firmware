# envy-log

Home environment monitoring with M5NanoC6 sensors and a self-hosted time-series stack.

## Hardware

- **MCU**: M5NanoC6 (ESP32-C6)
- **Sensor**: ENV.IV unit — SHT40 (temperature + humidity) + BMP280 (pressure)
- **Communication**: WiFi → HTTP to InfluxDB Line Protocol API

## Stack

| Component | Role |
|---|---|
| PlatformIO + Arduino | Firmware |
| InfluxDB 2.x | Time-series storage |
| Grafana 11.x | Dashboard |
| Docker Compose | Runs InfluxDB + Grafana on a NAS |

## Repository layout

```
firmware/           PlatformIO project (ESP32-C6, Arduino framework)
  src/config.h      Configuration template — copy to config.local.h
  src/main.cpp      Sensor read + InfluxDB write loop
infra/
  docker-compose.yml   InfluxDB + Grafana services
  .env.example         Environment variable template
  grafana/provisioning Auto-provisioned datasource and dashboard
```

## Setup

### 1. NAS — start the stack

```bash
cd infra
cp .env.example .env
# fill in .env: set passwords and generate a token with:
#   openssl rand -hex 32
docker compose up -d
```

- InfluxDB UI: `http://nas-ip:8086`
- Grafana: `http://nas-ip:3000`

### 2. Firmware — flash each sensor

```bash
cd firmware
cp src/config.h src/config.local.h
# edit config.local.h:
#   WIFI_SSID / WIFI_PASSWORD
#   INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET
#   DEVICE_ID, DEVICE_LOCATION  ← unique per board
#   TZ_INFO                     ← your timezone
pio run --target upload
pio device monitor
```

For each additional sensor, flash the same firmware with a different `DEVICE_LOCATION` (e.g. `bedroom`).

### 3. Dashboard

The Grafana dashboard auto-provisions on first start. Open Grafana → **Home** folder → **Home Environment**. Use the **Location** dropdown to filter or compare sensors.

## Measured fields

| Field | Sensor | Unit |
|---|---|---|
| `temperature_c` | SHT40 | °C |
| `humidity_rh` | SHT40 | % RH |
| `pressure_hpa` | BMP280 | hPa |
| `bmp_temperature_c` | BMP280 | °C |

All points are tagged with `location` and `device`.

## License

MIT
