# Yana Wheelbot

A wheeled-robot board built on the Yana Robot / XiaoZhi platform. Adds
differential-drive movement, a selectable motor backend (continuous-rotation
servo pair or L298N DC motor driver), a VL53L0X anti-fall sensor, dual LEDs,
arm+neck servos, and switchable display orientation/theme — on top of the
platform's existing voice AI and animated-face display.

## Credit

The GPIO defaults in `config.h` (motor, ToF, LED, arm/neck, mic, display pins)
are aligned with the [KST AI Robot](https://ai.kenhsangtao.com/)'s publicly
published wiring diagram — a real, community-built ESP32-S3 robot from the
Vietnamese "Kênh Sáng Tạo" channel with a nearly identical feature set (dual
motor backend, arm/neck servos, ToF anti-fall, dual LED, voice AI). Only pin
*numbers* from their public diagram were used; no code was taken from their
firmware (a closed binary with no stated reuse license — see the diagram
fetch date and details in this project's own history for what was and
wasn't used). This board's firmware, board definition, and MCP tools are
written independently.

Local control (no cloud round-trip) works the same way as
`main/boards/otto-robot`: this board runs a WebSocket server on port 8080 that
forwards messages directly into the same MCP tool dispatch used by the cloud
protocol, so any client speaking the JSON-RPC-2.0-style envelope below can
drive the robot over the LAN.

**Connect:** `ws://<device-ip>:8080/ws`

```json
{"jsonrpc":"2.0","method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{}},"id":1}
{"jsonrpc":"2.0","method":"tools/list","params":{},"id":2}
{"jsonrpc":"2.0","method":"tools/call","params":{"name":"self.wheelbot.move_forward","arguments":{"duration_ms":2000,"speed":80}},"id":3}
```

## Tools

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

## Known gaps / needs hardware verification

- **L298N pin convention**: this board assumes one PWM/EN pin + one direction
  pin per side (`in1`=left PWM, `in2`=left DIR, `in3`=right PWM, `in4`=right
  DIR). Some L298N breakouts wire IN1-4 differently (4 direct H-bridge-leg
  pins, no separate EN). The KST AI Robot's firmware strings (see the
  "Credit" section above) suggest their "mini motor driver" uses static
  (non-PWM) levels directly on IN1-4 for a similar module — this is a signal
  worth investigating, not a confirmed spec, since it came from binary
  strings, not documented source. Verify against your actual module before
  trusting `l298n_motor_driver.cc`.
- **Continuous-rotation servo speed mapping** (`servo_motor_driver.cc`): the
  microseconds-per-percent-speed slope is a reasonable default, not measured
  against a real servo. Tune via `self.wheelbot.set_servo_stop_pulse` and the
  `kHalfRangeUs` constant.
- **VL53L0X driver** (`vl53l0x.cc`): implements presence detection + the
  documented single-shot ranging trigger/poll/read sequence only. It does
  **not** implement ST's full reference calibration (SPAD, timing budget,
  signal-rate-limit tuning). Verify readings against a known distance before
  trusting them for anti-fall safety.
- **Display orientation**: applied via a reboot rather than a live re-layout,
  since re-invoking `esp_lcd_panel_swap_xy`/`mirror` at runtime without a
  reboot is not verified safe on this panel/driver combination.
