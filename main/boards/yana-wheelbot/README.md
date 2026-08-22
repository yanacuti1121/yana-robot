# Yana Wheelbot

(English | [Tiếng Việt](README_vi.md) | [한국어](README_ko.md))

A wheeled-robot (differential-drive) board on the Yana Robot / XiaoZhi
platform. Adds: a selectable motor backend (continuous-rotation servo or
L298N), an anti-fall ToF sensor, dual LEDs, arm + neck servos, and
switchable display orientation/theme — on top of the platform's existing
voice AI and animated-face display.

## Required parts

| Part | Suggested component | Notes |
|---|---|---|
| MCU | ESP32-S3 (any dev board with enough pins) | `target: esp32s3` in `config.json` |
| Motor driver (pick one) | 2x continuous-rotation servo | Drives the wheels directly, no separate driver needed |
| | *or* L298N + 2x DC motor | See the IN1-4 pin-convention caveat in "Not yet verified on real hardware" |
| Anti-fall sensor (ToF, I2C) | VL53L0X (default, ~2m range) | |
| | *or* VL6180X (build option, ~200mm range) | See "Build-time variant selection" |
| LEDs | 2x single LED (left/right) | |
| Arm + neck servos | 2x standard angle servo (0-180°) | |
| Mic | I2S, e.g. INMP441 or equivalent | |
| Speaker | I2S amp, e.g. MAX98357A | |
| Display | SPI ST7789 128x160 (default) | |
| | *or* ST7735 (build option) | See "Build-time variant selection" |
| Boot button | already on the ESP32-S3 dev board (GPIO0) | Nothing extra needed |

## Default wiring

