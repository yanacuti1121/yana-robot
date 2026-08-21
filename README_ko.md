# Yana Robot

(한국어 | [English](README.md) | [Tiếng Việt](README_vi.md))

## 개요

Yana Robot은 ESP32 하드웨어를 위한 MCP 기반 음성 어시스턴트 펌웨어입니다. 오프라인 웨이크워드 감지, 스트리밍 ASR/LLM/TTS 파이프라인, WebSocket 및 MQTT+UDP 두 가지 전송 방식, 그리고 디바이스 측/클라우드 측 MCP 도구 제어를 결합하여 다양한 보드 변형에서 동작합니다.

<img src="docs/mcp-based-graph.jpg" alt="MCP로 모든 것을 제어" width="320">

## 출처

Yana Robot은 [XiaoZhi AI Chatbot](https://github.com/78/xiaozhi-esp32) 프로젝트를 기반으로 만들어졌습니다. 이는 Xiaoqiang([78](https://github.com/78))과 Shenzhen Xinzhi Future Technology Co., Ltd.가 만든 오리지널 오픈소스 ESP32 음성 어시스턴트 펌웨어입니다. 원본 코드베이스, 프로토콜 설계, 하드웨어 생태계에 대한 모든 공로는 해당 프로젝트와 기여자들에게 있습니다.

이 저장소는 독립적으로 이어가는 프로젝트입니다. 업스트림 프로젝트와 제휴 관계가 없으며 더 이상 동기화되지 않습니다. 이 시점 이후로 이 저장소의 보드, 버그 수정, 기능은 Yana Robot이라는 이름 아래 독립적으로 설계 및 유지보수됩니다.

## 최근 업데이트

- 새 보드: [`yana-wheelbot`](main/boards/yana-wheelbot) — 런타임에 전환 가능한 모터 백엔드(연속 회전 서보 페어 또는 L298N DC 드라이버), VL53L0X 추락 방지 센서, 듀얼 LED, 팔+목 서보, 전환 가능한 화면 방향/테마를 갖춘 바퀴형 로봇으로, 모두 MCP 도구로 노출됩니다. 클라우드 프로토콜과 동일한 로컬 WebSocket/MCP 채널로 제어하는 브라우저 기반 컨트롤 패널 [`apps/controller-web`](apps/controller-web)도 함께 제공됩니다.
- 메인라인은 이제 ESP-IDF v6.0 이상을 대상으로 하며, v6.0.2를 권장 안정 버전으로 사용합니다. 이전 157개 릴리스 변형은 ESP-IDF v6.0.1에서 빌드 검증되었으며, 현재 매트릭스는 171개 변형을 포함하고 그중 170개가 IDF 6.0.x를 지원하며 ESP32-S31 변형은 IDF 6.1 이상이 필요합니다.
- MQTT 및 BluFi 암호화 코드가 PSA Crypto로 마이그레이션되었습니다. IDF 6 컴포넌트 분리 및 서드파티 의존성 호환성도 처리되었습니다.
- 오디오 파이프라인 동시성, MQTT/UDP 패킷 검증, 릴리스 매트릭스 선택 로직이 강화되었습니다.
- ESP-IDF v5.5는 문서화된 레거시 보드에만 유지됩니다. ESP32-P4 Rev1과 Rev3 모두 ESP-SR 2.4.7과 함께 IDF 6에서 지원됩니다. 자세한 호환성 및 보드 검증 상태는 [ESP-IDF 6.0 마이그레이션 가이드](docs/esp-idf-6-migration.md)를 참고하세요.

### 구현된 기능

- Wi-Fi, 유선 이더넷, USB RNDIS, ML307/EC801E 또는 NT26 Cat.1 4G 네트워킹; 지원 보드는 Wi-Fi와 4G 간 전환 가능
- [ESP-SR](https://github.com/espressif/esp-sr) 기반 오프라인 음성 웨이크업, 커스텀 웨이크워드 지원
- 두 가지 통신 방식: [WebSocket](docs/websocket.md)과 [MQTT + UDP](docs/mqtt-udp.md)
- 기존 스트리밍 ASR + LLM + TTS 파이프라인과 Realtime 엔드투엔드 음성 모델을 모두 지원하는 Opus 오디오 스트리밍; AEC 지원 하드웨어에서는 실시간 전이중 상호작용 가능
- 화자 인식, 현재 발화자 식별 [3D Speaker](https://github.com/modelscope/3D-Speaker)
- 이모지와 풍부한 표정을 지원하는 OLED / LCD 디스플레이, 지원 보드에서는 카메라 비전 입력도 가능
- 배터리 표시 및 전원 관리
- 39개 인터페이스 언어, 가능한 경우 현지화된 음성 프롬프트 사용, 없을 경우 영어로 폴백
- ESP32, ESP32-C3, ESP32-C5, ESP32-C6, ESP32-S3, ESP32-P4 칩 플랫폼
- 핫스팟 또는 BluFi를 통한 Wi-Fi 프로비저닝
- 디바이스 제어를 위한 디바이스 측 MCP (스피커, LED, 서보, GPIO 등)
- 대형 모델 기능 확장을 위한 클라우드 측 MCP (스마트홈 제어, PC 데스크톱 조작, 지식 검색, 이메일 등)
- 온라인 웹 편집을 지원하는 커스텀 웨이크워드, 폰트, 이모지, 채팅 배경 ([Custom Assets Generator](https://github.com/78/xiaozhi-assets-generator))

## 하드웨어

### 138개 보드 디렉토리 및 171개 릴리스 변형 지원 (일부 목록)

- <a href="https://oshwhub.com/li-chuang-kai-fa-ban/li-chuang-shi-zhan-pai-esp32-s3-kai-fa-ban" target="_blank" title="LiChuang ESP32-S3 Development Board">LiChuang ESP32-S3 Development Board</a>
- <a href="https://github.com/espressif/esp-box" target="_blank" title="Espressif ESP32-S3-BOX-3">Espressif ESP32-S3-BOX-3</a>
- <a href="https://docs.m5stack.com/zh_CN/core/CoreS3" target="_blank" title="M5Stack CoreS3">M5Stack CoreS3</a>
- <a href="https://docs.m5stack.com/en/atom/Atomic%20Echo%20Base" target="_blank" title="AtomS3R + Echo Base">M5Stack AtomS3R + Echo Base</a>
- <a href="https://gf.bilibili.com/item/detail/1108782064" target="_blank" title="Magic Button 2.4">Magic Button 2.4</a>
- <a href="https://www.waveshare.net/shop/ESP32-S3-Touch-AMOLED-1.8.htm" target="_blank" title="Waveshare ESP32-S3-Touch-AMOLED-1.8">Waveshare ESP32-S3-Touch-AMOLED-1.8</a>
- <a href="https://github.com/Xinyuan-LilyGO/T-Circle-S3" target="_blank" title="LILYGO T-Circle-S3">LILYGO T-Circle-S3</a>
- <a href="https://oshwhub.com/tenclass01/xmini_c3" target="_blank" title="XiaGe Mini C3">XiaGe Mini C3</a>
- <a href="https://oshwhub.com/movecall/cuican-ai-pendant-lights-up-y" target="_blank" title="Movecall CuiCan ESP32S3">CuiCan AI Pendant</a>
- <a href="https://github.com/WMnologo/xingzhi-ai" target="_blank" title="WMnologo-Xingzhi-1.54">WMnologo-Xingzhi-1.54TFT</a>
- <a href="https://www.seeedstudio.com/SenseCAP-Watcher-W1-A-p-5979.html" target="_blank" title="SenseCAP Watcher">SenseCAP Watcher</a>
- <a href="https://www.bilibili.com/video/BV1BHJtz6E2S/" target="_blank" title="ESP-HI Low Cost Robot Dog">ESP-HI Low Cost Robot Dog</a>

<div style="display: flex; justify-content: space-between;">
  <a href="docs/v1/lichuang-s3.jpg" target="_blank" title="LiChuang ESP32-S3 Development Board">
    <img src="docs/v1/lichuang-s3.jpg" width="240" />
  </a>
  <a href="docs/v1/espbox3.jpg" target="_blank" title="Espressif ESP32-S3-BOX3">
    <img src="docs/v1/espbox3.jpg" width="240" />
  </a>
  <a href="docs/v1/m5cores3.jpg" target="_blank" title="M5Stack CoreS3">
    <img src="docs/v1/m5cores3.jpg" width="240" />
  </a>
  <a href="docs/v1/atoms3r.jpg" target="_blank" title="AtomS3R + Echo Base">
    <img src="docs/v1/atoms3r.jpg" width="240" />
  </a>
  <a href="docs/v1/magiclick.jpg" target="_blank" title="Magic Button 2.4">
    <img src="docs/v1/magiclick.jpg" width="240" />
  </a>
  <a href="docs/v1/waveshare.jpg" target="_blank" title="Waveshare ESP32-S3-Touch-AMOLED-1.8">
    <img src="docs/v1/waveshare.jpg" width="240" />
  </a>
  <a href="docs/v1/lilygo-t-circle-s3.jpg" target="_blank" title="LILYGO T-Circle-S3">
    <img src="docs/v1/lilygo-t-circle-s3.jpg" width="240" />
  </a>
  <a href="docs/v1/xmini-c3.jpg" target="_blank" title="XiaGe Mini C3">
    <img src="docs/v1/xmini-c3.jpg" width="240" />
  </a>
  <a href="docs/v1/movecall-cuican-esp32s3.jpg" target="_blank" title="CuiCan">
    <img src="docs/v1/movecall-cuican-esp32s3.jpg" width="240" />
  </a>
  <a href="docs/v1/wmnologo_xingzhi_1.54.jpg" target="_blank" title="WMnologo-Xingzhi-1.54">
    <img src="docs/v1/wmnologo_xingzhi_1.54.jpg" width="240" />
  </a>
  <a href="docs/v1/sensecap_watcher.jpg" target="_blank" title="SenseCAP Watcher">
    <img src="docs/v1/sensecap_watcher.jpg" width="240" />
  </a>
  <a href="docs/v1/esp-hi.jpg" target="_blank" title="ESP-HI Low Cost Robot Dog">
    <img src="docs/v1/esp-hi.jpg" width="240" />
  </a>
</div>

## 소프트웨어

### 펌웨어 플래싱

초보자는 개발 환경을 구성하지 않고 바로 플래싱할 수 있는 펌웨어 사용을 권장합니다.

펌웨어는 기본적으로 공식 [xiaozhi.me](https://xiaozhi.me) 서버에 연결됩니다. 개인 사용자는 계정을 등록하여 Qwen 실시간 모델을 무료로 사용할 수 있습니다.

### 개발 환경

- Cursor 또는 VSCode
- ESP-IDF 플러그인 설치. [ESP-IDF v6.0.2](https://github.com/espressif/esp-idf/releases/tag/v6.0.2)를 권장하며, v6.0 이상의 안정 버전을 사용하세요. ESP-IDF v5.5.2는 레거시 보드 호환성을 위해서만 유지됩니다
- 컴파일 속도와 드라이버 문제 측면에서 Linux가 Windows보다 낫습니다
- 이 프로젝트는 Google C++ 코드 스타일을 사용하므로 코드 제출 시 준수 여부를 확인하세요

### 개발자 문서

- [ESP-IDF 6.0 마이그레이션 가이드](docs/esp-idf-6-migration.md) - SDK 호환성, 컴포넌트 변경사항, 레거시 하드웨어 지원, 보드 검증 상태
- [커스텀 보드 가이드](docs/custom-board.md) - 커스텀 보드를 만드는 방법
- [MCP 프로토콜 IoT 제어 사용법](docs/mcp-usage.md) - MCP 프로토콜로 IoT 디바이스를 제어하는 방법
- [MCP 프로토콜 상호작용 흐름](docs/mcp-protocol.md) - 디바이스 측 MCP 프로토콜 구현
- [MQTT + UDP 하이브리드 통신 프로토콜 문서](docs/mqtt-udp.md)
- [상세한 WebSocket 통신 프로토콜 문서](docs/websocket.md)
- [Yana Wheelbot 보드](main/boards/yana-wheelbot) - 바퀴형 로봇 보드: 모터 도구, ToF 추락 방지 센서, LED, 팔/목 서보
- [Wheelbot 웹 컨트롤러](apps/controller-web) - 위 보드를 위한 브라우저 기반 컨트롤 패널

## 대형 모델 설정

공식 서버에 연결된 디바이스가 이미 있다면 [xiaozhi.me](https://xiaozhi.me) 콘솔에 로그인하여 설정할 수 있습니다.

## 관련 오픈소스 프로젝트

이 펌웨어는 XiaoZhi 통신 프로토콜을 사용합니다. 개인 PC에 서버를 배포하려면 다음의 서드파티 오픈소스 프로젝트들이 호환됩니다:

- [xinnan-tech/xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server) Python 서버
- [joey-zhou/xiaozhi-esp32-server-java](https://github.com/joey-zhou/xiaozhi-esp32-server-java) Java 서버
- [AnimeAIChat/xiaozhi-server-go](https://github.com/AnimeAIChat/xiaozhi-server-go) Golang 서버
- [hackers365/xiaozhi-esp32-server-golang](https://github.com/hackers365/xiaozhi-esp32-server-golang) Golang 서버

동일한 프로토콜을 사용하는 다른 클라이언트 프로젝트:

- [huangjunsen0406/py-xiaozhi](https://github.com/huangjunsen0406/py-xiaozhi) Python 클라이언트
- [TOM88812/xiaozhi-android-client](https://github.com/TOM88812/xiaozhi-android-client) Android 클라이언트
- [100askTeam/xiaozhi-linux](http://github.com/100askTeam/xiaozhi-linux) 100ask의 Linux 클라이언트
- [78/xiaozhi-sf32](https://github.com/78/xiaozhi-sf32) Sichuan의 블루투스 칩 펌웨어
- [QuecPython/solution-xiaozhiAI](https://github.com/QuecPython/solution-xiaozhiAI) Quectel의 QuecPython 펌웨어

커스텀 에셋 도구:

- [78/xiaozhi-assets-generator](https://github.com/78/xiaozhi-assets-generator) Custom Assets Generator (웨이크워드, 폰트, 이모지, 배경)

## 이 프로젝트에 대하여

Yana Robot은 MIT 라이선스로 배포되며, 상업적 용도를 포함하여 누구나 무료로 사용할 수 있습니다. LICENSE 파일은 해당 라이선스의 요구사항에 따라 업스트림 XiaoZhi 프로젝트의 원본 저작권 표시를 그대로 유지합니다.

원본 펌웨어, 프로토콜 설계, 보드 생태계는 [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)와 그 기여자들이 만들었습니다. 이 시점 이후 이 저장소에 있는 모든 것 — 새 보드, 버그 수정, 기능 — 은 업스트림 프로젝트와 더 이상 연관 없이 Yana Robot이라는 이름 아래 독립적으로 설계 및 유지보수됩니다.

아이디어나 제안이 있다면 이 저장소에 Issue를 열어주세요.

## Star 히스토리

<a href="https://star-history.com/#yanacuti1121/yana-robot&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date" />
 </picture>
</a>
