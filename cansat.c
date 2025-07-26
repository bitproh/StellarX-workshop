#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_BMP280.h>
#include <MPU9250_asukiaaa.h>

// --- WiFi Credentials ---
const char* ssid = "Buga";
const char* password = "12345678";

// --- Web Server ---
WebServer server(80);

// --- BMP280 Sensor ---
#define BMP280_I2C_ADDRESS 0x76
Adafruit_BMP280 bmp;

// --- MPU9250 Sensor ---
MPU9250_asukiaaa mySensor;

// --- Sensor Values ---
float bmp_temperature = 0, pressure = 0, altitude = 0;
float ax, ay, az, gx, gy, gz, mx, my, mz;

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>CanSat Dashboard</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; text-align: center; margin-top: 40px; }
        h1 { color: #444; }
        .reading { font-size: 1.3rem; margin: 8px; }
      </style>
    </head>
    <body>
      <h1>ESP32 Sensor Readings</h1>

      <div class="reading" id="bmpTemp">BMP Temp: --</div>
      <div class="reading" id="pressure">Pressure: --</div>
      <div class="reading" id="altitude">Altitude: --</div>

      <div class="reading" id="accel">Accel (x,y,z): --</div>
      <div class="reading" id="gyro">Gyro (x,y,z): --</div>
      <div class="reading" id="mag">Mag (x,y,z): --</div>

      <script>
        setInterval(() => {
          fetch('/data')
            .then(response => response.json())
            .then(data => {
              document.getElementById('bmpTemp').innerHTML = "BMP Temp: " + data.bmp_temp + " °C";
              document.getElementById('pressure').innerHTML = "Pressure: " + data.pressure + " hPa";
              document.getElementById('altitude').innerHTML = "Altitude: " + data.altitude + " m";
              document.getElementById('accel').innerHTML = "Accel (x,y,z): " + data.ax + ", " + data.ay + ", " + data.az;
              document.getElementById('gyro').innerHTML = "Gyro (x,y,z): " + data.gx + ", " + data.gy + ", " + data.gz;
              document.getElementById('mag').innerHTML = "Mag (x,y,z): " + data.mx + ", " + data.my + ", " + data.mz;
            });
        }, 2000);
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"bmp_temp\":" + String(bmp_temperature, 2) + ",";
  json += "\"pressure\":" + String(pressure, 2) + ",";
  json += "\"altitude\":" + String(altitude, 2) + ",";
  json += "\"ax\":" + String(ax, 2) + ",";
  json += "\"ay\":" + String(ay, 2) + ",";
  json += "\"az\":" + String(az, 2) + ",";
  json += "\"gx\":" + String(gx, 2) + ",";
  json += "\"gy\":" + String(gy, 2) + ",";
  json += "\"gz\":" + String(gz, 2) + ",";
  json += "\"mx\":" + String(mx, 2) + ",";
  json += "\"my\":" + String(my, 2) + ",";
  json += "\"mz\":" + String(mz, 2);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- Init BMP280 ---
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("BMP280 not found!");
    while (1);
  }

  // --- Init MPU9250 ---
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();
  mySensor.beginMag();

  // --- Connect WiFi ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // --- Start Web Server ---
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // --- Read BMP280 ---
  bmp_temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(1013.25);

  // --- Read MPU9250 ---
  mySensor.accelUpdate();
  mySensor.gyroUpdate();
  mySensor.magUpdate();

  ax = mySensor.accelX();
  ay = mySensor.accelY();
  az = mySensor.accelZ();

  gx = mySensor.gyroX();
  gy = mySensor.gyroY();
  gz = mySensor.gyroZ();

  mx = mySensor.magX();
  my = mySensor.magY();
  mz = mySensor.magZ();

  // --- Serve Web Page ---
  server.handleClient();
  delay(2000);
}
