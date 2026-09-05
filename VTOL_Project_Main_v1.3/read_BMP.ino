uint16_t dig_T1 = 0;
int16_t dig_T2 = 0;
int16_t dig_T3 = 0;

uint16_t dig_P1 = 0;
int16_t dig_P2 = 0;
int16_t dig_P3 = 0;
int16_t dig_P4 = 0;
int16_t dig_P5 = 0;
int16_t dig_P6 = 0;
int16_t dig_P7 = 0;
int16_t dig_P8 = 0;
int16_t dig_P9 = 0;

int32_t bmp_tFine = 0;
int32_t bmpRawPressure = 0;
int32_t bmpRawTemperature = 0;


// BMP280'den istenilen register grubunu okur
bool read_BMP_Bytes(
  uint8_t registerAddress,
  uint8_t *data,
  size_t dataLength) {

  Wire.beginTransmission(BMP_addr);
  Wire.write(registerAddress);

  byte communicationResult =
    Wire.endTransmission(false);

  if (communicationResult != 0) {
    return false;
  }

  size_t incomingByte =
    Wire.requestFrom(
      (uint8_t)BMP_addr,
      dataLength,
      true);

  if (incomingByte != dataLength || Wire.available() < dataLength) {

    while (Wire.available()) {
      Wire.read();
    }

    return false;
  }

  for (size_t i = 0; i < dataLength; i++) {
    data[i] = Wire.read();
  }

  return true;
}


// BMP280 kimliğini kontrol eder
bool read_BMP_ID() {

  uint8_t chipID = 0;

  if (!read_BMP_Bytes(0xD0, &chipID, 1)) {
    return false;
  }

  return chipID == 0x58;
}


