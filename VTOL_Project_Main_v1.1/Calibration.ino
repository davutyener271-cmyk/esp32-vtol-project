// Açılışta, uçak SABİT ve DÜZ dururken jiroskop + ivmeölçer sapmasını ölçer
void kalibrasyonYap() {
  long toplamGX = 0, toplamGY = 0, toplamGZ = 0;
  float toplamPitch = 0, toplamRoll = 0, toplamYaw = 0;
  const int ornekSayisi = 1000;
  int basariliOkuma = 0;

  Serial.println("I2C Hatti temizleniyor ve sensor isitiliyor...");

  // YENİ EKLENTİ: Çöpe Atma Döngüsü (İlk 100 okumadaki çöp verileri yutar)
  for (int i = 0; i < 100; i++) {
    read_Gyro();
    delay(1);
    read_Accel();
    delay(2);
  }

  Serial.println("Temizlik bitti, kalibrasyon basliyor...");
  for (int i = 0; i < ornekSayisi; i++) {

    bool gyroBasarili = read_Gyro();
    delay(1);
    bool accelBasarili = read_Accel();

    if (gyroBasarili && accelBasarili) {

      if (basariliOkuma < 10) {
        Serial.print("Kalibrasyon GyZ[");
        Serial.print(basariliOkuma);
        Serial.print("]: ");
        Serial.println(GyZ);
      }

      toplamGX += GyX;
      toplamGY += GyY;
      toplamGZ += GyZ;

      toplamPitch += atan2(
                       AcY,
                       sqrt((float)AcX * AcX + (float)AcZ * AcZ))
                     * 180.0 / PI;


      toplamRoll += atan2(
                      AcX,
                      sqrt((float)AcY * AcY + (float)AcZ * AcZ))
                    * 180.0 / PI;

      basariliOkuma++;
    }

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
    gyroX_hata = (float)toplamGX / basariliOkuma;
    gyroY_hata = (float)toplamGY / basariliOkuma;
    gyroZ_hata = (float)toplamGZ / basariliOkuma;
    Pitch_offset = toplamPitch / basariliOkuma;
    Roll_offset = toplamRoll / basariliOkuma;
  }
  Serial.print("gyroX_hata: ");
  Serial.println(gyroX_hata);

  Serial.print("gyroY_hata: ");
  Serial.println(gyroY_hata);

  Serial.print("Pitch_offset: ");
  Serial.println(Pitch_offset);

  Serial.print("Roll_offset: ");
  Serial.println(Roll_offset);

  Serial.print("basariliOkuma: ");
  Serial.println(basariliOkuma);

  Serial.print("toplamGZ: ");
  Serial.println(toplamGZ);

  Serial.print("gyroZ_hata: ");
  Serial.println(gyroZ_hata);
}