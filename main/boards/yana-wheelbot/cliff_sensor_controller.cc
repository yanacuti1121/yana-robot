#include "cliff_sensor_controller.h"

#include <esp_log.h>
#include <cJSON.h>

#include "mcp_server.h"
#include "settings.h"

static const char* TAG = "CliffSensorController";
static constexpr const char* kNamespace = "cliff_sensor";
static constexpr int kPollIntervalMs = 200;

CliffSensorController::CliffSensorController(TofSensor* sensor,
                                             WheelbotController* wheelbot_controller)
    : sensor_(sensor), wheelbot_controller_(wheelbot_controller) {
    Settings settings(kNamespace, true);
    enabled_ = settings.GetBool("enabled", true);
    threshold_mm_ = settings.GetInt("threshold_mm", 50);

    xTaskCreate(PollTask, "CliffSensorPoll", 3072, this, tskIDLE_PRIORITY + 1, &poll_task_handle_);

    RegisterMcpTools();
}

CliffSensorController::~CliffSensorController() {
    if (poll_task_handle_ != nullptr) {
        vTaskDelete(poll_task_handle_);
    }
}

void CliffSensorController::PollTask(void* arg) {
    auto* controller = static_cast<CliffSensorController*>(arg);

    while (true) {
        if (controller->enabled_) {
            int mm = controller->sensor_->ReadDistanceMm();
            if (mm >= 0 && mm < controller->threshold_mm_) {
                ESP_LOGW(TAG, "Cliff detected at %d mm (threshold %d mm), emergency stop", mm,
                         controller->threshold_mm_.load());
                controller->wheelbot_controller_->EmergencyStop();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(kPollIntervalMs));
    }
}

void CliffSensorController::RegisterMcpTools() {
    auto& mcp = McpServer::GetInstance();

    mcp.AddTool("self.cliff_sensor.set_enabled", "Enable or disable the anti-fall ToF sensor.",
                PropertyList({Property("enabled", kPropertyTypeBoolean)}),
                [this](const PropertyList& p) -> ReturnValue {
                    bool enabled = p["enabled"].value<bool>();
                    enabled_ = enabled;
                    Settings settings(kNamespace, true);
                    settings.SetBool("enabled", enabled);
                    return true;
                });

    mcp.AddTool("self.cliff_sensor.set_threshold",
                "Set the safe-distance threshold (mm) below which the robot emergency-stops.",
                PropertyList({Property("threshold_mm", kPropertyTypeInteger, 50, 5, 500)}),
                [this](const PropertyList& p) -> ReturnValue {
                    int threshold = p["threshold_mm"].value<int>();
                    threshold_mm_ = threshold;
                    Settings settings(kNamespace, true);
                    settings.SetInt("threshold_mm", threshold);
                    return true;
                });

    mcp.AddTool("self.cliff_sensor.get_config", "Get the current anti-fall sensor configuration.",
                PropertyList(), [this](const PropertyList&) -> ReturnValue {
                    cJSON* json = cJSON_CreateObject();
                    cJSON_AddBoolToObject(json, "enabled", enabled_);
                    cJSON_AddNumberToObject(json, "threshold_mm", threshold_mm_);
                    return json;
                });

    mcp.AddTool("self.cliff_sensor.test_now",
                "Trigger a single distance reading right now and return the value in mm.",
                PropertyList(),
                [this](const PropertyList&) -> ReturnValue { return sensor_->ReadDistanceMm(); });
}
