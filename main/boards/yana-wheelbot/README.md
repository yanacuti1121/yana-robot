# Yana Wheelbot

Board robot 2 bánh (differential-drive) trên nền tảng Yana Robot / XiaoZhi.
Thêm: driver động cơ chọn được (servo liên tục hoặc L298N), cảm biến chống
rơi ToF, 2 LED, servo tay + cổ, và màn hình đổi được hướng/theme — bên trên
phần voice AI + màn hình biểu cảm có sẵn của nền tảng.

## Linh kiện cần có

| Bộ phận | Linh kiện gợi ý | Ghi chú |
|---|---|---|
| Vi điều khiển | ESP32-S3 (bất kỳ board dev nào có đủ chân) | `target: esp32s3` trong `config.json` |
| Driver động cơ (chọn 1) | 2x servo xoay liên tục (continuous-rotation) | Kéo bánh trực tiếp, không cần driver rời |
| | *hoặc* L298N + 2 động cơ DC | Xem lưu ý về quy ước chân IN1-4 ở phần "Chưa xác minh" |
| Cảm biến chống rơi (ToF, I2C) | VL53L0X (mặc định, tầm ~2m) | |
| | *hoặc* VL6180X (build option, tầm ~200mm) | Xem "Chọn biến thể lúc build" |
| LED | 2x LED đơn (trái/phải) | |
| Servo tay + cổ | 2x servo góc thường (0-180°) | |
| Mic | I2S, kiểu INMP441 hoặc tương đương | |
| Loa | I2S amp, kiểu MAX98357A | |
| Màn hình | SPI ST7789 128x160 (mặc định) | |
| | *hoặc* ST7735 (build option) | Xem "Chọn biến thể lúc build" |
| Nút Boot | có sẵn trên board ESP32-S3 (GPIO0) | Không cần thêm |

## Sơ đồ đấu nối (mặc định)

