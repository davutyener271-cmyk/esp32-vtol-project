#include <Wire.h>
#include <ESP32Servo.h>

// =====================================================
// HABERLESME VE SISTEM DURUMLARI
// =====================================================

HardwareSerial iBusSerial(2);

bool lsmReady = false;
bool bmpReady = false;
bool bmpAltitudeReady = false;
bool lastBMPReadingSuccessful = false;
bool iBusConnected = false;
bool rcFailsafeActive = true;
bool isArmed = false;
bool armSwitchWasOff = false;
bool ledIsOn = false;

// =====================================================
// ZAMAN DEGISKENLERI
// =====================================================

unsigned long lastTime = 0;
unsigned long lastBMPTime = 0;
unsigned long lastSerialTime = 0;
unsigned long lastIBusFrameTime = 0;
unsigned long lastLedTime = 0;

// =====================================================
// YON VE SENSOR DEGISKENLERI
// =====================================================

float ac_Pitch = 0;
float ac_Roll = 0;
float ac_Yaw = 0;

float gyro_Pitch = 0;
float gyro_Roll = 0;
float gyro_Yaw = 0;

float Pitch_offset = 0;
float Roll_offset = 0;
float Yaw_Offset = 0;

float Filtered_Roll = 0;
float Filtered_Pitch = 0;
float Filtered_Yaw = 0;

float gyroX_hata = 0;
float gyroY_hata = 0;
float gyroZ_hata = 0;

int16_t AcX = 0;
int16_t AcY = 0;
int16_t AcZ = 0;
int16_t GyX = 0;
int16_t GyY = 0;
int16_t GyZ = 0;

// =====================================================
// BMP280 DEGISKENLERI
// =====================================================

float bmpTemperature = 0;
float bmpPressure = 0;
float bmpReferencePressure = 0;
float bmpAltitude = 0;
float Filtered_Altitude = 0;

// =====================================================
// KUMANDA DEGISKENLERI
// =====================================================

float rollCommand = 0.0f;
float pitchCommand = 0.0f;
float yawCommand = 0.0f;
float throttleCommand = 0.0f;

uint16_t iBusChannels[14] = { 0 };

// =====================================================
// SERVO NESNELERI VE PINLER
// =====================================================

Servo leftTailServo;
Servo rightTailServo;
Servo rightAileronServo;
Servo leftAileronServo;
Servo leftTiltServo;
Servo rightTiltServo;

const int LEFT_TAIL_SERVO_PIN = 13;
const int RIGHT_TAIL_SERVO_PIN = 14;
const int RIGHT_AILERON_SERVO_PIN = 26;
const int LEFT_AILERON_SERVO_PIN = 27;
const int LEFT_TILT_SERVO_PIN = 23;
const int RIGHT_TILT_SERVO_PIN = 25;

const int pinRed = 33;
const int pinGreen = 32;
const int pinBlue = 15;
const int pinBuzzer = 2;

const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int IBUS_RX_PIN = 4;

const uint8_t LSM6DS3_addr = 0x6B;
uint8_t BMP_addr = 0x76;

