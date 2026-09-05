void read_IBUS() {

  static uint8_t packet[32];
  static uint8_t packetIndex = 0;

  while (iBusSerial.available() > 0) {

    uint8_t incomingByte =
      iBusSerial.read();


    // iBUS kanal paketinin ilk baytı 0x20 olmalıdır
    if (packetIndex == 0) {

      if (incomingByte != 0x20) {
        continue;
      }
    }


    // İkinci bayt kanal verisi paketi için 0x40 olmalıdır
    if (packetIndex == 1) {

      if (incomingByte != 0x40) {

        packetIndex = 0;

        // Gelen bayt yeni paketin başlangıcı olabilir
        if (incomingByte == 0x20) {

          packet[0] = incomingByte;
          packetIndex = 1;
        }

        continue;
      }
    }


    packet[packetIndex] =
      incomingByte;

    packetIndex++;


    // Tam iBUS paketi 32 bayttır
    if (packetIndex == 32) {

      uint16_t calculatedChecksum =
        0xFFFF;

      // İlk 30 bayttan checksum hesaplanır
      for (int i = 0; i < 30; i++) {

        calculatedChecksum -=
          packet[i];
      }


      // Paketin son iki baytındaki checksum
      uint16_t receivedChecksum =
        (uint16_t)packet[30] |
        ((uint16_t)packet[31] << 8);


      if (calculatedChecksum ==
          receivedChecksum) {

        // 14 kanalın her biri iki bayttır
        for (int channel = 0;
             channel < 14;
             channel++) {

          int bytePosition =
            2 + channel * 2;

          iBusChannels[channel] =
            (uint16_t)packet[bytePosition] |
            ((uint16_t)
             packet[bytePosition + 1] << 8);
        }


        lastIBusFrameTime =
          millis();

        iBusConnected = true;
      }

      packetIndex = 0;
    }
  }


  // 100 ms boyunca geçerli paket gelmezse
  // kumanda bağlantısını kopmuş kabul et
  if (lastIBusFrameTime == 0 ||
      millis() - lastIBusFrameTime > 100) {

    iBusConnected = false;
  }
}

void update_RC_Failsafe() {
  rcFailsafeActive =
    !iBusConnected || iBusChannels[5] < 1800;
}