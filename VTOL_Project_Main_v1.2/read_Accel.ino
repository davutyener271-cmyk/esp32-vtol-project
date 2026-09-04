bool read_Accel() {

  uint8_t data[6] = { 0 };

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);

  byte communicationResult = Wire.endTransmission(false);

  if (communicationResult != 0) {
    return false;
  }
  byte incomingByte = Wire.requestFrom(MPU_addr, 6, true);

  if (incomingByte != 6 || Wire.available() < 6) {

    while (Wire.available()) {
      Wire.read();
    }
    return false;
  }

  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }
  int16_t newAcX = (int16_t)(((uint16_t)data[0] << 8) | data[1]);

  int16_t newAcY = (int16_t)(((uint16_t)data[2] << 8) | data[3]);

  int16_t newAcZ = (int16_t)(((uint16_t)data[4] << 8) | data[5]);

  AcX = newAcX;
  AcY = newAcY;
  AcZ = newAcZ;

  return true;
}