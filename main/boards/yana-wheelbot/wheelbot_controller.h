#ifndef WHEELBOT_CONTROLLER_H
#define WHEELBOT_CONTROLLER_H

#include <atomic>
#include <memory>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "motor_driver.h"
#include "motor_settings.h"
#include "servo_motor_driver.h"

// Owns the active MotorDriver and registers every movement / motor-config /
// servo-calibration MCP tool, following main/boards/otto-robot/otto_controller.cc's
// pattern: a dedicated FreeRTOS action task + queue so MCP callbacks return
// immediately instead of blocking on motor timing.
class WheelbotController {
public:
    WheelbotController(gpio_num_t default_in1, gpio_num_t default_in2, gpio_num_t default_in3,
                       gpio_num_t default_in4);
    ~WheelbotController();

    // Called by CliffSensorController (or anything else) to force an
    // immediate stop, interrupting any in-flight timed move.
    void EmergencyStop();

private:
    enum ActionType {
        kActionForward = 1,
        kActionBackward = 2,
        kActionTurnLeft = 3,
        kActionTurnRight = 4,
    };

    struct ActionParams {
        int action_type;
        int speed;
        int duration_ms;
    };

    void BuildMotorDriver();
    void RegisterMcpTools();
    void QueueMove(ActionType action, int speed, int duration_ms);

    static void ActionTask(void* arg);

    MotorSettingsData settings_;
    std::unique_ptr<MotorDriver> motor_driver_;
    ServoMotorDriver* servo_driver_ = nullptr;  // non-owning alias, only valid when type==kServo

    QueueHandle_t action_queue_ = nullptr;
    TaskHandle_t action_task_handle_ = nullptr;
    std::atomic<bool> stop_requested_{false};
    std::atomic<bool> is_moving_{false};
};

#endif  // WHEELBOT_CONTROLLER_H
