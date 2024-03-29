#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include "../lib/keys.h"
#include "RoboCore_SMW_SX1276M0.h"
#include <Wire.h>


//This example shows the basic usage of Blynk platform and Firebase RTDB.
#include <WiFi.h>
#include <FirebaseESP32.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// Temperature/Humidity Sensor (DHT11)
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Temperature/Atmosferic Pressure Sensor (BMP280) 
// using SPI interface
#define BMP_SDA 21 
#define BMP_SCL 22
Adafruit_BMP280 bmp;
byte I2C_ADRESS = 0x76; // '../test/getI2C.ino'.

// UV Radiation Sensor (ML8511)
#define UV_PIN 15

// Rain Sensor (PDB10U)
#define RAIN_PIN 35
const unsigned long RAIN_TIME = 5000;
int val = 0;
int old_val = 0;
int REEDCOUNT = 0;

// Anemometer and Wind Vane (MISOL WH-SP-WD e MISOL WH-SP-WS01)
#define ANEMOMETER_PIN 26
#define VANE_PIN 34
#define CALC_INTERVAL 1000

boolean state = false;
const unsigned long WINDSPEED_TIME = 5000;
unsigned long lastMillis = 0;
float mps, kph;
int clicked, wspd, wdir, wdirRaw;


/* 1. Define the WiFi credentials */
#define WIFI_SSID "Conectando..."
#define WIFI_PASSWORD "Virus0503"

#define API_KEY "njGKJ8FiJ9GcdfnBgVfECYkSG8p9lhTaCkQbxLLB"
#define DATABASE_URL "https://esp32-firebase-weatherstation-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
FirebaseData fbdo;


/*================================ FUNCTIONS ================================*/

// DHT11 - Temperature
float getTemperature()
{
  return dht.readTemperature();
}

// DHT11 - Humidity
float getHumidity()
{
  return dht.readHumidity();
}

// BMP280 - Atmospheric Pressure
float getPressure()
{
  // bmp.readTemperature();
  // bmp.readAltitude(1019) 
  return bmp.readPressure()/100; // return pressure in hPa
}

// Map for UV reading
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Takes an average of readings on a given pin
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}

// ML8511 - UV Intensity
float getUV(int SensorPIN)
{
  int uvLevel = averageAnalogRead(SensorPIN);

  float outputVoltage = 3.3 * uvLevel/4095;
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);

  return uvIntensity;
}

// PDB10U - Rain Gauge
float getRain(int SensorPIN)
{
  lastMillis = xTaskGetTickCount();
  // It takes RAIN_TIME seconds to measure
  while(xTaskGetTickCount() - lastMillis < RAIN_TIME){
    val = digitalRead(SensorPIN);
    if (((val == LOW) && (old_val == HIGH)) || ((val == HIGH) && (old_val == LOW))) {           
      REEDCOUNT = REEDCOUNT + 1;   
      old_val = val;              
    } else {
      old_val = val;
    }
  }
  return REEDCOUNT * 0.5;
}

// MISOL WH-SP-WS01 - Anemometer (Wind Speed Sensor)
float getWindSpeed()
{
  lastMillis = xTaskGetTickCount();
  // It takes WINDSPEED_TIME seconds to measure wind speed
  while(xTaskGetTickCount() - lastMillis < WINDSPEED_TIME){
    if(digitalRead(ANEMOMETER_PIN) == HIGH) if(state == false){
        delay(50);
        clicked++;
        state = true;
    }
    if(digitalRead(ANEMOMETER_PIN) == LOW) state = false;
  }

  mps = clicked * 0.0333; // m/s
  kph = mps * 3.6;        // km/h
  wspd = int(mps*10);
  clicked = 0;

  return kph;
}

