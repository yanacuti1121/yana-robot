#include "wheelbot_controller.h"

#include <esp_log.h>
#include <cJSON.h>

#include "application.h"
#include "config.h"
#include "l298n_motor_driver.h"
#include "mcp_server.h"

static const char* TAG = "WheelbotController";

WheelbotController::WheelbotController(gpio_num_t default_in1, gpio_num_t default_in2,
                                       gpio_num_t default_in3, gpio_num_t default_in4) {
    settings_ = LoadMotorSettings(default_in1, default_in2, default_in3, default_in4);
    BuildMotorDriver();

    action_queue_ = xQueueCreate(4, sizeof(ActionParams));
    xTaskCreate(ActionTask, "WheelbotAction", 3072, this, configMAX_PRIORITIES - 2,
                &action_task_handle_);

    RegisterMcpTools();
}

WheelbotController::~WheelbotController() {
    if (action_task_handle_ != nullptr) {
        vTaskDelete(action_task_handle_);
    }
    if (action_queue_ != nullptr) {
        vQueueDelete(action_queue_);
    }
}

void WheelbotController::BuildMotorDriver() {
    if (settings_.type == MotorType::kServo) {
        auto* driver = new ServoMotorDriver(
            WHEELBOT_HARDWARE_CONFIG.servo_left_pin, WHEELBOT_HARDWARE_CONFIG.servo_right_pin,
            settings_.stop_pulse_us, settings_.reverse_left, settings_.reverse_right);
        servo_driver_ = driver;
        motor_driver_.reset(driver);
        ESP_LOGI(TAG, "Motor backend: continuous-rotation servo pair");
    } else {
        motor_driver_.reset(
            new L298nMotorDriver(settings_.in1, settings_.in2, settings_.in3, settings_.in4));
        servo_driver_ = nullptr;
        ESP_LOGI(TAG, "Motor backend: L298N (in1=%d in2=%d in3=%d in4=%d)", settings_.in1,
                 settings_.in2, settings_.in3, settings_.in4);
    }
}

void WheelbotController::QueueMove(ActionType action, int speed, int duration_ms) {
    stop_requested_ = false;
    ActionParams params{static_cast<int>(action), speed, duration_ms};
    // Drop any queued-but-not-yet-started move; the newest command wins.
    xQueueReset(action_queue_);
    xQueueSend(action_queue_, &params, 0);
}

void WheelbotController::EmergencyStop() {
    stop_requested_ = true;
    xQueueReset(action_queue_);
    motor_driver_->Stop();
    is_moving_ = false;
}

void WheelbotController::ActionTask(void* arg) {
    auto* controller = static_cast<WheelbotController*>(arg);
    ActionParams params;

    while (true) {
        if (xQueueReceive(controller->action_queue_, &params, pdMS_TO_TICKS(1000)) != pdTRUE) {
            continue;
        }

        controller->is_moving_ = true;
        int left = 0, right = 0;
        switch (params.action_type) {
            case kActionForward:
                left = params.speed;
                right = params.speed;
                break;
            case kActionBackward:
                left = -params.speed;
                right = -params.speed;
                break;
            case kActionTurnLeft:
                left = -params.speed;
                right = params.speed;
                break;
            case kActionTurnRight:
                left = params.speed;
                right = -params.speed;
                break;
        }
        controller->motor_driver_->Drive(left, right);

        // Chunked wait so EmergencyStop() (setting stop_requested_) can
        // interrupt a long-running timed move instead of a single blocking delay.
        constexpr int kTickMs = 50;
        int waited = 0;
        while (waited < params.duration_ms && !controller->stop_requested_) {
            vTaskDelay(pdMS_TO_TICKS(kTickMs));
            waited += kTickMs;
        }

        controller->motor_driver_->Stop();
        controller->is_moving_ = false;
    }
}

