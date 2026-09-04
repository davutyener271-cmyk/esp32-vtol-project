#include <Wire.h>

unsigned long lastTime = 0;
float ac_Pitch = 0;
float ac_Roll = 0;
float ac_Yaw = 0;
float gyro_Pitch = 0;
float gyro_Roll = 0;
float gyro_Yaw = 0;
float Pitch_offset = 0.79;
float Roll_offset = -2.66;
float Yaw_Offset = 0;
float Filtered_Roll = 0;
float Filtered_Pitch = 0;
float Filtered_Yaw = 0;
float gyroX_hata = -45.47;
float gyroY_hata = 46.75;
float gyroZ_hata = 49.47;

int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;

const int pinRed = 33;
const int pinGreen = 32;
const int pinBlue = 15;
const int pinBuzzer = 23;
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int MPU_addr = 0x68;

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

  // 1. Sensörü Uyandır
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

  Serial.println("Calibration is starting. Please keep your aircraft in a horizontal position...");
  kalibrasyonYap();
  Serial.println("Calibration is done !");
  tone(pinBuzzer, 2250);
  statusLedBlueOpen();
  delay(400);
  noTone(pinBuzzer);
  statusLedBlueClose();

  lastTime = millis();
}

void loop() {

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  bool gyroReadingSuccessful = read_Gyro();
  delay(1);
  bool accelReadingSuccessful = read_Accel();

  if (accelReadingSuccessful && gyroReadingSuccessful) {

    calculate_Direction(dt);

  } else {
    Serial.print("READ ERROR");

    Serial.print(", Accel:");
    Serial.print(accelReadingSuccessful);

    Serial.print(", Gyro:");
    Serial.println(gyroReadingSuccessful);
  }

  delay(100);
}
