#include <Wire.h>

unsigned long sonZaman = 0;
float ivme_Pitch = 0;
float ivme_Roll = 0;
float gyro_Pitch = 0;
float gyro_Roll = 0;
float Pitch_offset = 0;
float Roll_offset = 0;
float Filtreli_Roll = 0;
float Filtreli_Pitch = 0;
float gyroX_hata = 0;
float gyroY_hata = 0;



int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;


const int pinRed = 33;
const int pinGreen = 32;
const int pinBlue = 15;
const int pinBuzzer = 23;
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int MPU_addr = 0x68;

void statusLedRedOpen() {

  digitalWrite(pinRed, HIGH);
}
void statusLedBlueOpen() {
  digitalWrite(pinBlue,HIGH);
}
void statusLedGreenOpen() {
  digitalWrite(pinGreen,HIGH);
}
void statusLedRedClose() {
  digitalWrite(pinRed, LOW);
}
void statusLedBlueClose() {
  digitalWrite(pinBlue, LOW);
}
void statusLedGreenClose() {
  digitalWrite(pinGreen, LOW);
}


  // Açılışta, uçak SABİT ve DÜZ dururken jiroskop + ivmeölçer sapmasını ölçer
void kalibrasyonYap() {
  long toplamGX = 0, toplamGY = 0;
  float toplamPitch = 0, toplamRoll = 0;
  const int ornekSayisi = 1000;
  int basariliOkuma = 0; 

  Serial.println("I2C Hatti temizleniyor ve sensor isitiliyor...");
  
  // YENİ EKLENTİ: Çöpe Atma Döngüsü (İlk 100 okumadaki çöp verileri yutar)
  for(int i = 0; i < 100; i++) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);
    while(Wire.available()) {
      Wire.read(); // Gelen veriyi hiçbir değişkene atamadan çöpe at
    }
    delay(3);
  }

  Serial.println("Temizlik bitti, kalibrasyon basliyor...");
  for (int i = 0; i < ornekSayisi; i++) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);

    if (Wire.available() >= 14) {
      int16_t ax = Wire.read() << 8 | Wire.read();
      int16_t ay = Wire.read() << 8 | Wire.read();
      int16_t az = Wire.read() << 8 | Wire.read();
      Wire.read();
      Wire.read();  // Tmp - kullanılmıyor
      int16_t gx = Wire.read() << 8 | Wire.read();
      int16_t gy = Wire.read() << 8 | Wire.read();
      Wire.read();
      Wire.read();  // GyZ - kullanılmıyor

      toplamGX += gx;
      toplamGY += gy;
      toplamPitch += atan2(ay, sqrt((float)ax * ax + (float)az * az)) * 180.0 / PI;
      toplamRoll += atan2(ax, sqrt((float)ay * ay + (float)az * az)) * 180.0 / PI;
      
      basariliOkuma++; // Veriyi başarıyla aldıysak sayacı artırıyoruz
    }
    
    // Kusursuz çalışan flaşör efekti
    if (i % 100 < 50) {
      statusLedGreenOpen();
    } else {
      statusLedGreenClose();
    }

    delay(2);
  }

  // Kalibrasyon bitince LED'i kapat
  statusLedGreenClose();

  // KRİTİK NOKTA: Toplamları 1000'e değil, "gerçekten" okunan sayıya bölüyoruz!
  if (basariliOkuma > 0) {
    gyroX_hata = toplamGX / basariliOkuma;
    gyroY_hata = toplamGY / basariliOkuma;
    Pitch_offset = toplamPitch / basariliOkuma;
    Roll_offset = toplamRoll / basariliOkuma;
  }
}

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

  Serial.println("Kalibrasyon basliyor, ucagi sabit ve DUZ tutun...");
  kalibrasyonYap();
  Serial.println("Kalibrasyon tamam!");
  tone(pinBuzzer, 2250);
  statusLedBlueOpen();
  delay(400);
  noTone(pinBuzzer);
  statusLedBlueClose();

  sonZaman = millis();
}

void loop() {
  unsigned long simdikiZaman = millis();
  float dt = (simdikiZaman - sonZaman) / 1000.0;
  sonZaman = simdikiZaman;

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  if (Wire.available() >= 14) {
    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();
  }

  // Sensör verilerini ondalıklı sayıya çevir
  float aX = AcX;
  float aY = AcY;
  float aZ = AcZ;

  // 1. İVME MATEMATİĞİ (Röntgenden Çıkan Kusursuz Eksenlerin)
  ivme_Pitch = (atan2(aY, sqrt((aX * aX) + (aZ * aZ))) * 180.0 / PI) - Pitch_offset;
  ivme_Roll = (atan2(aX, sqrt((aY * aY) + (aZ * aZ))) * 180.0 / PI) - Roll_offset;

  // 2. JİROSKOP MATEMATİĞİ (Kalibrasyon Hataları Çıkarıldı ve Limit 16.4'e Çekildi)
  gyro_Pitch = (GyX - gyroX_hata) / 16.4;
  gyro_Roll = -(GyY - gyroY_hata) / 16.4;  // Senin testinde bulduğumuz o hayat kurtaran EKSİ işareti!

  // 3. TAMAMLAYICI FİLTRE
  Filtreli_Pitch = 0.95 * (Filtreli_Pitch + (gyro_Pitch * dt)) + 0.05 * ivme_Pitch;
  Filtreli_Roll = 0.95 * (Filtreli_Roll + (gyro_Roll * dt)) + 0.05 * ivme_Roll;

  // Ekrana Yazdır
  // Ekrana Yazdır (Seri Çizici İçin Tam Format)
  Serial.print("Filtreli_Pitch:");
  Serial.print(Filtreli_Pitch);
  Serial.print(","); 
  
  Serial.print("Ivme_Pitch:");
  Serial.print(ivme_Pitch);
  Serial.print(",");
  
  Serial.print("Gyro_Pitch:");
  Serial.print(gyro_Pitch);
  Serial.print(",");
  
  Serial.print("Filtreli_Roll:");
  Serial.print(Filtreli_Roll);
  Serial.print(",");
  
  Serial.print("Ivme_Roll:");
  Serial.print(ivme_Roll);
  Serial.print(",");
  
  Serial.print("Gyro_Roll:");
  Serial.println(gyro_Roll); // Son değer alt satıra geçirir

  // Uçuş sırasında loop'un gecikmemesi için bekleme süresi 10ms'de kalmalı
  delay(10);
}
