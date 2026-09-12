bool tiltSwitchWasHover = false;

enum TiltState {
  TILT_HOVER,
  TILT_TRANSITION_FORWARD,
  TILT_FORWARD,
  TILT_TRANSITION_HOVER
};

TiltState tiltState = TILT_HOVER;

const float MAX_TILT_ANGLE = 15.0f;

const int RIGHT_TILT_MIN_PULSE = 700;
const int RIGHT_TILT_MAX_PULSE = 2000;
const int RIGHT_TILT_FORWARD_PULSE = 740;
const int RIGHT_TILT_HOVER_PULSE = 1790;

const int LEFT_TILT_MIN_PULSE = 700;
const int LEFT_TILT_MAX_PULSE = 2000;
const int LEFT_TILT_FORWARD_PULSE = 1920;
const int LEFT_TILT_HOVER_PULSE = 880;

// Yaw hareket miktarları
const int YAW_TILT_FORWARD_TRAVEL = 70;
const int YAW_TILT_BACKWARD_TRAVEL = 50;

const unsigned long TILT_UPDATE_INTERVAL = 20;
const float TILT_PROGRESS_STEP = 0.01f;

unsigned long lastTiltUpdateTime = 0;

float tiltProgress = 0.0f;
float tiltTargetProgress = 0.0f;

int rightTiltCurrentPulse = RIGHT_TILT_HOVER_PULSE;
int rightTiltTargetPulse = RIGHT_TILT_HOVER_PULSE;
int leftTiltCurrentPulse = LEFT_TILT_HOVER_PULSE;
int leftTiltTargetPulse = LEFT_TILT_HOVER_PULSE;


void set_Tilt_Target_Hover() {
  tiltTargetProgress = 0.0f;
  rightTiltTargetPulse = RIGHT_TILT_HOVER_PULSE;
  leftTiltTargetPulse = LEFT_TILT_HOVER_PULSE;

  if (tiltProgress <= 0.0f) {
    tiltState = TILT_HOVER;
  } else {
    tiltState = TILT_TRANSITION_HOVER;
  }
}


void set_Tilt_Target_Forward() {
  tiltTargetProgress = 1.0f;
  rightTiltTargetPulse = RIGHT_TILT_FORWARD_PULSE;
  leftTiltTargetPulse = LEFT_TILT_FORWARD_PULSE;

  if (tiltProgress >= 1.0f) {
    tiltState = TILT_FORWARD;
  } else {
    tiltState = TILT_TRANSITION_FORWARD;
  }
}


void initialize_Tilt_Servos() {
  rightTiltServo.setPeriodHertz(50);
  leftTiltServo.setPeriodHertz(50);

  rightTiltServo.attach(RIGHT_TILT_SERVO_PIN, RIGHT_TILT_MIN_PULSE, RIGHT_TILT_MAX_PULSE);
  leftTiltServo.attach(LEFT_TILT_SERVO_PIN, LEFT_TILT_MIN_PULSE, LEFT_TILT_MAX_PULSE);

  if (!rightTiltServo.attached()) {
    Serial.println("Sag tilt servosu baslatilamadi.");
    return;
  }

  if (!leftTiltServo.attached()) {
    Serial.println("Sol tilt servosu baslatilamadi.");
    return;
  }

  tiltProgress = 0.0f;
  tiltTargetProgress = 0.0f;
  tiltState = TILT_HOVER;
  tiltSwitchWasHover = false;

  rightTiltCurrentPulse = RIGHT_TILT_HOVER_PULSE;
  rightTiltTargetPulse = RIGHT_TILT_HOVER_PULSE;
  leftTiltCurrentPulse = LEFT_TILT_HOVER_PULSE;
  leftTiltTargetPulse = LEFT_TILT_HOVER_PULSE;

  rightTiltServo.writeMicroseconds(rightTiltCurrentPulse);
  leftTiltServo.writeMicroseconds(leftTiltCurrentPulse);

  Serial.println("Tilt servolari hover konumuna alindi.");
}


