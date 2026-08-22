# Yana Wheelbot

([English](README.md) | [Tiếng Việt](README_vi.md) | 한국어)

Yana Robot / XiaoZhi 플랫폼 기반의 2륜 로봇(차동 구동) 보드입니다. 선택 가능한
모터 드라이버(연속 회전 서보 또는 L298N), 낙하 방지 ToF 센서, 듀얼 LED, 팔 +
목 서보, 전환 가능한 디스플레이 방향/테마를 플랫폼의 기존 음성 AI 및 감정
표현 디스플레이 위에 추가합니다.

## 필요한 부품

| 부품 | 예시 구성품 | 비고 |
|---|---|---|
| MCU | ESP32-S3-WROOM-1 N16R8 (16MB flash / 8MB PSRAM) | `config.json`의 `target: esp32s3` |
| 모터 드라이버 (택 1) | 연속 회전 서보(360°) 2개 | 바퀴를 직접 구동, 별도 드라이버 불필요 |
| | *또는* L298N + DC 모터 2개 | IN1-4 핀 규약 관련 주의사항은 "실제 하드웨어 미검증" 항목 참고 |
| 낙하 방지 센서 (ToF, I2C) | VL53L0X (기본값, 약 2m 범위) | |
| | *또는* VL6180X / TOF050C (빌드 옵션, 약 200mm 범위) | "빌드 시 변형 선택" 참고 |
| LED | 단색 LED 2개 (좌/우) | |
| 팔 + 목 서보 | 표준 각도 서보 2개 (0-180°) | |
| 터치 센서 | TTP223 정전식 터치 모듈 | 두 번 터치하면 채팅 켜기/끄기, 부팅 버튼과 동일 |
| 마이크 | I2S, 예: INMP441 또는 동급 제품 | |
| 스피커 | I2S 앰프, 예: MAX98357A + 3W/4Ω 스피커 | 자체 강한 5V 레일 필요 — 아래 전원 참고 |
| 디스플레이 | SPI ST7789 128x160 (기본값) | |
| | *또는* ST7735 (빌드 옵션) | "빌드 시 변형 선택" 참고 |
| 부팅 버튼 | ESP32-S3 개발 보드에 이미 내장 (GPIO0) | 추가 부품 불필요 |
| 전원 | Li-ion/LiPo 3.7V, 약 2000mAh | |
| | TP4056 Type-C 충전 + 5V 승압 모듈 | |
| | ON/OFF 전원 스위치 | |

**전원 참고:** 스피커 앰프와 두 바퀴 서보는 자체 강한 5V 레일이 필요합니다
(MCU의 3.3V 레귤레이터에서 직접 전원을 받으면 안 됨) — TP4056 같은 승압
모듈이 단일 3.7V LiPo 셀로 이를 처리합니다. 모든 GND(MCU, 앰프, 서보, 센서)
는 여전히 서로 연결되어 있어야 합니다.

## 기본 배선

![Yana Wheelbot 배선도](wiring-diagram.svg)

