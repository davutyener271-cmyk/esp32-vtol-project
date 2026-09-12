void print_Telemetry(
  unsigned long currentTime,
  bool imuReadingSuccessful) {

  
  if (currentTime - lastSerialTime < 50) {
    return;
  }

  lastSerialTime = currentTime;
  if (imuReadingSuccessful) {

    Serial.print("Pitch: ");
    Serial.print(Filtered_Pitch, 2);

    Serial.print(" | Roll: ");
    Serial.print(Filtered_Roll, 2);

    Serial.print(" | Yaw hizi: ");
    Serial.print(gyro_Yaw, 2);

  } else {

    Serial.print("LSM6DS3: HATA");
  }
/*

  if (bmpReady &&
      bmpAltitudeReady &&
      lastBMPReadingSuccessful) {

    Serial.print(" | Irtifa: ");
    Serial.print(Filtered_Altitude, 2);
    Serial.print(" m");

  } else {

    Serial.print(" | BMP: HATA");
  }
  */

  /*

  Serial.print(" | iBUS: ");

  if (iBusConnected) {
    Serial.print("OK");
  } else {
    Serial.print("HATA");
  }

  Serial.print(" | ARM: ");

  if (isArmed) {
    Serial.print("ACIK");
  } else {
    Serial.print("KAPALI");
  }

  Serial.print(" | Roll komut: ");
  Serial.print(rollCommand, 2);

  Serial.print(" | Pitch komut: ");
  Serial.print(pitchCommand, 2);

  Serial.print(" | Yaw komut: ");
  Serial.print(yawCommand, 2);

  Serial.print(" | Gaz: ");
  Serial.print(throttleCommand, 2);
*/

  
  Serial.println();
}