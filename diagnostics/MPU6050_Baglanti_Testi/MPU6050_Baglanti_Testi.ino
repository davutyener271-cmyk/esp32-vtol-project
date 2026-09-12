#include <Wire.h>
#include <math.h>

const uint8_t MPU_ADDR = 0x68;
const int SDA_PIN = 21;
const int SCL_PIN = 22;

// This diagnostic supports WHO_AM_I 0x68 (MPU6050) and 0x70 (MPU6500).
// I2C address remains 0x68 for both devices. Identity is NOT a bus address.
bool testReady = false;

struct ReadResult {
  uint8_t tx = 255;
  size_t requested = 0;
  int available = 0;
};

bool readBytes(uint8_t reg, uint8_t *data, size_t count, ReadResult &result) {
  result.tx = 255;
  result.requested = 0;
  result.available = 0;
  while (Wire.available()) {
    Wire.read();
  }

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  result.tx = Wire.endTransmission(false);

  if (result.tx != 0) {
    return false;
  }

  result.requested = Wire.requestFrom(MPU_ADDR, count, true);
  result.available = Wire.available();

  if (result.requested != count || result.available < (int)count) {
    while (Wire.available()) {
      Wire.read();
    }
    return false;
  }

  for (size_t i = 0; i < count; i++) {
    int value = Wire.read();
    if (value < 0) {
      while (Wire.available()) Wire.read();
      return false;
    }
    data[i] = (uint8_t)value;
  }

  return true;
}

bool readRegister(uint8_t reg, uint8_t &value) {
  ReadResult result;
  return readBytes(reg, &value, 1, result);
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);

  uint8_t result = Wire.endTransmission(true);

  Serial.print("YAZ 0x");
  if (reg < 0x10) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print(" = 0x");
  if (value < 0x10) Serial.print("0");
  Serial.print(value, HEX);
  Serial.print(" | TX:");
  Serial.println(result);

  return result == 0;
}

bool writeAndVerify(uint8_t reg, uint8_t value) {
  if (!writeRegister(reg, value)) {
    return false;
  }

  delay(5);

  uint8_t readBack = 0;

  if (!readRegister(reg, readBack)) {
    Serial.println("GERI OKUMA HATASI");
    return false;
  }

  Serial.print("GERI OKU 0x");
  if (reg < 0x10) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print(" = 0x");
  if (readBack < 0x10) Serial.print("0");
  Serial.print(readBack, HEX);

  if (readBack == value) {
    Serial.println(" | DOGRU");
    return true;
  }

  Serial.println(" | UYUSMUYOR");
  return false;
}

void printRegister(const char *name, uint8_t reg) {
  uint8_t value = 0;

  Serial.print(name);
  Serial.print(" [0x");
  if (reg < 0x10) Serial.print("0");
  Serial.print(reg, HEX);
  Serial.print("] = ");

  if (!readRegister(reg, value)) {
    Serial.println("OKUNAMADI");
    return;
  }

  Serial.print("0x");
  if (value < 0x10) Serial.print("0");
  Serial.println(value, HEX);
}

void scanI2C() {
  Serial.println("I2C taramasi:");

  int found = 0;

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t result = Wire.endTransmission(true);

    if (result == 0) {
      Serial.print("Cihaz bulundu: 0x");
      if (address < 0x10) Serial.print("0");
      Serial.println(address, HEX);
      found++;
    }
  }

  Serial.print("Toplam cihaz: ");
  Serial.println(found);
  Serial.println("Beklenen: MPU=0x68, BMP=0x76");
}

void printRawBytes(const uint8_t data[6]) {
  for (int i = 0; i < 6; i++) {
    if (data[i] < 0x10) Serial.print("0");
    Serial.print(data[i], HEX);
    if (i < 5) Serial.print(" ");
  }
}