// BMP280'in fabrika kalibrasyon katsayılarını okur
bool read_BMP_Calibration() {

  uint8_t data[6] = { 0 };


  // Sıcaklık katsayıları: T1, T2, T3
  if (!read_BMP_Bytes(0x88, data, 6)) {
    return false;
  }

  dig_T1 =
    (uint16_t)(((uint16_t)data[1] << 8) | data[0]);

  dig_T2 =
    (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  dig_T3 =
    (int16_t)(((uint16_t)data[5] << 8) | data[4]);


  delay(1);


  // Basınç katsayıları: P1, P2, P3
  if (!read_BMP_Bytes(0x8E, data, 6)) {
    return false;
  }

  dig_P1 =
    (uint16_t)(((uint16_t)data[1] << 8) | data[0]);

  dig_P2 =
    (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  dig_P3 =
    (int16_t)(((uint16_t)data[5] << 8) | data[4]);


  delay(1);


  // Basınç katsayıları: P4, P5, P6
  if (!read_BMP_Bytes(0x94, data, 6)) {
    return false;
  }

  dig_P4 =
    (int16_t)(((uint16_t)data[1] << 8) | data[0]);

  dig_P5 =
    (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  dig_P6 =
    (int16_t)(((uint16_t)data[5] << 8) | data[4]);


  delay(1);


  // Basınç katsayıları: P7, P8, P9
  if (!read_BMP_Bytes(0x9A, data, 6)) {
    return false;
  }

  dig_P7 =
    (int16_t)(((uint16_t)data[1] << 8) | data[0]);

  dig_P8 =
    (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  dig_P9 =
    (int16_t)(((uint16_t)data[5] << 8) | data[4]);


  // P1 sıfırsa basınç hesabı yapılamaz
  if (dig_P1 == 0) {
    return false;
  }

  return true;
}


// BMP280 ölçüm ayarlarını yapar
bool configure_BMP() {

  // Önce uyku moduna al
  Wire.beginTransmission(BMP_addr);
  Wire.write(0xF4);
  Wire.write(0x00);

  if (Wire.endTransmission(true) != 0) {
    return false;
  }


  delay(1);


  // 62.5 ms bekleme ve IIR filtre katsayısı 4
  // Yaklaşık 10 Hz BMP okuma hızımıza daha uygundur
  Wire.beginTransmission(BMP_addr);
  Wire.write(0xF5);
  Wire.write(0x28);

  if (Wire.endTransmission(true) != 0) {
    return false;
  }


  delay(1);


  // Sıcaklık x1, basınç x4, normal ölçüm modu
  Wire.beginTransmission(BMP_addr);
  Wire.write(0xF4);
  Wire.write(0x2F);

  if (Wire.endTransmission(true) != 0) {
    return false;
  }

  return true;
}


// BMP280'i tamamen başlatır
bool initialize_BMP() {

  if (!read_BMP_ID()) {
    Serial.println("BMP280 kimlik kontrolu basarisiz.");
    return false;
  }

  delay(1);

  if (!read_BMP_Calibration()) {
    Serial.println("BMP280 katsayilari okunamadi.");
    return false;
  }

  delay(1);

  if (!configure_BMP()) {
    Serial.println("BMP280 ayarlanamadi.");
    return false;
  }

  return true;
}


// Ham basınç ve sıcaklık ölçümünü okur
bool read_BMP_Raw() {

  uint8_t data[6] = { 0 };

  if (!read_BMP_Bytes(0xF7, data, 6)) {
    return false;
  }

  bmpRawPressure =
    ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);

  bmpRawTemperature =
    ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);

  return true;
}


// Ham sıcaklığı santigrat dereceye dönüştürür
float calculate_BMP_Temperature() {

  int32_t var1;
  int32_t var2;

  var1 =
    (((bmpRawTemperature >> 3) - ((int32_t)dig_T1 << 1)) * (int32_t)dig_T2) >> 11;

  var2 =
    (((((bmpRawTemperature >> 4) - (int32_t)dig_T1) * ((bmpRawTemperature >> 4) - (int32_t)dig_T1)) >> 12) * (int32_t)dig_T3) >> 14;

  bmp_tFine = var1 + var2;

  int32_t temperatureHundredths =
    (bmp_tFine * 5 + 128) >> 8;

  return temperatureHundredths / 100.0f;
}


// Ham basıncı Pascal değerine dönüştürür
float calculate_BMP_Pressure() {

  int64_t var1;
  int64_t var2;
  int64_t pressure;

  var1 =
    (int64_t)bmp_tFine - 128000;

  var2 =
    var1 * var1 * (int64_t)dig_P6;

  var2 +=
    (var1 * (int64_t)dig_P5) << 17;

  var2 +=
    (int64_t)dig_P4 << 35;

  var1 =
    ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);

  var1 =
    (((((int64_t)1 << 47) + var1) * (int64_t)dig_P1) >> 33);

  if (var1 == 0) {
    return 0;
  }

  pressure =
    1048576 - (int64_t)bmpRawPressure;

  pressure =
    (((pressure << 31) - var2) * 3125) / var1;

  var1 =
    ((int64_t)dig_P9 * (pressure >> 13) * (pressure >> 13)) >> 25;

  var2 =
    ((int64_t)dig_P8 * pressure) >> 19;

  pressure =
    ((pressure + var1 + var2) >> 8) + ((int64_t)dig_P7 << 4);

  return pressure / 256.0f;
}


// Başlangıç basıncına göre irtifayı hesaplar
float calculate_BMP_Altitude() {

  if (bmpReferencePressure <= 0 || bmpPressure <= 0) {

    return 0;
  }

  float pressureRatio =
    bmpPressure / bmpReferencePressure;

  float altitude =
    44330.0f * (1.0f - pow(pressureRatio, 0.19029495f));

  return altitude;
}


// Açılış konumunu sıfır metre olarak ayarlar
bool calibrate_BMP_Altitude() {

  double totalPressure = 0;
  int successfulReading = 0;

  Serial.println(
    "BMP irtifa referansi aliniyor. Ucagi sabit tutun...");

  for (int i = 0; i < 50; i++) {

    delay(20);

    if (read_BMP_Raw()) {

      // Bu sıra önemlidir:
      // Önce sıcaklık, ardından basınç
      bmpTemperature =
        calculate_BMP_Temperature();

      bmpPressure =
        calculate_BMP_Pressure();

      if (bmpPressure >= 30000.0f && bmpPressure <= 110000.0f) {

        totalPressure += bmpPressure;
        successfulReading++;
      }
    }
  }

  if (successfulReading < 40) {

    Serial.print(
      "Basarili BMP referans okumasi: ");

    Serial.println(successfulReading);

    return false;
  }

  bmpReferencePressure =
    (float)(totalPressure / successfulReading);

  bmpAltitude = 0;
  Filtered_Altitude = 0;

  return true;
}