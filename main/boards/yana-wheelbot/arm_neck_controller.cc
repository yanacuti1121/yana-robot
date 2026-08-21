#include "arm_neck_controller.h"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "mcp_server.h"

static const char* TAG = "ArmNeckController";

namespace {
constexpr int kArmHome = 90;
constexpr int kNeckHome = 90;
constexpr int kNeckLeft = 30;
constexpr int kNeckRight = 150;

// Short-lived task for the canned "wave" sequence, so the MCP callback that
// triggers it returns immediately (same reasoning as
// main/boards/otto-robot/otto_controller.cc's dedicated action task, just
// scoped to a single one-shot gesture instead of a persistent queue).
struct WaveTaskArgs {
    Oscillator* arm;
};

void WaveTask(void* arg) {
    auto* args = static_cast<WaveTaskArgs*>(arg);
    for (int i = 0; i < 3; i++) {
        args->arm->SetPosition(60);
        vTaskDelay(pdMS_TO_TICKS(250));
        args->arm->SetPosition(120);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
    args->arm->SetPosition(kArmHome);
    delete args;
    vTaskDelete(nullptr);
}
}  // namespace

ArmNeckController::ArmNeckController(gpio_num_t arm_pin, gpio_num_t neck_pin)
    : arm_pin_(arm_pin), neck_pin_(neck_pin) {
    arm_.Attach(arm_pin_);
    arm_attached_ = true;
    arm_.SetPosition(kArmHome);
    neck_.Attach(neck_pin_);
    neck_attached_ = true;
    neck_.SetPosition(kNeckHome);

    RegisterMcpTools();
}

void ArmNeckController::SetArmAngle(int angle) { arm_.SetPosition(angle); }

void ArmNeckController::SetNeckAngle(int angle) { neck_.SetPosition(angle); }

void ArmNeckController::Wave() {
    auto* args = new WaveTaskArgs{&arm_};
    xTaskCreate(WaveTask, "ArmWave", 2048, args, tskIDLE_PRIORITY + 1, nullptr);
}

void ArmNeckController::TurnNeck(const std::string& direction) {
    if (direction == "left") {
        neck_.SetPosition(kNeckLeft);
    } else if (direction == "right") {
        neck_.SetPosition(kNeckRight);
    } else {
        neck_.SetPosition(kNeckHome);
    }
}

void ArmNeckController::ReleaseArm() {
    arm_.Detach();
    arm_attached_ = false;
}

void ArmNeckController::ReleaseNeck() {
    neck_.Detach();
    neck_attached_ = false;
}

void ArmNeckController::RegisterMcpTools() {
    auto& mcp = McpServer::GetInstance();

    mcp.AddTool("self.arm.set_angle", "Set the arm servo angle (0-180 degrees).",
                PropertyList({Property("angle", kPropertyTypeInteger, 90, 0, 180)}),
                [this](const PropertyList& p) -> ReturnValue {
                    if (!arm_attached_) {
                        arm_.Attach(arm_pin_);
                        arm_attached_ = true;
                    }
                    SetArmAngle(p["angle"].value<int>());
                    return true;
                });

    mcp.AddTool("self.neck.set_angle", "Set the neck servo angle (0-180 degrees).",
                PropertyList({Property("angle", kPropertyTypeInteger, 90, 0, 180)}),
                [this](const PropertyList& p) -> ReturnValue {
                    if (!neck_attached_) {
                        neck_.Attach(neck_pin_);
                        neck_attached_ = true;
                    }
                    SetNeckAngle(p["angle"].value<int>());
                    return true;
                });

    mcp.AddTool("self.arm.wave", "Wave the arm back and forth a few times.", PropertyList(),
                [this](const PropertyList&) -> ReturnValue {
                    if (!arm_attached_) {
                        arm_.Attach(arm_pin_);
                        arm_attached_ = true;
                    }
                    Wave();
                    return true;
                });

    mcp.AddTool("self.neck.turn", "Turn the neck: left, right, or center.",
                PropertyList({Property("direction", kPropertyTypeString)}),
                [this](const PropertyList& p) -> ReturnValue {
                    if (!neck_attached_) {
                        neck_.Attach(neck_pin_);
                        neck_attached_ = true;
                    }
                    TurnNeck(p["direction"].value<std::string>());
                    return true;
                });

    mcp.AddTool("self.arm.release", "Stop driving the arm servo so it goes limp.", PropertyList(),
                [this](const PropertyList&) -> ReturnValue {
                    ReleaseArm();
                    return true;
                });

    mcp.AddTool("self.neck.release", "Stop driving the neck servo so it goes limp.", PropertyList(),
                [this](const PropertyList&) -> ReturnValue {
                    ReleaseNeck();
                    return true;
                });
}