// BMP280 kartina gore adres 0x76 veya 0x77 olabilir.
bool find_BMP_Address() {
  const uint8_t addresses[] = { 0x76, 0x77 };

  for (uint8_t address : addresses) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission(true) == 0) {
      BMP_addr = address;
      return true;
    }
  }

  return false;
}

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("VTOL sistemi baslatiliyor...");

  // I2C'yi LSM islemlerinden once baslat.
  if (!Wire.begin(SDA_PIN, SCL_PIN, 100000)) {
    Serial.println("I2C baslatilamadi.");
    while (true) {
      delay(1000);
    }
  }

  Wire.setTimeOut(20);
  delay(100);

  // iBUS alicisi.
  iBusSerial.begin(115200, SERIAL_8N1, IBUS_RX_PIN, -1);

  // LED ve buzzer pinleri.
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinRed, OUTPUT);
  pinMode(pinGreen, OUTPUT);
  pinMode(pinBlue, OUTPUT);

  // Baslangic melodisi.
  tone(pinBuzzer, 2000);
  statusLedGreenOpen();
  delay(70);

  noTone(pinBuzzer);
  statusLedGreenClose();
  delay(40);

  tone(pinBuzzer, 2000);
  statusLedGreenOpen();
  delay(150);

  noTone(pinBuzzer);
  statusLedGreenClose();
  delay(70);

  tone(pinBuzzer, 2250);
  statusLedGreenOpen();
  delay(400);

  noTone(pinBuzzer);
  statusLedGreenClose();

  // LSM6DS3'u baslat.
  lsmReady = initialize_LSM6DS3();

  if (!lsmReady) {
    Serial.println("LSM6DS3 baslatilamadi.");
    statusLedRedOpen();
    while (true) {
      delay(1000);
    }
  }

  // Sensor sabitken sifir hatalarini hesapla.
  delay(1000);
  Serial.println("Kalibrasyon basliyor. Sensoru sabit ve duz tutun.");

  lsmReady = kalibrasyonYap();

  if (!lsmReady) {
    Serial.println("LSM6DS3 kalibrasyonu basarisiz.");
    statusLedRedOpen();
    while (true) {
      delay(1000);
    }
  }

  Serial.println("LSM6DS3 kalibrasyonu tamamlandi.");

  // BMP280 adresini bul ve sensoru baslat.
  bmpReady = find_BMP_Address();

  if (!bmpReady) {
    Serial.println("BMP280 0x76 veya 0x77 adresinde bulunamadi.");
  } else {
    Serial.printf("BMP280 I2C adresi: 0x%02X\n", BMP_addr);
    bmpReady = initialize_BMP();

    if (!bmpReady) {
      Serial.println("BMP280 baslatilamadi.");
    }
  }

  // Acilis konumunu sifir metre olarak kaydet.
  if (bmpReady) {
    delay(100);
    bmpAltitudeReady = calibrate_BMP_Altitude();

    if (!bmpAltitudeReady) {
      Serial.println("BMP irtifa referansi alinamadi.");
    } else {
      Serial.printf(
        "BMP irtifa referansi hazir: %.2f hPa\n",
        bmpReferencePressure / 100.0f);
    }
  }

  lastBMPReadingSuccessful = bmpAltitudeReady;

  // Basarili kalibrasyon bildirimi.
  tone(pinBuzzer, 2250);
  statusLedBlueOpen();
  delay(400);
  noTone(pinBuzzer);
  statusLedBlueClose();

  initialize_Servos();
  initialize_Tilt_Servos();

  lastTime = millis();
  lastBMPTime = lastTime;
  lastSerialTime = lastTime;
  lastLedTime = lastTime;

  Serial.println("VTOL sistemi kullanima hazir.");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
  unsigned long currentTime = millis();

  // Kumanda ve guvenlik durumu.
  read_IBUS();
  update_RC_Failsafe();
  update_Arm();

  if (!rcFailsafeActive) {
    calculate_RC_Commands();
  } else {
    rollCommand = 0.0f;
    pitchCommand = 0.0f;
    yawCommand = 0.0f;
    throttleCommand = 0.0f;
  }

  update_All_Servos();
  update_Tilt_Target();
  update_Tilt_Servos(currentTime);

  // Durum LED'i.
  if (!ledIsOn && currentTime - lastLedTime >= 2000) {
    statusLedRedOpen();
    statusLedBlueOpen();
    ledIsOn = true;
    lastLedTime = currentTime;
  } else if (ledIsOn && currentTime - lastLedTime >= 150) {
    statusLedRedClose();
    statusLedBlueClose();
    ledIsOn = false;
    lastLedTime = currentTime;
  }

  // Dongu suresi.
  float dt = (currentTime - lastTime) / 1000.0f;
  lastTime = currentTime;

  // LSM6DS3 okumalari.
  bool gyroReadingSuccessful = read_Gyro();
  delay(1);
  bool accelReadingSuccessful = read_Accel();

  bool imuReadingSuccessful =
    gyroReadingSuccessful && accelReadingSuccessful;

  if (imuReadingSuccessful) {
    calculate_Direction(dt);
  }

  // BMP280 baglanip etkinlestirildiginde calisir.
  if (bmpReady && bmpAltitudeReady && currentTime - lastBMPTime >= 100) {
    lastBMPTime = currentTime;
    lastBMPReadingSuccessful = read_BMP_Raw();

    if (lastBMPReadingSuccessful) {
      bmpTemperature = calculate_BMP_Temperature();
      bmpPressure = calculate_BMP_Pressure();
      bmpAltitude = calculate_BMP_Altitude();
      Filtered_Altitude =
        0.90f * Filtered_Altitude + 0.10f * bmpAltitude;
    }
  }

  print_Telemetry(currentTime, imuReadingSuccessful);
}