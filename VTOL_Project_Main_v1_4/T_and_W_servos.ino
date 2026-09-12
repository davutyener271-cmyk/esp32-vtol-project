void initialize_Servos() {

  leftTailServo.setPeriodHertz(50);
  rightTailServo.setPeriodHertz(50);
  leftAileronServo.setPeriodHertz(50);
  rightAileronServo.setPeriodHertz(50);

  leftTailServo.attach(
    LEFT_TAIL_SERVO_PIN,
    1000,
    2000
  );

  rightTailServo.attach(
    RIGHT_TAIL_SERVO_PIN,
    900,
    2100
  );

  leftAileronServo.attach(
    LEFT_AILERON_SERVO_PIN,
    1000,
    2000
  );

  rightAileronServo.attach(
    RIGHT_AILERON_SERVO_PIN,
    1000,
    2000
  );


  if (!leftTailServo.attached()) {
    Serial.println(
      "Sol kuyruk servosu baslatilamadi."
    );
  }

  if (!rightTailServo.attached()) {
    Serial.println(
      "Sag kuyruk servosu baslatilamadi."
    );
  }

  if (!leftAileronServo.attached()) {
    Serial.println(
      "Sol kanat servosu baslatilamadi."
    );
  }

  if (!rightAileronServo.attached()) {
    Serial.println(
      "Sag kanat servosu baslatilamadi."
    );
  }


  leftTailServo.writeMicroseconds(1500);
  rightTailServo.writeMicroseconds(1400);

  leftAileronServo.writeMicroseconds(1500);
  rightAileronServo.writeMicroseconds(1500);

  Serial.println("Tum servolar merkeze alindi.");
}


void update_All_Servos() {

  // Kuyruk servo merkezleri
  const int leftServoCenter = 1500;
  const int rightServoCenter = 1400;

  // Aileron servo merkezleri
  const int leftWingServoCenter = 1500;
  const int rightWingServoCenter = 1500;

  // Kuyruk hareket miktarları
  const int leftServoTravel = 480;
  const int rightServoTravel = 500;

  // Aileron hareket miktarları
  const int leftWingServoTravel = 500;
  const int rightWingServoTravel = 500;

  // Kuyruk yüzeylerinin yaw desteği
  const int yawServoTravel = 150;


  int pitchLeftContribution =
    (int)(pitchCommand * leftServoTravel);

  int pitchRightContribution =
    (int)(pitchCommand * rightServoTravel);

  int rollLeftContribution =
    (int)(rollCommand * leftWingServoTravel);

  int rollRightContribution =
    (int)(rollCommand * rightWingServoTravel);

  int yawContribution =
    (int)(yawCommand * yawServoTravel);


  // Kuyruk: pitch için ters, yaw desteği için aynı yön
  int leftServoPulse =
    leftServoCenter
    + pitchLeftContribution
    + yawContribution;

  int rightServoPulse =
    rightServoCenter
    - pitchRightContribution
    + yawContribution;


  // Aileronlar roll için birbirinin tersine hareket eder
  int leftWingServoPulse =
    leftWingServoCenter
    - rollLeftContribution;

  int rightWingServoPulse =
    rightWingServoCenter
    - rollRightContribution;


  leftServoPulse = constrain(
    leftServoPulse,
    leftServoCenter - leftServoTravel,
    leftServoCenter + leftServoTravel
  );

  rightServoPulse = constrain(
    rightServoPulse,
    rightServoCenter - rightServoTravel,
    rightServoCenter + rightServoTravel
  );

  leftWingServoPulse = constrain(
    leftWingServoPulse,
    leftWingServoCenter - leftWingServoTravel,
    leftWingServoCenter + leftWingServoTravel
  );

  rightWingServoPulse = constrain(
    rightWingServoPulse,
    rightWingServoCenter - rightWingServoTravel,
    rightWingServoCenter + rightWingServoTravel
  );


  leftTailServo.writeMicroseconds(
    leftServoPulse
  );

  rightTailServo.writeMicroseconds(
    rightServoPulse
  );

  leftAileronServo.writeMicroseconds(
    leftWingServoPulse
  );

  rightAileronServo.writeMicroseconds(
    rightWingServoPulse
  );
}