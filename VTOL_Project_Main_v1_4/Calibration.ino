bool kalibrasyonYap() {

  long toplamGX = 0;
  long toplamGY = 0;
  long toplamGZ = 0;

  float toplamPitch = 0;
  float toplamRoll = 0;

  const int ornekSayisi = 1000;
  int basariliOkuma = 0;

  Serial.println(
    "Kalibrasyon icin ucagi sabit ve duz tutun.");

  // İlk ölçümleri kullanma
  for (int i = 0; i < 100; i++) {
    read_Gyro();
    delay(1);
    read_Accel();
    delay(2);
  }

  for (int i = 0; i < ornekSayisi; i++) {

    bool gyroBasarili = read_Gyro();

    delay(1);

    bool accelBasarili = read_Accel();

    if (gyroBasarili && accelBasarili) {

      toplamGX += GyX;
      toplamGY += GyY;
      toplamGZ += GyZ;

      toplamPitch +=
        atan2(
          AcY,
          sqrt((float)AcX * AcX +
               (float)AcZ * AcZ)
        ) * 180.0f / PI;

      toplamRoll +=
        atan2(
          AcX,
          sqrt((float)AcY * AcY +
               (float)AcZ * AcZ)
        ) * 180.0f / PI;

      basariliOkuma++;
    }

    if (i % 100 < 50) {
      statusLedGreenOpen();
    } else {
      statusLedGreenClose();
    }

    delay(2);
  }

  statusLedGreenClose();

  // En az %90 başarılı okuma gerekli
  if (basariliOkuma < 900) {
    Serial.print("Kalibrasyon basarisiz. Okuma: ");
    Serial.println(basariliOkuma);
    return false;
  }

  gyroX_hata =
    (float)toplamGX / basariliOkuma;

  gyroY_hata =
    (float)toplamGY / basariliOkuma;

  gyroZ_hata =
    (float)toplamGZ / basariliOkuma;

  Pitch_offset =
    toplamPitch / basariliOkuma;

  Roll_offset =
    toplamRoll / basariliOkuma;

  Filtered_Pitch = 0;
  Filtered_Roll = 0;

  Serial.print("Gyro X hata: ");
  Serial.println(gyroX_hata);

  Serial.print("Gyro Y hata: ");
  Serial.println(gyroY_hata);

  Serial.print("Gyro Z hata: ");
  Serial.println(gyroZ_hata);

  Serial.print("Pitch offset: ");
  Serial.println(Pitch_offset);

  Serial.print("Roll offset: ");
  Serial.println(Roll_offset);

  return true;
}