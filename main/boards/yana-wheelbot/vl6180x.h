#ifndef VL6180X_H
#define VL6180X_H

#include <driver/i2c_master.h>

#include "tof_sensor.h"

// Minimal VL6180X time-of-flight distance sensor driver (alternate to
// VL53L0X, selected via Kconfig YANA_WHEELBOT_TOF_VL6180X). Shorter range
// (~200mm max) than VL53L0X (~2m), but a common substitute (e.g. the KST AI
// Robot's public wiring diagram lists VL6180X/TOF050C as its anti-fall
// sensor -- see this board's README "Credit" section).
//
// Does NOT inherit I2cDevice: VL6180X uses 16-bit register addresses,
// unlike VL53L0X's 8-bit ones that I2cDevice's WriteReg/ReadReg assume, so
// this class does its own minimal I2C read/write via the same underlying
// i2c_master_transmit/i2c_master_transmit_receive APIs I2cDevice itself
// uses internally (main/boards/common/i2c_device.cc).
//
// SCOPE NOTE (same honest-scope reasoning as vl53l0x.h): this driver only
// does presence detection (model ID register) and the basic single-shot
// ranging trigger/poll/read sequence. ST's datasheet (AN4545) requires a
// mandatory one-time "private register" initialization sequence (~30
// register writes, checked via a fresh-out-of-reset flag) before the
// sensor produces valid readings -- that sequence is NOT implemented here,
// since reproducing ~30 undocumented-by-design register values from memory
// without the actual datasheet in hand risks silently wrong writes to
// registers ST explicitly doesn't publish the meaning of. Without it, this
// sensor may not range correctly (or at all). Verify against a known
// distance on real hardware; if it doesn't work, implement ST's official
// init sequence from the real AN4545 document or a maintained driver
// (e.g. Adafruit_VL6180X, Pololu VL6180X) before trusting this for safety.
class Vl6180x : public TofSensor {
public:
    Vl6180x(i2c_master_bus_handle_t i2c_bus, uint8_t addr = 0x29);

    // Returns true if the model-ID register matches the known VL6180X value.
    bool Probe();

    int ReadDistanceMm() override;

private:
    void WriteReg8(uint16_t reg, uint8_t value);
    uint8_t ReadReg8(uint16_t reg);

    i2c_master_dev_handle_t device_;

    static constexpr uint16_t kRegModelId = 0x0000;
    static constexpr uint8_t kExpectedModelId = 0xB4;
    static constexpr uint16_t kRegSysRangeStart = 0x0018;
    static constexpr uint16_t kRegResultInterruptStatusGpio = 0x004F;
    static constexpr uint16_t kRegResultRangeVal = 0x0062;
    static constexpr uint16_t kRegSystemInterruptClear = 0x0015;
};

#endif  // VL6180X_H
