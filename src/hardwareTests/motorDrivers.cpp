// Author: Robert Polk
// Copyright (c) 2024 BLINK. All rights reserved.
// Last Modified: 11/25/2024

#include <Arduino.h>
#include <array>

struct MotorDriver {
    uint8_t pwmPin; // PWM signal pin connected to IN1
    uint8_t directionPin;   // Direction signal pin connected to IN2
};

// Configuration variables - set these to match the hardware setup
constexpr uint8_t FIRST_DRIVER_PWM_PIN = 0;
constexpr uint8_t FIRST_DRIVER_DIRECTION_PIN = 0;
constexpr uint8_t SECOND_DRIVER_PWM_PIN = 0;
constexpr uint8_t SECOND_DRIVER_DIRECTION_PIN = 0;
constexpr uint8_t THIRD_DRIVER_PWM_PIN = 0;
constexpr uint8_t THIRD_DRIVER_DIRECTION_PIN = 0;
constexpr uint32_t PWM_FREQUENCY = 20000;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t BAUD_RATE = 115200;

// Program Variables
constexpr std::array<std::array<uint8_t, 2>, 3> motorPins = {FIRST_DRIVER_DIRECTION_PIN,
                                                             FIRST_DRIVER_PWM_PIN,
                                                             SECOND_DRIVER_DIRECTION_PIN,
                                                             SECOND_DRIVER_PWM_PIN,
                                                             THIRD_DRIVER_DIRECTION_PIN,
                                                             THIRD_DRIVER_PWM_PIN};
std::array<MotorDriver, 3> drivers;
std::array<int16_t, 3> motorSpeeds = {0, 0, 0};

void setMotorSpeeds(const std::array<int16_t, 3> &speeds) {
    uint16_t maxDutyCycle = (1 << PWM_RESOLUTION) - 1;

    for (size_t i(0); i < speeds.size(); ++i) {
        // Determine dutyCycle and direction
        uint16_t dutyCycle = abs(constrain(speeds[i], -maxDutyCycle, maxDutyCycle));
        uint8_t direction = speeds[i] >= 0 ? LOW : HIGH;

        // Set the pins
        digitalWrite(drivers[i].directionPin, direction);
        ledcWrite(i, dutyCycle);
    }
}

int16_t max() {
    return (1 << PWM_RESOLUTION) - 1;
}

int16_t min() {
    return -1 * ((1 << PWM_RESOLUTION) - 1);
}

void setup() {
    Serial.begin(BAUD_RATE);

    for (size_t i(0); i < drivers.size(); ++i) {
        // Set direction pin values
        drivers[i].directionPin = motorPins[i][0];
        pinMode(drivers[i].directionPin, OUTPUT);
        digitalWrite(drivers[i].directionPin, LOW);

        // Set pwm pin
        drivers[i].pwmPin = motorPins[i][1];
        ledcSetup(i, PWM_FREQUENCY, PWM_RESOLUTION);
        ledcAttachPin(drivers[i].pwmPin, i);
        ledcWrite(i, 0);
    }
}

void loop() {
    // make things move forward backward and test pwm variation

    // Move forward at full
    motorSpeeds = {max(), 0, 0};
    setMotorSpeeds(motorSpeeds);
    delay(1000);

    motorSpeeds = {0, 0, 0};
    setMotorSpeeds(motorSpeeds);
    delay(1000);

    motorSpeeds = {min(), 0, 0};
    setMotorSpeeds(motorSpeeds);
    delay(1000);

    motorSpeeds = {0, 0, 0};
    setMotorSpeeds(motorSpeeds);
    delay(1000);
}