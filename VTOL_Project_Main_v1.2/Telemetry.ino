void print_Telemetry(
  unsigned long currentTime,
  bool mpuReadingSuccessful
) {

  // Seri monitöre yalnızca 100 ms'de bir yazdır
  if (currentTime - lastSerialTime < 100) {
    return;
  }

  lastSerialTime = currentTime;


  if (mpuReadingSuccessful) {

    Serial.print("Pitch: ");
    Serial.print(Filtered_Pitch, 2);

    Serial.print(" | Roll: ");
    Serial.print(Filtered_Roll, 2);

    // Gyro Z açı değil, derece/saniye dönüş hızıdır
    Serial.print(" | Yaw hizi: ");
    Serial.print(gyro_Yaw, 2);

  } else {

    Serial.print("MPU: HATA");
  }


  if (bmpReady &&
      bmpAltitudeReady &&
      lastBMPReadingSuccessful) {

    Serial.print(" | Sicaklik: ");
    Serial.print(bmpTemperature, 2);
    Serial.print(" C");

    Serial.print(" | Basinc: ");
    Serial.print(bmpPressure / 100.0f, 2);
    Serial.print(" hPa");

    Serial.print(" | Irtifa: ");
    Serial.print(Filtered_Altitude, 2);
    Serial.print(" m");

  } else {

    Serial.print(" | BMP: HATA");
  }


  Serial.println();
}