void initialize_Servos() {

  leftTailServo.setPeriodHertz(50);

  leftTailServo.attach(
    LEFT_TAIL_SERVO_PIN,
    1000,
    2000
  );

  if (!leftTailServo.attached()) {
    Serial.println("D13 servo PWM baslatilamadi.");
    return;
  }

  leftTailServo.writeMicroseconds(1500);
  Serial.println("D13: 1500 us");
  delay(2000);

  leftTailServo.writeMicroseconds(1600);
  Serial.println("D13: 1600 us");
  delay(2000);

  leftTailServo.writeMicroseconds(1400);
  Serial.println("D13: 1400 us");
  delay(2000);

  leftTailServo.writeMicroseconds(1500);
  Serial.println("D13 testi tamamlandi.");
}