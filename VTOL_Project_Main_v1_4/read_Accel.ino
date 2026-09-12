bool read_Accel() {

  uint8_t data[6] = {0};

  Wire.beginTransmission(LSM6DS3_addr);
  Wire.write(0x28); // OUTX_L_XL

  byte communicationResult = Wire.endTransmission(false);

  if (communicationResult != 0) {
    return false;
  }

  byte incomingByte =
      Wire.requestFrom(LSM6DS3_addr, (uint8_t)6, (uint8_t)true);

  if (incomingByte != 6 || Wire.available() < 6) {
    while (Wire.available()) {
      Wire.read();
    }
    return false;
  }

  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  // LSM6DS3: düşük byte önce gelir
  int16_t newAcX =
      (int16_t)(((uint16_t)data[1] << 8) | data[0]);

  int16_t newAcY =
      (int16_t)(((uint16_t)data[3] << 8) | data[2]);

  int16_t newAcZ =
      (int16_t)(((uint16_t)data[5] << 8) | data[4]);

  AcX = newAcX;
  AcY = newAcY;
  AcZ = newAcZ;

  return true;
}