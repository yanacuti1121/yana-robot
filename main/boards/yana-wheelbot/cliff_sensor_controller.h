#ifndef CLIFF_SENSOR_CONTROLLER_H
#define CLIFF_SENSOR_CONTROLLER_H

#include <atomic>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "vl53l0x.h"
#include "wheelbot_controller.h"

// Polls the VL53L0X ToF sensor for anti-fall/cliff detection while enabled,
// and forces an emergency stop via WheelbotController when the reading drops
// below the configured safe-distance threshold. Settings namespace:
// "cliff_sensor" (keys: "enabled" bool, "threshold_mm" int).
class CliffSensorController {
public:
    CliffSensorController(Vl53l0x* sensor, WheelbotController* wheelbot_controller);
    ~CliffSensorController();

private:
    void RegisterMcpTools();
    static void PollTask(void* arg);

    Vl53l0x* sensor_;
    WheelbotController* wheelbot_controller_;
    std::atomic<bool> enabled_{true};
    std::atomic<int> threshold_mm_{50};
    TaskHandle_t poll_task_handle_ = nullptr;
};

#endif  // CLIFF_SENSOR_CONTROLLER_H