Chân GPIO mặc định lấy theo sơ đồ đấu nối công khai của
[KST AI Robot](https://ai.kenhsangtao.com/) (xem mục "Credit" bên dưới) —
đa số đã xác nhận khớp với sơ đồ đó, 2 chân còn lại là tự chọn (đánh dấu rõ ở
cột Ghi chú). Toàn bộ định nghĩa nằm ở `config.h`.

| Bộ phận | Tín hiệu | GPIO | Ghi chú |
|---|---|---|---|
| Động cơ (L298N) | IN1 (trái, PWM/EN) | 38 | Đổi được lúc chạy qua `self.wheelbot.set_motor_pins` |
| | IN2 (trái, DIR) | 39 | |
| | IN3 (phải, PWM/EN) | 40 | |
| | IN4 (phải, DIR) | 41 | |
| Servo bánh (nếu dùng servo thay L298N) | Trái | 47 | |
| | Phải | 45 | |
| Cảm biến ToF (I2C) | SDA | 1 | |
| | SCL | 2 | |
| LED | Trái | 3 | |
| | Phải | 18 | |
| Servo tay | Tín hiệu | 20 | |
| Servo cổ | Tín hiệu | 21 | |
| Mic (I2S) | WS | 4 | |
| | SCK | 5 | |
| | DIN | 6 | |
| Loa (I2S) | DOUT | 9 | **Chưa xác nhận** — không thấy rõ trong sơ đồ KST, tự chọn chân trống |
| | BCLK | 16 | |
| | LRCK | 15 | |
| Màn hình (SPI) | Backlight | 17 | **Chưa xác nhận** — module ST7735 của KST có thể không có chân backlight riêng |
| | MOSI | 11 | |
| | CLK | 12 | |
| | DC | 10 | |
| | RST | 14 | |
| | CS | 13 | |

Chân IN1-4 của motor (và loại motor servo/L298N) có thể đổi lúc chạy qua MCP
tool, lưu vào NVS, không cần nạp lại firmware — các giá trị trên chỉ là mặc
định lúc mới nạp.

## Chọn biến thể lúc build

Mặc định build cho ST7789 + VL53L0X. Nếu linh kiện thực tế là ST7735 và/hoặc
VL6180X, bật 2 tùy chọn Kconfig sau trước khi build (`idf.py menuconfig` →
"Yana Wheelbot" hoặc sửa thẳng `sdkconfig`):

```
CONFIG_YANA_WHEELBOT_DISPLAY_ST7735=y   # dùng driver ST7735 thay vì ST7789
CONFIG_YANA_WHEELBOT_TOF_VL6180X=y      # dùng cảm biến VL6180X thay vì VL53L0X
```

Sau khi đổi, chạy `idf.py fullclean` rồi build lại.

## Giao thức điều khiển

Board này nói 2 giao thức, dùng chung một bộ MCP tool:

**1. Điều khiển cục bộ (LAN, không qua cloud)** — giống hệt cách
`main/boards/otto-robot` làm: board mở WebSocket server ở cổng 8080, nhận
thẳng message theo đúng khuôn JSON-RPC 2.0 mà MCP dùng, không cần đi qua
backend AI. `apps/controller-web` (trong repo này) là một client web sẵn
dùng cho giao thức này.

**Kết nối:** `ws://<ip-của-board>:8080/ws`

```json
{"jsonrpc":"2.0","method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{}},"id":1}
{"jsonrpc":"2.0","method":"tools/list","params":{},"id":2}
{"jsonrpc":"2.0","method":"tools/call","params":{"name":"self.wheelbot.move_forward","arguments":{"duration_ms":2000,"speed":80}},"id":3}
```

**2. Điều khiển bằng giọng nói (cloud/backend AI)** — board đóng vai trò MCP
**server** (giữ và thực thi tool), backend AI đóng vai trò MCP **client** —
đúng giao thức WebSocket chuẩn của XiaoZhi/yana-robot
(`docs/websocket.md`, `docs/mcp-protocol.md` ở gốc repo). Không cần sửa
firmware để đổi backend — chỉ cần trỏ server URL của board sang bất kỳ
backend nào nói đúng giao thức này (ví dụ dự án `Yana-AI`'s
`tools/yana-web/robot.js`).

### Danh sách tool

| Tool | Tham số | Ghi chú |
|---|---|---|
| `self.wheelbot.move_forward` | `duration_ms` (0-30000, mặc định 2000), `speed` (0-100, mặc định 80) | |
| `self.wheelbot.move_backward` | như trên | |
| `self.wheelbot.turn_left` | như trên | quay tại chỗ |
| `self.wheelbot.turn_right` | như trên | quay tại chỗ |
| `self.wheelbot.stop` | — | dừng khẩn cấp, ngắt lệnh đang chạy |
| `self.wheelbot.set_motor_type` | `type`: `"servo"` \| `"l298n"` | lưu NVS, khởi động lại để áp dụng |
| `self.wheelbot.set_motor_pins` | `in1`,`in2`,`in3`,`in4` (số GPIO) | chỉ áp dụng cho L298N; lưu NVS, khởi động lại |
| `self.wheelbot.get_motor_config` | — | trả về backend + chân + hiệu chỉnh hiện tại (JSON) |
| `self.wheelbot.set_servo_stop_pulse` | `microseconds` (1000-2000, mặc định 1500) | áp dụng ngay; chỉ backend servo |
| `self.wheelbot.set_servo_reverse` | `side`: `"left"`\|`"right"`, `reversed` (bool) | áp dụng ngay; chỉ backend servo |
| `self.cliff_sensor.set_enabled` | `enabled` (bool) | |
| `self.cliff_sensor.set_threshold` | `threshold_mm` (5-500, mặc định 50) | |
| `self.cliff_sensor.get_config` | — | |
| `self.cliff_sensor.test_now` | — | trả về 1 lần đọc khoảng cách (mm) |
| `self.led.set_mode` | `mode`: `follow_state`\|`both_on`\|`both_off`\|`left_only`\|`right_only` | `follow_state` giống hành vi LED trạng thái mặc định của các board khác |
| `self.led.get_mode` | — | |
| `self.arm.set_angle` / `self.neck.set_angle` | `angle` (0-180) | |
| `self.arm.wave` | — | cử chỉ vẫy tay dựng sẵn, không chặn luồng |
| `self.neck.turn` | `direction`: `left`\|`right`\|`center` | |
| `self.arm.release` / `self.neck.release` | — | ngắt PWM, servo mềm tự do |
| `self.screen.set_theme` | `theme`: `light`\|`dark`\|`ocean` | tool chung của `mcp_server.cc`; `ocean` là theme riêng của board này |
| `self.screen.set_orientation` | `orientation`: `portrait`\|`landscape` | lưu NVS, khởi động lại để áp dụng |

## Credit

Chân GPIO mặc định trong `config.h` (motor, ToF, LED, tay/cổ, mic, màn hình)
được căn theo sơ đồ đấu nối công khai của
[KST AI Robot](https://ai.kenhsangtao.com/) — một robot ESP32-S3 thật, do
cộng đồng xây dựng, từ kênh "Kênh Sáng Tạo" (Việt Nam), có tính năng gần
giống hệt (driver động cơ chọn được, servo tay/cổ, chống rơi ToF, LED đôi,
voice AI). Chỉ lấy **số GPIO** từ sơ đồ công khai của họ; không lấy bất kỳ
dòng code nào từ firmware của họ (một file binary đóng, không nêu license
cho phép tái sử dụng). Firmware, board definition, và MCP tool ở đây được
viết độc lập.

## Chưa xác minh trên phần cứng thật

Toàn bộ mục dưới đây mới chỉ build-verified (compile sạch), **chưa có robot
thật nào chạy thử** — cần kiểm chứng trước khi tin tưởng hoàn toàn:

- **Quy ước chân L298N**: board này giả định mỗi bên có 1 chân PWM/EN + 1
  chân DIR (`in1`=PWM trái, `in2`=DIR trái, `in3`=PWM phải, `in4`=DIR phải).
  Một số board L298N đấu khác (4 chân H-bridge trực tiếp, không tách EN
  riêng). Chuỗi ký tự trong firmware của KST AI Robot gợi ý "mini motor
  driver" của họ dùng mức tĩnh (không PWM) trực tiếp trên IN1-4 — đây là
  *tín hiệu đáng lưu ý*, không phải spec đã xác nhận, vì lấy từ binary
  string chứ không phải tài liệu công khai. Kiểm tra lại với module thật
  trước khi tin `l298n_motor_driver.cc`.
- **Tốc độ servo xoay liên tục** (`servo_motor_driver.cc`): độ dốc
  microseconds-trên-phần-trăm-tốc-độ là giá trị mặc định hợp lý, chưa đo
  trên servo thật. Hiệu chỉnh qua `self.wheelbot.set_servo_stop_pulse` và
  hằng số `kHalfRangeUs`.
- **Driver VL53L0X** (`vl53l0x.cc`): chỉ implement kiểm tra sự hiện diện +
  chuỗi trigger/poll/read đo khoảng cách đơn giản theo tài liệu. **Không**
  implement calibration đầy đủ của ST (SPAD, timing budget, signal-rate
  limit). Kiểm tra lại với khoảng cách đã biết trước khi dùng cho an toàn
  chống rơi.
- **Driver VL6180X** (`vl6180x.cc`): tương tự VL53L0X — chỉ kiểm tra sự hiện
  diện + đọc khoảng cách đơn giản, **không** implement chuỗi khởi tạo bắt
  buộc ~30 thanh ghi riêng theo AN4545 của ST. Rủi ro đọc sai nếu dùng thật
  mà chưa kiểm chứng.
- **Hướng màn hình**: áp dụng bằng cách khởi động lại, không phải đổi layout
  ngay lập tức — vì gọi lại `esp_lcd_panel_swap_xy`/`mirror` lúc đang chạy
  mà không khởi động lại chưa được xác minh an toàn với cặp panel/driver
  này.
