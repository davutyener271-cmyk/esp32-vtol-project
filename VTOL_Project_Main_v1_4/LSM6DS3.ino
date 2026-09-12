// =====================================================
// LSM6DS3 KAYIT YAZMA
// =====================================================

bool write_LSM6DS3_Register(uint8_t reg, uint8_t value) {

  Wire.beginTransmission(LSM6DS3_addr);
  Wire.write(reg);
  Wire.write(value);

  uint8_t result = Wire.endTransmission(true);

  if (result != 0) {
    Serial.printf(
      "LSM yazma hatasi: adres=0x%02X kayit=0x%02X hata=%u\n",
      LSM6DS3_addr, reg, result
    );
    return false;
  }

  return true;
}


// =====================================================
// LSM6DS3 KAYIT OKUMA
// =====================================================

bool read_LSM6DS3_Register(uint8_t reg, uint8_t &value) {

  Wire.beginTransmission(LSM6DS3_addr);
  Wire.write(reg);

  uint8_t result = Wire.endTransmission(false);

  if (result != 0) {
    Serial.printf(
      "LSM okuma hazirlik hatasi: kayit=0x%02X hata=%u\n",
      reg, result
    );
    return false;
  }

  // Çalışan kısa testteki okuma çağrısı.
  uint8_t received =
    Wire.requestFrom(LSM6DS3_addr, (uint8_t)1);

  if (received != 1 || Wire.available() < 1) {
    Serial.printf(
      "LSM okuma basarisiz: adres=0x%02X kayit=0x%02X "
      "alinan=%u SDA=%d SCL=%d\n",
      LSM6DS3_addr,
      reg,
      received,
      digitalRead(SDA_PIN),
      digitalRead(SCL_PIN)
    );

    while (Wire.available()) {
      Wire.read();
    }

    return false;
  }

  value = Wire.read();
  return true;
}


// =====================================================
// LSM6DS3 BAŞLATMA
// =====================================================

bool initialize_LSM6DS3() {

  uint8_t chipID = 0;

  Serial.printf(
    "LSM baslatiliyor: adres=0x%02X SDA=%d SCL=%d\n",
    LSM6DS3_addr, SDA_PIN, SCL_PIN
  );

  if (!read_LSM6DS3_Register(0x0F, chipID)) {
    Serial.println("LSM kimlik kaydi okunamadi.");
    return false;
  }

  Serial.printf("WHO_AM_I: 0x%02X\n", chipID);

  if (chipID != 0x69) {
    Serial.println("Beklenen LSM6DS3 kimligi: 0x69.");
    return false;
  }

  // Yazılımsal sıfırlama.
  if (!write_LSM6DS3_Register(0x12, 0x01)) {
    return false;
  }

  // Sıfırlama bitinin temizlenmesini bekle.
  unsigned long resetStart = millis();
  uint8_t control = 0;

  while (true) {

    delay(2);

    if (!read_LSM6DS3_Register(0x12, control)) {
      return false;
    }

    if ((control & 0x01) == 0) {
      break;
    }

    if (millis() - resetStart >= 200) {
      Serial.println("LSM sifirlama zaman asimi.");
      return false;
    }
  }

  // BDU ve otomatik kayıt adresi artırma.
  if (!write_LSM6DS3_Register(0x12, 0x44)) {
    return false;
  }

  // İvmeölçer: 416 Hz, ±8 g.
  if (!write_LSM6DS3_Register(0x10, 0x6C)) {
    return false;
  }

  // Jiroskop: 416 Hz, ±2000 derece/saniye.
  if (!write_LSM6DS3_Register(0x11, 0x6C)) {
    return false;
  }

  delay(100);

  // Yazılan ayarları geri okuyarak doğrula.
  uint8_t accelSetting = 0;
  uint8_t gyroSetting = 0;
  uint8_t commonSetting = 0;

  if (!read_LSM6DS3_Register(0x10, accelSetting) ||
      !read_LSM6DS3_Register(0x11, gyroSetting) ||
      !read_LSM6DS3_Register(0x12, commonSetting)) {
    return false;
  }

  Serial.printf(
    "LSM ayarlar: XL=0x%02X G=0x%02X CTRL3=0x%02X\n",
    accelSetting, gyroSetting, commonSetting
  );

  if (accelSetting != 0x6C ||
      gyroSetting != 0x6C ||
      commonSetting != 0x44) {
    Serial.println("LSM ayarlari dogrulanamadi.");
    return false;
  }

  Serial.println("LSM6DS3 basariyla baslatildi.");
  return true;
}


