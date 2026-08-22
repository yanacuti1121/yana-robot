#ifndef CLIFF_SENSOR_CONTROLLER_H
#define CLIFF_SENSOR_CONTROLLER_H

#include <atomic>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "tof_sensor.h"
#include "wheelbot_controller.h"

// Polls the ToF sensor (VL53L0X or VL6180X, selected at build time -- see
// yana_wheelbot_board.cc) for anti-fall/cliff detection while enabled, and
// forces an emergency stop via WheelbotController when the reading drops
// below the configured safe-distance threshold. Settings namespace:
// "cliff_sensor" (keys: "enabled" bool, "threshold_mm" int).
class CliffSensorController {
public:
    CliffSensorController(TofSensor* sensor, WheelbotController* wheelbot_controller);
    ~CliffSensorController();

private:
    void RegisterMcpTools();
    static void PollTask(void* arg);

    TofSensor* sensor_;
    WheelbotController* wheelbot_controller_;
    std::atomic<bool> enabled_{true};
    std::atomic<int> threshold_mm_{50};
    TaskHandle_t poll_task_handle_ = nullptr;
};

#endif  // CLIFF_SENSOR_CONTROLLER_H
