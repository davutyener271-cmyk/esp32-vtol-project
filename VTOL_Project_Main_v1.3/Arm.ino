void update_Arm() {

  // Bağlantı kaybında kanal değerlerini kullanıcı komutu sayma.
  if (rcFailsafeActive) {
    armSwitchWasOff = false;
    return;
  }

  // Geçerli bağlantıda SwA kapalıysa DISARM ol.
  if (iBusChannels[4] < 1200) {
    isArmed = false;
    armSwitchWasOff = true;
    return;
  }

  // Zaten ARM durumundaysak yeniden ARM kontrolü yapma.
  if (isArmed) {
    return;
  }

  // Anahtar önce kapalı görülmüş, ardından açılmışsa:
  if (iBusChannels[4] > 1800 && armSwitchWasOff) {
    armSwitchWasOff = false;

    // Gaz düşükse ARM izni ver.
    if (iBusChannels[2] <= 1050) {
      isArmed = true;
    }
  }
}