int16_t combineBytes(uint8_t highByte, uint8_t lowByte) {
  return (int16_t)(((uint16_t)highByte << 8) | lowByte);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("===== MPU BAGLANTI TESTI v2 (6050 / 6500) =====");
  Serial.println("Bu kod motor ve servo calistirmaz.");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(50);
  delay(100);

  scanI2C();

  Serial.println();
  Serial.println("Ilk register durumlari:");
  printRegister("WHO_AM_I", 0x75);
  printRegister("PWR_MGMT_1", 0x6B);
  printRegister("PWR_MGMT_2", 0x6C);
  printRegister("CONFIG", 0x1A);
  printRegister("GYRO_CONFIG", 0x1B);
  printRegister("ACCEL_CONFIG", 0x1C);

  uint8_t whoAmI = 0;
  bool identityStable = true;

  Serial.println("Kimlik 10 kez kontrol ediliyor:");
  for (int i = 0; i < 10; i++) {
    uint8_t currentID = 0;
    Serial.print("ID[");
    Serial.print(i);
    Serial.print("] = ");
    if (!readRegister(0x75, currentID)) {
      identityStable = false;
      Serial.println("OKUMA HATASI");
    } else {
      Serial.print("0x");
      if (currentID < 0x10) Serial.print("0");
      Serial.println(currentID, HEX);
      if (i == 0) whoAmI = currentID;
      if (currentID != whoAmI) identityStable = false;
    }
    delay(20);
  }

  if (!identityStable) {
    Serial.println("TEST DURDU: Kimlik okumalari kararsiz veya eksik.");
    Serial.println("Besleme, GND ve I2C baglantilarini kontrol et.");
    return;
  }

  if (whoAmI == 0x68) {
    Serial.println("MODEL: MPU6050 kimligi (0x68)");
  } else if (whoAmI == 0x70) {
    Serial.println("MODEL: MPU6500 kimligi (0x70)");
    printRegister("ACCEL_CONFIG2", 0x1D);
  } else {
    Serial.println("TEST DURDU: Cihaz cevapliyor fakat kimligi desteklenmiyor.");
    Serial.println("Bu sonuc tek basina baglanti hatasi demek degildir.");
    return;
  }

  Serial.println();
  Serial.println("MPU sifirlaniyor ve ayarlar zorla yeniden yaziliyor:");

  if (!writeRegister(0x6B, 0x80)) {
    Serial.println("TEST DURDU: Reset komutu gonderilemedi.");
    return;
  }
  delay(150);

  uint8_t idAfterReset = 0;
  if (!readRegister(0x75, idAfterReset) || idAfterReset != whoAmI) {
    Serial.println("TEST DURDU: Reset sonrasi kimlik dogrulanamadi.");
    return;
  }

  bool settingsOK = true;
  settingsOK &= writeAndVerify(0x6B, 0x00);  // Uyku kapali
  settingsOK &= writeAndVerify(0x6C, 0x00);  // Tum gyro/accel eksenleri acik
  settingsOK &= writeAndVerify(0x1A, 0x04);  // DLPF
  settingsOK &= writeAndVerify(0x1B, 0x18);  // Gyro +-2000 dps
  settingsOK &= writeAndVerify(0x1C, 0x10);  // Accel +-8g
  if (whoAmI == 0x70) {
    settingsOK &= writeAndVerify(0x1D, 0x04);  // MPU6500 accel DLPF
  }

  Serial.println();

  if (settingsOK) {
    Serial.println("AYAR SONUCU: Butun registerlar dogru yazildi.");
  } else {
    Serial.println("AYAR SONUCU: Yazma/geri okuma hatasi var.");
    Serial.println("TEST DURDU: Ayarlar dogrulanmadan olcum yapilmayacak.");
    return;
  }

  delay(200);  // Olcumlerin baslamasi icin bekle.
  testReady = true;
  Serial.println("Canli test basliyor. MPU'yu uc eksende hareket ettir.");
  Serial.println();
}

void loop() {
  if (!testReady) {
    delay(20);
    return;
  }
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint < 250) {
    return;
  }

  lastPrint = millis();

  uint8_t accelData[6] = { 0 };
  uint8_t gyroData[6] = { 0 };
  ReadResult accelResult;
  ReadResult gyroResult;

  bool accelOK = readBytes(0x3B, accelData, 6, accelResult);
  bool gyroOK = readBytes(0x43, gyroData, 6, gyroResult);

  if (!accelOK) {
    Serial.print("ACC HATA | TX:");
    Serial.print(accelResult.tx);
    Serial.print(" Gelen:");
    Serial.print(accelResult.requested);
    Serial.print(" Available:");
    Serial.println(accelResult.available);
  } else {
    int16_t ax = combineBytes(accelData[0], accelData[1]);
    int16_t ay = combineBytes(accelData[2], accelData[3]);
    int16_t az = combineBytes(accelData[4], accelData[5]);

    float magnitude = sqrtf(
      (float)ax * ax +
      (float)ay * ay +
      (float)az * az
    ) / 4096.0f;

    Serial.print("ACC  [");
    printRawBytes(accelData);
    Serial.print("] | X:");
    Serial.print(ax);
    Serial.print(" Y:");
    Serial.print(ay);
    Serial.print(" Z:");
    Serial.print(az);
    Serial.print(" | Toplam:");
    Serial.print(magnitude, 2);
    Serial.println(" g");
  }

  if (!gyroOK) {
    Serial.print("GYRO HATA | TX:");
    Serial.print(gyroResult.tx);
    Serial.print(" Gelen:");
    Serial.print(gyroResult.requested);
    Serial.print(" Available:");
    Serial.println(gyroResult.available);
  } else {
    int16_t gx = combineBytes(gyroData[0], gyroData[1]);
    int16_t gy = combineBytes(gyroData[2], gyroData[3]);
    int16_t gz = combineBytes(gyroData[4], gyroData[5]);

    Serial.print("GYRO [");
    printRawBytes(gyroData);
    Serial.print("] | X:");
    Serial.print(gx);
    Serial.print(" Y:");
    Serial.print(gy);
    Serial.print(" Z:");
    Serial.print(gz);

    if (gx == 0 && gy == 0 && gz == 0) {
      Serial.print(" | UYARI: UC EKSEN TAM SIFIR");
    }

    Serial.println();
  }

  Serial.println("----------------------------------------");
}
