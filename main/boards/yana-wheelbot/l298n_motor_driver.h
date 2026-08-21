#ifndef L298N_MOTOR_DRIVER_H
#define L298N_MOTOR_DRIVER_H

#include <driver/gpio.h>
#include <driver/ledc.h>

#include "motor_driver.h"

// Drives two DC gear motors through an L298N H-bridge module.
//
// Pin convention assumed (verify against your actual L298N breakout before
// trusting this): one PWM/EN pin + one plain direction pin per side —
// in1 = left PWM/EN, in2 = left DIR, in3 = right PWM/EN, in4 = right DIR.
// Some L298N breakouts instead expose 4 direct H-bridge-leg pins with no
// separate EN pin; if that's your wiring, this class's pin semantics do not
// apply as-is and the mapping in the .cc file needs adjusting first.
class L298nMotorDriver : public MotorDriver {
public:
    L298nMotorDriver(gpio_num_t in1, gpio_num_t in2, gpio_num_t in3, gpio_num_t in4);
    ~L298nMotorDriver() override;

    void Drive(int left_speed, int right_speed) override;
    void Stop() override;

private:
    void ConfigurePwmPin(gpio_num_t pin, ledc_channel_t channel, ledc_timer_t timer);
    void WriteSide(gpio_num_t pwm_pin, ledc_channel_t pwm_channel, gpio_num_t dir_pin, int speed);

    gpio_num_t in1_, in2_, in3_, in4_;
};

#endif  // L298N_MOTOR_DRIVER_H