// MISOL WH-SP-WD - Wind Vane (Wind Directtion Sensor)
int getWindDirection()
{
  wdirRaw = analogRead(VANE_PIN);
  // V_REF = 3,3V; R_REF = 10K
  if(wdirRaw > 3225 && wdirRaw <= 3495) wdir = 0;             // 33k  - 0º    - N
  else if (wdirRaw > 1759  && wdirRaw <= 1879) wdir = 22.5;   // 6k57 - 22.5º - NNE
  else if (wdirRaw > 1879  && wdirRaw <= 2298) wdir = 45;     // 8k2  - 45º   - NE
  else if (wdirRaw > 324 && wdirRaw <= 383) wdir = 67.5;      // 891  - 67.5º - ENE
  else if (wdirRaw > 383 && wdirRaw <= 476) wdir = 90;        // 1k   - 90º   - E
  else if (wdirRaw >= 0 && wdirRaw <= 324) wdir = 112.5;      // 688  - 112.5º- ESE
  else if (wdirRaw > 674 && wdirRaw <= 930) wdir = 135;       // 2k2  - 135º  - SE
  else if (wdirRaw > 476 && wdirRaw <= 674) wdir = 157.5;     // 1k41 - 157.5º- SSE
  else if (wdirRaw > 1152 && wdirRaw <= 1502) wdir = 180;     // 3k9  - 180º  - S
  else if (wdirRaw > 930 && wdirRaw <= 1152) wdir = 202.5;    // 3k14 - 202.5º- SSW
  else if (wdirRaw > 2664 && wdirRaw <= 2887) wdir = 225;     // 16k  - 225º  - SW
  else if (wdirRaw > 2298 && wdirRaw <= 2664) wdir = 247.5;   // 14k12- 247.5º- WSW 
  else if (wdirRaw > 3969 && wdirRaw <= 4095) wdir = 270;     // 120k - 270º  - W
  else if (wdirRaw > 3495 && wdirRaw <= 3715) wdir = 292.5;   // 42k12- 292.5º- WNW
  else if (wdirRaw > 3715 && wdirRaw <= 3969) wdir = 315;     // 64k9 - 315º  - NW
  else if (wdirRaw > 2887 && wdirRaw <= 3225) wdir = 337.5;   // 21k88- 337.5º- NNW

  return wdir;
}

// Blink function for SENDING DATA status
void blink(int LED_PIN)
{
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}

// Print variables readings on Serial Monitor
void printData()
{
  Serial.println("===========================================================");
  
  Serial.println("---------- DHT11 ---------");
  Serial.print("Temperature: "); Serial.print(getTemperature()); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(getHumidity()); Serial.println(" %");
  
  Serial.println("--------- BMP280 ---------");
  Serial.print("Atmospheric Pressure: "); Serial.print(getPressure()); Serial.println(" hPa");
  
  Serial.println("--------- ML8511 ---------");
  Serial.print("UV Intensity: "); Serial.print(getUV(UV_PIN)); Serial.println(" mW/cm²");
  
  Serial.println("--------- PDB10U ---------");
  Serial.print("Precipitation: "); Serial.print(getRain(RAIN_PIN)); Serial.println(" mm");
  
  Serial.println("--------- WH-SP-WS01 ---------");
  Serial.print("Wind Speed: "); Serial.print(getWindSpeed()); Serial.println(" km/h");
  
  Serial.println("---------  WH-SP-WD ---------");
  Serial.print("Wind Direction: "); Serial.print(getWindDirection()); Serial.println(" °");


  Serial.println("===========================================================");
}

void setup()
{
  Serial.begin(115200);
  /*=============================== SENSORS ===============================*/
  pinMode(STATUS_LED, OUTPUT);
  // Temperature and Humidty Sensor DHT11
  dht.begin();
  // Pressure Sensor BMP280
  bmp.begin(I2C_ADRESS);
  // UV Sensor
  pinMode(UV_PIN, INPUT);
  // Rain gauge sensor
  pinMode (RAIN_PIN, INPUT_PULLUP); //This activates the internal pull up resistor
  // Wind wane sensor
  pinMode(ANEMOMETER_PIN, INPUT);
  /*=============================== WIFI ===============================*/
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /*=============================== FIREBASE ===============================*/
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  Firebase.begin(API_KEY, DATABASE_URL);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(fbdo, 60000);
  Firebase.setwriteSizeLimit(fbdo, 'tiny');
}

void loop()
{
  Firebase.setFloat(fbdo, "/temperature", getTemperature());
  Firebase.setInt(fbdo, "/latitude", latitude);
  Firebase.setInt(fbdo, "/longitude", longitude);
}