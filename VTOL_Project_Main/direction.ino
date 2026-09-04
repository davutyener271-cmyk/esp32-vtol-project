void calculate_Direction(float dt) {

  ac_Pitch = atan2(AcY, sqrt((float)AcX * AcX + (float)AcZ * AcZ)) * 180.0 / PI - Pitch_offset;
  ac_Roll = atan2(AcX, sqrt((float)AcY * AcY + (float)AcZ * AcZ)) * 180.0 / PI - Roll_offset;

  gyro_Pitch = (GyX -gyroX_hata) / 16.4;
  gyro_Roll = -(GyY -gyroY_hata) / 16.4;
  gyro_Yaw = (GyZ- gyroZ_hata) / 16.4;

  Filtered_Pitch = 0.95 * (Filtered_Pitch + gyro_Pitch * dt) + 0.05 * ac_Pitch;
  Filtered_Roll = 0.95 * (Filtered_Roll + gyro_Roll * dt) + 0.05 * ac_Roll;

  Serial.print("Pitch: ");
  Serial.print(Filtered_Pitch);
  Serial.print("  |  ");
  Serial.print("Roll: ");
  Serial.print(Filtered_Roll);
  Serial.print("  |  ");
  Serial.print("Gyro Z: ");
  Serial.println(gyro_Yaw);
}