void update_Tilt_Target() {
  bool tiltAngleSafe =
    fabs(Filtered_Pitch) < MAX_TILT_ANGLE &&
    fabs(Filtered_Roll) < MAX_TILT_ANGLE;

  if (rcFailsafeActive || !isArmed) {
    set_Tilt_Target_Hover();
    tiltSwitchWasHover = false;
    return;
  }

  if (iBusChannels[7] < 1200) {
    set_Tilt_Target_Hover();
    tiltSwitchWasHover = true;
    return;
  }

  if (iBusChannels[7] <= 1800) {
    set_Tilt_Target_Hover();
    tiltSwitchWasHover = false;
    return;
  }

  if (tiltState == TILT_FORWARD) {
    set_Tilt_Target_Forward();
    return;
  }

  if (tiltState == TILT_TRANSITION_FORWARD && !tiltAngleSafe) {
    set_Tilt_Target_Hover();
    tiltSwitchWasHover = false;
    return;
  }

  if (tiltSwitchWasHover && tiltAngleSafe) {
    set_Tilt_Target_Forward();
    return;
  }

  set_Tilt_Target_Hover();

  if (!tiltAngleSafe) {
    tiltSwitchWasHover = false;
  }
}


void update_Tilt_Servos(unsigned long currentTime) {
  if (!rightTiltServo.attached() || !leftTiltServo.attached()) {
    return;
  }

  if (currentTime - lastTiltUpdateTime < TILT_UPDATE_INTERVAL) {
    return;
  }

  lastTiltUpdateTime = currentTime;

  if (tiltProgress < tiltTargetProgress) {
    tiltProgress += TILT_PROGRESS_STEP;

    if (tiltProgress > tiltTargetProgress) {
      tiltProgress = tiltTargetProgress;
    }
  } else if (tiltProgress > tiltTargetProgress) {
    tiltProgress -= TILT_PROGRESS_STEP;

    if (tiltProgress < tiltTargetProgress) {
      tiltProgress = tiltTargetProgress;
    }
  }

  // Ortak geçiş oranını iki servonun kendi değerlerine dönüştür
  rightTiltCurrentPulse = (int)roundf(
    RIGHT_TILT_HOVER_PULSE +
    tiltProgress * (RIGHT_TILT_FORWARD_PULSE - RIGHT_TILT_HOVER_PULSE)
  );

  leftTiltCurrentPulse = (int)roundf(
    LEFT_TILT_HOVER_PULSE +
    tiltProgress * (LEFT_TILT_FORWARD_PULSE - LEFT_TILT_HOVER_PULSE)
  );

  // Şimdilik yalnızca hover modunda doğrudan kumanda yaw kontrolü
  int rightYawCorrection = 0;
  int leftYawCorrection = 0;

  if (tiltState == TILT_HOVER && isArmed && !rcFailsafeActive) {
    float limitedYawCommand = constrain(yawCommand, -1.0f, 1.0f);

    if (limitedYawCommand > 0.0f) {
      // Kumanda sağ: sol motor burna, sağ motor kuyruğa
      leftYawCorrection =
        (int)roundf(limitedYawCommand * YAW_TILT_FORWARD_TRAVEL);

      rightYawCorrection =
        (int)roundf(limitedYawCommand * YAW_TILT_BACKWARD_TRAVEL);
    } else if (limitedYawCommand < 0.0f) {
      // Kumanda sol: sağ motor burna, sol motor kuyruğa
      rightYawCorrection =
        (int)roundf(limitedYawCommand * YAW_TILT_FORWARD_TRAVEL);

      leftYawCorrection =
        (int)roundf(limitedYawCommand * YAW_TILT_BACKWARD_TRAVEL);
    }
  }

  rightTiltCurrentPulse += rightYawCorrection;
  leftTiltCurrentPulse += leftYawCorrection;

  rightTiltCurrentPulse = constrain(
    rightTiltCurrentPulse,
    RIGHT_TILT_MIN_PULSE,
    RIGHT_TILT_MAX_PULSE
  );

  leftTiltCurrentPulse = constrain(
    leftTiltCurrentPulse,
    LEFT_TILT_MIN_PULSE,
    LEFT_TILT_MAX_PULSE
  );

  rightTiltServo.writeMicroseconds(rightTiltCurrentPulse);
  leftTiltServo.writeMicroseconds(leftTiltCurrentPulse);

  if (tiltProgress >= 1.0f) {
    tiltProgress = 1.0f;
    tiltState = TILT_FORWARD;
  } else if (tiltProgress <= 0.0f) {
    tiltProgress = 0.0f;
    tiltState = TILT_HOVER;
  }
}