Default GPIO pins are taken from the [KST AI Robot](https://ai.kenhsangtao.com/)'s
publicly published wiring diagram (see the "Credit" section below) — most
are confirmed against that diagram, the remaining two are this project's own
free-pin picks (flagged in the Notes column). All definitions live in
`config.h`.

| Part | Signal | GPIO | Notes |
|---|---|---|---|
| Motor (L298N) | IN1 (left, PWM/EN) | 38 | Remappable at runtime via `self.wheelbot.set_motor_pins` |
| | IN2 (left, DIR) | 39 | |
| | IN3 (right, PWM/EN) | 40 | |
| | IN4 (right, DIR) | 41 | |
| Wheel servo (if using servo instead of L298N) | Left | 47 | |
| | Right | 45 | |
| ToF sensor (I2C) | SDA | 1 | |
| | SCL | 2 | |
| LED | Left | 3 | |
| | Right | 18 | |
| Arm servo | Signal | 20 | |
| Neck servo | Signal | 21 | |
| Mic (I2S) | WS | 4 | |
| | SCK | 5 | |
| | DIN | 6 | |
| Speaker (I2S) | DOUT | 9 | **Not confirmed** — not legible in KST's diagram, picked as a free pin |
| | BCLK | 16 | |
| | LRCK | 15 | |
| Display (SPI) | Backlight | 17 | **Not confirmed** — KST's ST7735 module may not expose a separate backlight pin |
| | MOSI | 11 | |
| | CLK | 12 | |
| | DC | 10 | |
| | RST | 14 | |
| | CS | 13 | |

Motor IN1-4 pins (and the servo/L298N backend choice) can be changed at
runtime via an MCP tool, persisted to NVS, no reflash required — the values
above are only the first-boot defaults.

## Build-time variant selection

Defaults to ST7789 + VL53L0X. If your actual parts are ST7735 and/or
VL6180X, enable these two Kconfig options before building (`idf.py
menuconfig` → "Yana Wheelbot", or edit `sdkconfig` directly):

```
CONFIG_YANA_WHEELBOT_DISPLAY_ST7735=y   # use the ST7735 driver instead of ST7789
CONFIG_YANA_WHEELBOT_TOF_VL6180X=y      # use the VL6180X sensor instead of VL53L0X
```

Run `idf.py fullclean` and rebuild after changing either option.

## Control protocol

This board speaks 2 protocols, sharing the same set of MCP tools:

**1. Local control (LAN, no cloud round-trip)** — works the same way as
`main/boards/otto-robot`: the board runs a WebSocket server on port 8080
that accepts messages in the same JSON-RPC 2.0 envelope MCP uses, with no
backend AI in the loop. `apps/controller-web` (in this repo) is a ready-made
web client for this protocol.

**Connect:** `ws://<device-ip>:8080/ws`

```json
{"jsonrpc":"2.0","method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{}},"id":1}
{"jsonrpc":"2.0","method":"tools/list","params":{},"id":2}
{"jsonrpc":"2.0","method":"tools/call","params":{"name":"self.wheelbot.move_forward","arguments":{"duration_ms":2000,"speed":80}},"id":3}
```

**2. Voice control (cloud/backend AI)** — the board acts as the MCP
**server** (owns and executes the tools), the backend AI acts as the MCP
**client** — the standard XiaoZhi/yana-robot WebSocket protocol
(`docs/websocket.md`, `docs/mcp-protocol.md` at the repo root). No firmware
change is needed to switch backends — just point the board's server URL at
any backend speaking this same protocol (for example, the `Yana-AI`
project's `tools/yana-web/robot.js`).

### Tool list

| Tool | Arguments | Notes |
|---|---|---|
| `self.wheelbot.move_forward` | `duration_ms` (0-30000, default 2000), `speed` (0-100, default 80) | |
| `self.wheelbot.move_backward` | same | |
| `self.wheelbot.turn_left` | same | in-place turn |
| `self.wheelbot.turn_right` | same | in-place turn |
| `self.wheelbot.stop` | — | immediate e-stop, interrupts any in-flight move |
| `self.wheelbot.set_motor_type` | `type`: `"servo"` \| `"l298n"` | persists, reboots to apply |
| `self.wheelbot.set_motor_pins` | `in1`,`in2`,`in3`,`in4` (GPIO numbers) | L298N pins only; persists, reboots to apply |
| `self.wheelbot.get_motor_config` | — | returns current backend + pins + calibration as JSON |
| `self.wheelbot.set_servo_stop_pulse` | `microseconds` (1000-2000, default 1500) | live-apply; servo backend only |
| `self.wheelbot.set_servo_reverse` | `side`: `"left"`\|`"right"`, `reversed` (bool) | live-apply; servo backend only |
| `self.cliff_sensor.set_enabled` | `enabled` (bool) | |
| `self.cliff_sensor.set_threshold` | `threshold_mm` (5-500, default 50) | |
| `self.cliff_sensor.get_config` | — | |
| `self.cliff_sensor.test_now` | — | returns one mm reading |
| `self.led.set_mode` | `mode`: `follow_state`\|`both_on`\|`both_off`\|`left_only`\|`right_only` | `follow_state` mirrors the device-state color/blink behavior every other board's LED already has |
| `self.led.get_mode` | — | |
| `self.arm.set_angle` / `self.neck.set_angle` | `angle` (0-180) | |
| `self.arm.wave` | — | canned gesture, non-blocking |
| `self.neck.turn` | `direction`: `left`\|`right`\|`center` | |
| `self.arm.release` / `self.neck.release` | — | stops PWM so the servo goes limp |
| `self.screen.set_theme` | `theme`: `light`\|`dark`\|`ocean` | already generic in `mcp_server.cc`; `ocean` is this board's addition |
| `self.screen.set_orientation` | `orientation`: `portrait`\|`landscape` | persists, reboots to apply |

## Credit

The GPIO defaults in `config.h` (motor, ToF, LED, arm/neck, mic, display pins)
are aligned with the [KST AI Robot](https://ai.kenhsangtao.com/)'s publicly
published wiring diagram — a real, community-built ESP32-S3 robot from the
Vietnamese "Kênh Sáng Tạo" channel with a nearly identical feature set (dual
motor backend, arm/neck servos, ToF anti-fall, dual LED, voice AI). Only pin
*numbers* from their public diagram were used; no code was taken from their
firmware (a closed binary with no stated reuse license). This board's
firmware, board definition, and MCP tools are written independently.

## Not yet verified on real hardware

Everything below is only build-verified (compiles cleanly) — **no real
robot has run this yet** — verify before fully trusting it:

- **L298N pin convention**: this board assumes one PWM/EN pin + one direction
  pin per side (`in1`=left PWM, `in2`=left DIR, `in3`=right PWM, `in4`=right
  DIR). Some L298N breakouts wire IN1-4 differently (4 direct H-bridge-leg
  pins, no separate EN). The KST AI Robot's firmware strings suggest their
  "mini motor driver" uses static (non-PWM) levels directly on IN1-4 for a
  similar module — this is a signal worth investigating, not a confirmed
  spec, since it came from binary strings, not documented source. Verify
  against your actual module before trusting `l298n_motor_driver.cc`.
- **Continuous-rotation servo speed mapping** (`servo_motor_driver.cc`): the
  microseconds-per-percent-speed slope is a reasonable default, not measured
  against a real servo. Tune via `self.wheelbot.set_servo_stop_pulse` and the
  `kHalfRangeUs` constant.
- **VL53L0X driver** (`vl53l0x.cc`): implements presence detection + the
  documented single-shot ranging trigger/poll/read sequence only. It does
  **not** implement ST's full reference calibration (SPAD, timing budget,
  signal-rate-limit tuning). Verify readings against a known distance before
  trusting them for anti-fall safety.
- **VL6180X driver** (`vl6180x.cc`): same scope as VL53L0X above — presence
  check + basic single-shot range read only, does **not** implement ST's
  mandatory ~30-register private init sequence from AN4545. Real risk of
  bad readings if used before verifying against the datasheet.
- **Display orientation**: applied via a reboot rather than a live re-layout,
  since re-invoking `esp_lcd_panel_swap_xy`/`mirror` at runtime without a
  reboot is not verified safe on this panel/driver combination.
