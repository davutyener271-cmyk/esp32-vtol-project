float normalize_RC_Axis(uint16_t channelValue) {

  float command =
    ((float)channelValue - 1500.0f) / 500.0f;

  command = constrain(command, -1.0f, 1.0f);

  const float deadband = 0.04f;

  if (command > deadband) {
    return (command - deadband) / (1.0f - deadband);
  }

  if (command < -deadband) {
    return (command + deadband) / (1.0f - deadband);
  }

  return 0.0f;
}


void calculate_RC_Commands() {

  rollCommand = normalize_RC_Axis(iBusChannels[0]);
  pitchCommand = normalize_RC_Axis(iBusChannels[1]);
  yawCommand = normalize_RC_Axis(iBusChannels[3]);

  throttleCommand =
    ((float)iBusChannels[2] - 1000.0f) / 1000.0f;

  throttleCommand =
    constrain(throttleCommand, 0.0f, 1.0f);
}