void WheelbotController::RegisterMcpTools() {
    auto& mcp = McpServer::GetInstance();

    auto move_properties = PropertyList({
        Property("duration_ms", kPropertyTypeInteger, 2000, 0, 30000),
        Property("speed", kPropertyTypeInteger, 80, 0, 100),
    });

    mcp.AddTool("self.wheelbot.move_forward", "Drive the robot forward for a duration.",
                move_properties, [this](const PropertyList& p) -> ReturnValue {
                    QueueMove(kActionForward, p["speed"].value<int>(),
                              p["duration_ms"].value<int>());
                    return true;
                });
    mcp.AddTool("self.wheelbot.move_backward", "Drive the robot backward for a duration.",
                move_properties, [this](const PropertyList& p) -> ReturnValue {
                    QueueMove(kActionBackward, p["speed"].value<int>(),
                              p["duration_ms"].value<int>());
                    return true;
                });
    mcp.AddTool("self.wheelbot.turn_left", "Turn the robot left in place for a duration.",
                move_properties, [this](const PropertyList& p) -> ReturnValue {
                    QueueMove(kActionTurnLeft, p["speed"].value<int>(),
                              p["duration_ms"].value<int>());
                    return true;
                });
    mcp.AddTool("self.wheelbot.turn_right", "Turn the robot right in place for a duration.",
                move_properties, [this](const PropertyList& p) -> ReturnValue {
                    QueueMove(kActionTurnRight, p["speed"].value<int>(),
                              p["duration_ms"].value<int>());
                    return true;
                });
    mcp.AddTool("self.wheelbot.stop", "Immediately stop all motor movement.", PropertyList(),
                [this](const PropertyList&) -> ReturnValue {
                    EmergencyStop();
                    return true;
                });

    mcp.AddTool("self.wheelbot.set_motor_type",
                "Switch the motor backend between the continuous-rotation servo pair and the L298N "
                "DC motor driver. Persists and reboots the device to apply.",
                PropertyList({Property("type", kPropertyTypeString)}),
                [this](const PropertyList& p) -> ReturnValue {
                    auto type_str = p["type"].value<std::string>();
                    MotorType type = (type_str == "l298n") ? MotorType::kL298n : MotorType::kServo;
                    SaveMotorTypeAndPins(type, settings_.in1, settings_.in2, settings_.in3,
                                         settings_.in4);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    Application::GetInstance().Reboot();
                    return true;
                });

    mcp.AddTool("self.wheelbot.set_motor_pins",
                "Remap the 4 L298N control GPIOs (in1-in4). Persists and reboots to apply.",
                PropertyList({
                    Property("in1", kPropertyTypeInteger, 0, 48),
                    Property("in2", kPropertyTypeInteger, 0, 48),
                    Property("in3", kPropertyTypeInteger, 0, 48),
                    Property("in4", kPropertyTypeInteger, 0, 48),
                }),
                [this](const PropertyList& p) -> ReturnValue {
                    auto in1 = static_cast<gpio_num_t>(p["in1"].value<int>());
                    auto in2 = static_cast<gpio_num_t>(p["in2"].value<int>());
                    auto in3 = static_cast<gpio_num_t>(p["in3"].value<int>());
                    auto in4 = static_cast<gpio_num_t>(p["in4"].value<int>());
                    SaveMotorTypeAndPins(settings_.type, in1, in2, in3, in4);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    Application::GetInstance().Reboot();
                    return true;
                });

    mcp.AddTool("self.wheelbot.get_motor_config", "Get the current motor backend and pin config.",
                PropertyList(), [this](const PropertyList&) -> ReturnValue {
                    cJSON* json = cJSON_CreateObject();
                    cJSON_AddStringToObject(
                        json, "type", settings_.type == MotorType::kL298n ? "l298n" : "servo");
                    cJSON_AddNumberToObject(json, "in1", settings_.in1);
                    cJSON_AddNumberToObject(json, "in2", settings_.in2);
                    cJSON_AddNumberToObject(json, "in3", settings_.in3);
                    cJSON_AddNumberToObject(json, "in4", settings_.in4);
                    cJSON_AddNumberToObject(json, "stop_pulse_us", settings_.stop_pulse_us);
                    cJSON_AddBoolToObject(json, "reverse_left", settings_.reverse_left);
                    cJSON_AddBoolToObject(json, "reverse_right", settings_.reverse_right);
                    return json;
                });

    mcp.AddTool(
        "self.wheelbot.set_servo_stop_pulse",
        "Calibrate the continuous-rotation servo stop/neutral pulse width. Applies live, no "
        "reboot needed. Only affects the servo motor backend.",
        PropertyList({Property("microseconds", kPropertyTypeInteger, 1500, 1000, 2000)}),
        [this](const PropertyList& p) -> ReturnValue {
            if (servo_driver_ == nullptr) {
                return std::string("Current motor backend is not the servo pair.");
            }
            int us = p["microseconds"].value<int>();
            servo_driver_->SetStopPulseUs(us);
            settings_.stop_pulse_us = us;
            SaveServoStopPulseUs(us);
            return true;
        });

    mcp.AddTool(
        "self.wheelbot.set_servo_reverse",
        "Reverse one side's continuous-rotation servo direction. Applies live. Only affects "
        "the servo motor backend.",
        PropertyList({
            Property("side", kPropertyTypeString),
            Property("reversed", kPropertyTypeBoolean),
        }),
        [this](const PropertyList& p) -> ReturnValue {
            if (servo_driver_ == nullptr) {
                return std::string("Current motor backend is not the servo pair.");
            }
            bool left = p["side"].value<std::string>() == "left";
            bool reversed = p["reversed"].value<bool>();
            servo_driver_->SetReverse(left, reversed);
            if (left) {
                settings_.reverse_left = reversed;
            } else {
                settings_.reverse_right = reversed;
            }
            SaveServoReverse(left, reversed);
            return true;
        });
}
