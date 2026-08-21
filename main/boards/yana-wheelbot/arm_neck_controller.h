#ifndef ARM_NECK_CONTROLLER_H
#define ARM_NECK_CONTROLLER_H

#include <string>

#include <driver/gpio.h>

#include "oscillator.h"

// Owns the arm and neck servos (plain 0-180 degree oscillator.h/.cc servos,
// copied from main/boards/otto-robot since that code is board-agnostic ledc
// PWM, not biped-specific) and registers their MCP tools.
class ArmNeckController {
public:
    ArmNeckController(gpio_num_t arm_pin, gpio_num_t neck_pin);

    void SetArmAngle(int angle);
    void SetNeckAngle(int angle);
    void Wave();
    void TurnNeck(const std::string& direction);
    void ReleaseArm();
    void ReleaseNeck();

private:
    void RegisterMcpTools();

    Oscillator arm_;
    Oscillator neck_;
    gpio_num_t arm_pin_;
    gpio_num_t neck_pin_;
    // Oscillator::Attach() grabs a *new* ledc channel from a shared rotating
    // counter every time it's called, even if already attached — so these
    // flags exist to make sure Attach() is only called again after a real
    // Detach() (release), never unconditionally on every tool invocation.
    bool arm_attached_ = false;
    bool neck_attached_ = false;
};

#endif  // ARM_NECK_CONTROLLER_H
