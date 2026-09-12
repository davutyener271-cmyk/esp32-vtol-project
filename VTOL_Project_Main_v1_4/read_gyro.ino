bool read_Gyro() {

  uint8_t data[6] = {0};

  Wire.beginTransmission(LSM6DS3_addr);
  Wire.write(0x22); // OUTX_L_G

  byte communicationResult =
    Wire.endTransmission(false);

  if (communicationResult != 0) {
    return false;
  }

  byte incomingByte =
    Wire.requestFrom(
      LSM6DS3_addr,
      (uint8_t)6,
      (uint8_t)true
    );

  if (incomingByte != 6 || Wire.available() < 6) {

    while (Wire.available()) {
      Wire.read();
    }

    return false;
  }

  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  // LSM6DS3 düşük byte'ı önce gönderir
  int16_t newGyX =
    (int16_t)(((uint16_t)data[1] << 8) | data[0]);

  int16_t newGyY =
    (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  int16_t newGyZ =
    (int16_t)(((uint16_t)data[5] << 8) | data[4]);

  GyX = newGyX;
  GyY = newGyY;
  GyZ = newGyZ;

  return true;
}