기본 GPIO 핀은 [KST AI Robot](https://ai.kenhsangtao.com/)이 공개한 배선도
(아래 "출처" 항목 참고)를 기준으로 하며, 디스플레이 백라이트를 제외한 모든
핀이 해당 배선도와 일치가 확인되었습니다(백라이트는 이 프로젝트 자체적으로
고른 여유 핀이며 비고 칸에 표시). 모든 정의는 `config.h`에 있습니다.

| 부품 | 신호 | GPIO | 비고 |
|---|---|---|---|
| 모터 (L298N) | IN1 (좌, PWM/EN) | 38 | `self.wheelbot.set_motor_pins`로 런타임 변경 가능 |
| | IN2 (좌, DIR) | 39 | |
| | IN3 (우, PWM/EN) | 40 | |
| | IN4 (우, DIR) | 41 | |
| 바퀴 서보 (L298N 대신 서보 사용 시) | 좌 | 47 | |
| | 우 | 45 | |
| ToF 센서 (I2C) | SDA | 1 | |
| | SCL | 2 | |
| LED | 좌 | 3 | |
| | 우 | 18 | |
| 팔 서보 | 신호 | 20 | |
| 목 서보 | 신호 | 21 | |
| 터치 센서 (TTP223) | OUT | 7 | Active-high; 두 번 터치하면 채팅 켜기/끄기 |
| 마이크 (I2S) | WS | 4 | |
| | SCK | 5 | |
| | DIN | 6 | |
| 스피커 (I2S) | DIN | 17 | |
| | BCLK | 16 | |
| | LRCK | 15 | |
| 디스플레이 (SPI) | 백라이트 | 9 | 이 프로젝트 자체 지정 핀 — KST의 모듈은 BL을 3V3에 직결, 소프트웨어 제어 없음 |
| | MOSI | 11 | |
| | CLK | 12 | |
| | DC | 10 | |
| | RST | 14 | |
| | CS | 13 | |

모터 IN1-4 핀(및 서보/L298N 드라이버 선택)은 MCP 툴을 통해 런타임에 변경
가능하며 NVS에 저장되어 재플래시가 필요 없습니다 — 위 값들은 최초 부팅 시의
기본값일 뿐입니다.

**GPIO36/GPIO37은 사용하지 마세요** — ESP32-S3-WROOM-1 N16R8 모듈에서
PSRAM 전용으로 예약되어 있습니다.

## 빌드 시 변형 선택

기본값은 ST7789 + VL53L0X입니다. 실제 부품이 ST7735 및/또는 VL6180X라면
빌드 전에 다음 두 Kconfig 옵션을 활성화하세요 (`idf.py menuconfig` →
"Yana Wheelbot", 또는 `sdkconfig`를 직접 수정):

```
CONFIG_YANA_WHEELBOT_DISPLAY_ST7735=y   # ST7789 대신 ST7735 드라이버 사용
CONFIG_YANA_WHEELBOT_TOF_VL6180X=y      # VL53L0X 대신 VL6180X 센서 사용
```

옵션 변경 후에는 `idf.py fullclean` 실행 후 다시 빌드하세요.

## 제어 프로토콜

이 보드는 동일한 MCP 툴 세트를 공유하는 2가지 프로토콜을 사용합니다:

**1. 로컬 제어 (LAN, 클라우드 왕복 없음)** — `main/boards/otto-robot`와
동일한 방식: 보드가 포트 8080에서 WebSocket 서버를 실행하며, 백엔드 AI 없이
MCP와 동일한 JSON-RPC 2.0 형식의 메시지를 그대로 받습니다.
`apps/controller-web`(이 저장소 내)이 이 프로토콜용으로 바로 사용 가능한
웹 클라이언트입니다.

**연결:** `ws://<장치-IP>:8080/ws`

```json
{"jsonrpc":"2.0","method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{}},"id":1}
{"jsonrpc":"2.0","method":"tools/list","params":{},"id":2}
{"jsonrpc":"2.0","method":"tools/call","params":{"name":"self.wheelbot.move_forward","arguments":{"duration_ms":2000,"speed":80}},"id":3}
```

**2. 음성 제어 (클라우드/백엔드 AI)** — 보드가 MCP **서버** 역할(툴을
소유하고 실행), 백엔드 AI가 MCP **클라이언트** 역할을 합니다 — 표준
XiaoZhi/yana-robot WebSocket 프로토콜(저장소 루트의 `docs/websocket.md`,
`docs/mcp-protocol.md`)을 그대로 따릅니다. 백엔드를 바꾸는 데 펌웨어 수정이
필요 없습니다 — 이 프로토콜을 구사하는 아무 백엔드로 보드의 서버 URL만
바꾸면 됩니다 (예: `Yana-AI` 프로젝트의 `tools/yana-web/robot.js`).

### 툴 목록

| 툴 | 인자 | 비고 |
|---|---|---|
| `self.wheelbot.move_forward` | `duration_ms` (0-30000, 기본값 2000), `speed` (0-100, 기본값 80) | |
| `self.wheelbot.move_backward` | 동일 | |
| `self.wheelbot.turn_left` | 동일 | 제자리 회전 |
| `self.wheelbot.turn_right` | 동일 | 제자리 회전 |
| `self.wheelbot.stop` | — | 즉시 비상 정지, 진행 중인 동작 중단 |
| `self.wheelbot.set_motor_type` | `type`: `"servo"` \| `"l298n"` | 저장 후 재부팅 시 적용 |
| `self.wheelbot.set_motor_pins` | `in1`,`in2`,`in3`,`in4` (GPIO 번호) | L298N 전용; 저장 후 재부팅 시 적용 |
| `self.wheelbot.get_motor_config` | — | 현재 드라이버 + 핀 + 보정값을 JSON으로 반환 |
| `self.wheelbot.set_servo_stop_pulse` | `microseconds` (1000-2000, 기본값 1500) | 즉시 적용; 서보 드라이버 전용 |
| `self.wheelbot.set_servo_reverse` | `side`: `"left"`\|`"right"`, `reversed` (bool) | 즉시 적용; 서보 드라이버 전용 |
| `self.cliff_sensor.set_enabled` | `enabled` (bool) | |
| `self.cliff_sensor.set_threshold` | `threshold_mm` (5-500, 기본값 50) | |
| `self.cliff_sensor.get_config` | — | |
| `self.cliff_sensor.test_now` | — | 거리 측정값 1회 반환(mm) |
| `self.led.set_mode` | `mode`: `follow_state`\|`both_on`\|`both_off`\|`left_only`\|`right_only` | `follow_state`는 다른 보드의 기본 LED 상태 동작과 동일 |
| `self.led.get_mode` | — | |
| `self.arm.set_angle` / `self.neck.set_angle` | `angle` (0-180) | |
| `self.arm.wave` | — | 미리 정의된 동작, 논블로킹 |
| `self.neck.turn` | `direction`: `left`\|`right`\|`center` | |
| `self.arm.release` / `self.neck.release` | — | PWM 정지, 서보가 힘없이 풀림 |
| `self.screen.set_theme` | `theme`: `light`\|`dark`\|`ocean` | `mcp_server.cc`의 공통 툴; `ocean`은 이 보드만의 추가 테마 |
| `self.screen.set_orientation` | `orientation`: `portrait`\|`landscape` | 저장 후 재부팅 시 적용 |

## 출처

`config.h`의 기본 GPIO 값(모터, ToF, LED, 팔/목, 터치 센서, 마이크, 스피커,
디스플레이 핀)은 [KST AI Robot](https://ai.kenhsangtao.com/)이 공개한
배선도와 펌웨어 센터(`kenhsangtao.github.io/robotai`)를 기준으로 맞췄습니다
— 베트남 "Kênh Sáng Tạo" 채널이 만든 실제 커뮤니티 제작 ESP32-S3 로봇으로,
거의 동일한 기능(선택 가능한 모터 드라이버, 팔/목 서보, ToF 낙하 방지, 듀얼
LED, 터치 센서, 음성 AI)을 갖추고 있습니다. 공개된 자료에서 **핀 번호**와
**공개된 부품명**만 가져왔으며, 그들의 펌웨어(재사용 라이선스가 명시되지
않은 비공개 바이너리)나 웹사이트에서 코드, 이미지, 텍스트를 가져오지
않았습니다. 위의 `wiring-diagram.svg`는 이러한 공개된 사실을 바탕으로 이
프로젝트가 직접 그린 독자적인 다이어그램이며, 그들의 배선도 이미지를 복사한
것이 아닙니다. 이 보드의 펌웨어, 보드 정의, MCP 툴은 독립적으로
작성되었습니다.

## 실제 하드웨어에서 아직 미검증

아래 항목들은 빌드 검증(컴파일 성공)만 되었을 뿐, **아직 실제 로봇으로
동작시켜 본 적이 없습니다** — 완전히 신뢰하기 전에 반드시 확인하세요:

- **L298N 핀 규약**: 이 보드는 각 측에 PWM/EN 핀 1개 + 방향 핀 1개를
  가정합니다(`in1`=좌 PWM, `in2`=좌 방향, `in3`=우 PWM, `in4`=우 방향).
  일부 L298N 보드는 배선이 다릅니다(별도 EN 없이 4개의 H-브리지 핀 직결).
  KST AI Robot의 펌웨어 문자열은 그들의 "미니 모터 드라이버"가 유사한
  모듈에서 IN1-4에 정적(비-PWM) 레벨을 직접 사용함을 암시합니다 — 이는
  확인된 사양이 아니라 문서화되지 않은 바이너리 문자열에서 나온 *주목할
  만한 신호*일 뿐입니다. 실제 모듈로 확인 후 `l298n_motor_driver.cc`를
  신뢰하세요.
- **연속 회전 서보 속도 매핑** (`servo_motor_driver.cc`): microseconds
  대비 속도(%) 기울기는 합리적인 기본값일 뿐, 실제 서보로 측정되지
  않았습니다. `self.wheelbot.set_servo_stop_pulse`와 `kHalfRangeUs`
  상수로 조정하세요.
- **VL53L0X 드라이버** (`vl53l0x.cc`): 존재 확인 + 문서화된 단발성 거리
  측정 트리거/폴링/읽기 시퀀스만 구현합니다. ST의 전체 기준 보정(SPAD,
  타이밍 버짓, 신호율 제한 튜닝)은 구현하지 **않았습니다**. 낙하 방지
  안전에 사용하기 전 알려진 거리로 측정값을 검증하세요.
- **VL6180X 드라이버** (`vl6180x.cc`): VL53L0X와 동일한 범위 — 존재 확인 +
  기본 단발성 거리 읽기만 구현하며, ST의 AN4545에 명시된 필수 약 30개
  레지스터 비공개 초기화 시퀀스는 구현하지 **않았습니다**. 데이터시트로
  검증하기 전에 사용하면 잘못된 측정값이 나올 실질적 위험이 있습니다.
- **디스플레이 방향**: 실시간 재배치가 아니라 재부팅을 통해 적용됩니다 —
  재부팅 없이 런타임에 `esp_lcd_panel_swap_xy`/`mirror`를 다시 호출하는
  것이 이 패널/드라이버 조합에서 안전하다고 검증되지 않았기 때문입니다.
- **TTP223 터치 센서**: 단순한 active-high 디지털 `Button`(`OnDoubleClick`)
  으로 연결되어 있습니다 — TTP223 모듈은 저렴하지만 3V3 레일이 불안정하면
  노이즈나 자체 트리거가 발생할 수 있으며, 디바운스 동작은 실제 하드웨어에서
  테스트되지 않았습니다.
