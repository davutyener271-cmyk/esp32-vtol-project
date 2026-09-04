#include <Wire.h>

bool bmpReady = false;
bool bmpAltitudeReady = false;
bool lastBMPReadingSuccessful = false;

unsigned long lastTime = 0;
unsigned long lastBMPTime = 0;
unsigned long lastSerialTime = 0;
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
float bmpTemperature = 0;
float bmpPressure = 0;
float bmpReferencePressure = 0;
float bmpAltitude = 0;
float Filtered_Altitude = 0;

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;

const int pinRed = 33;
const int pinGreen = 32;
const int pinBlue = 15;
const int pinBuzzer = 23;
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int MPU_addr = 0x68;
const uint8_t BMP_addr = 0x76;

void setup() {


  Serial.begin(115200);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinRed, OUTPUT);
  pinMode(pinGreen, OUTPUT);
  pinMode(pinBlue, OUTPUT);

  // Başlangıç Melodisi
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

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);

  delay(100);

  bmpReady = initialize_BMP();

  if (!bmpReady) {

    Serial.println(
      "BMP280 baslatilamadi.");
  }

  // BMP ve MPU I2C işlemlerinin arasına kısa süre bırak
  delay(1);

  // MPU6050'yi uyandır
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  delay(100);  // Çökmeyi engellemek için minik bir nefes

  // YENİ EKLENTİ: DLPF (Donanımsal Titreşim Filtresi) Açma (21Hz)
  // Uçak havadayken motor titremelerinin sensörü delirtmesini engeller!
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1A);
  Wire.write(0x04);
  Wire.endTransmission(true);

  // 2. Akrobasi Modunu Aç (2000 dps)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1B);
  Wire.write(0x18);
  Wire.endTransmission(true);

  // 3. İvmeölçer Aralığını ±8g'ye Çek (sert manevrada tavan yapmasın)
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x1C);
  Wire.write(0x10);
  Wire.endTransmission(true);


  delay(1000);

  Serial.println(
    "Calibration is starting. Please keep your aircraft in a horizontal position...");

  kalibrasyonYap();

  if (bmpReady) {

    delay(100);

    bmpAltitudeReady = calibrate_BMP_Altitude();

    if (!bmpAltitudeReady) {

      Serial.println("BMP irtifa referansi alinamadi.");
    }
  }

  Serial.println("Calibration is done !");
  tone(pinBuzzer, 2250);
  statusLedBlueOpen();
  delay(400);
  noTone(pinBuzzer);
  statusLedBlueClose();

  lastTime = millis();
  lastBMPTime = lastTime;
  lastSerialTime = lastTime;
}


void loop() {

  unsigned long currentTime = millis();

  float dt =
    (currentTime - lastTime) / 1000.0f;

  lastTime = currentTime;


  // MPU6050 jiroskop okuması
  bool gyroReadingSuccessful =
    read_Gyro();

  // I2C okumalarının birbirine karışmasını engeller
  delay(1);

  // MPU6050 ivmeölçer okuması
  bool accelReadingSuccessful =
    read_Accel();

  bool mpuReadingSuccessful =
    gyroReadingSuccessful && accelReadingSuccessful;


  // İki MPU ölçümü de başarılıysa yönü hesapla
  if (mpuReadingSuccessful) {

    calculate_Direction(dt);
  }


  // BMP280'i her 100 milisaniyede bir oku
  if (bmpReady && bmpAltitudeReady && currentTime - lastBMPTime >= 100) {

    lastBMPTime = currentTime;

    // MPU ve BMP I2C işlemlerini ayırır
    delay(1);

    lastBMPReadingSuccessful =
      read_BMP_Raw();


    if (lastBMPReadingSuccessful) {

      // Sıcaklık önce hesaplanmalıdır
      bmpTemperature =
        calculate_BMP_Temperature();

      // Basınç hesabı bmp_tFine değerini kullanır
      bmpPressure =
        calculate_BMP_Pressure();

      // Başlangıç noktasına göre ham irtifa
      bmpAltitude =
        calculate_BMP_Altitude();

      // Filtrelenmiş irtifa
      Filtered_Altitude = 0.90f * Filtered_Altitude + 0.10f * bmpAltitude;
    }
  }


  // Bütün değerleri Telemetry sekmesi yazdırır
  print_Telemetry(currentTime, mpuReadingSuccessful);
}
