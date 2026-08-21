# Yana Robot

(Tiếng Việt | [English](README.md) | [한국어](README_ko.md))

## Tổng quan

Yana Robot là firmware trợ lý giọng nói dựa trên giao thức MCP, dành cho phần cứng ESP32. Nó kết hợp phát hiện từ đánh thức (wake word) ngoại tuyến, pipeline ASR/LLM/TTS dạng streaming, hai kênh truyền WebSocket và MQTT+UDP, cùng khả năng điều khiển thiết bị qua MCP (cả ở thiết bị lẫn trên cloud), trên nhiều biến thể board khác nhau.

<img src="docs/mcp-based-graph.jpg" alt="Điều khiển mọi thứ qua MCP" width="320">

## Nguồn gốc

Yana Robot được xây dựng dựa trên nền tảng của [XiaoZhi AI Chatbot](https://github.com/78/xiaozhi-esp32) — dự án firmware trợ lý giọng nói ESP32 mã nguồn mở gốc, do Xiaoqiang ([78](https://github.com/78)) và Shenzhen Xinzhi Future Technology Co., Ltd. tạo ra. Toàn bộ công lao cho codebase gốc, thiết kế giao thức và hệ sinh thái phần cứng thuộc về dự án đó và các contributor của họ.

Repo này là một nhánh phát triển độc lập: không liên kết và không đồng bộ với dự án gốc. Từ thời điểm này trở đi, board mới, bản sửa lỗi và tính năng trong repo này được thiết kế và duy trì riêng dưới tên Yana Robot.

## Cập nhật gần đây

- Nhánh chính hiện nhắm tới ESP-IDF v6.0 trở lên, ưu tiên bản ổn định v6.0.2. Bộ 157 biến thể release trước đó đã được kiểm chứng trên ESP-IDF v6.0.1; ma trận hiện tại có 171 biến thể, trong đó 170 hỗ trợ IDF 6.0.x và biến thể ESP32-S31 cần IDF 6.1 trở lên.
- Mã hóa MQTT và BluFi đã chuyển sang PSA Crypto. Việc tách component của IDF 6 và tương thích dependency bên thứ ba cũng đã được xử lý.
- Đã gia cố xử lý đồng thời trong audio pipeline, kiểm tra gói tin MQTT/UDP, và logic chọn ma trận release.
- ESP-IDF v5.5 chỉ còn giữ lại cho các board legacy đã được ghi chú rõ. ESP32-P4 Rev1 và Rev3 đều được hỗ trợ trên IDF 6 với ESP-SR 2.4.7; xem [Hướng dẫn di chuyển ESP-IDF 6.0](docs/esp-idf-6-migration.md) để biết chi tiết tương thích và trạng thái kiểm chứng board.

### Tính năng đã triển khai

- Wi-Fi, Ethernet có dây, USB RNDIS, và mạng 4G ML307/EC801E hoặc NT26 Cat.1; các board hỗ trợ có thể chuyển đổi giữa Wi-Fi và 4G
- Đánh thức bằng giọng nói ngoại tuyến với [ESP-SR](https://github.com/espressif/esp-sr), hỗ trợ tùy chỉnh từ đánh thức
- Hai kênh truyền: [WebSocket](docs/websocket.md) và [MQTT + UDP](docs/mqtt-udp.md)
- Streaming audio Opus với pipeline ASR + LLM + TTS truyền thống và cả mô hình giọng nói Realtime end-to-end; phần cứng có AEC hỗ trợ giao tiếp song công thời gian thực
- Nhận diện người nói hiện tại [3D Speaker](https://github.com/modelscope/3D-Speaker)
- Màn hình OLED / LCD với emoji và biểu cảm phong phú, cùng đầu vào hình ảnh từ camera trên các board hỗ trợ
- Hiển thị pin và quản lý nguồn điện
- 39 ngôn ngữ giao diện, ưu tiên giọng nói bản địa hóa khi có, fallback về tiếng Anh khi thiếu
- Nền tảng chip ESP32, ESP32-C3, ESP32-C5, ESP32-C6, ESP32-S3, và ESP32-P4
- Cấu hình Wi-Fi qua hotspot hoặc BluFi
- MCP phía thiết bị để điều khiển thiết bị (loa, đèn LED, servo, GPIO...)
- MCP phía cloud để mở rộng khả năng của mô hình lớn (điều khiển nhà thông minh, thao tác desktop PC, tìm kiếm tri thức, email...)
- Tùy chỉnh từ đánh thức, font chữ, emoji, và hình nền chat với công cụ chỉnh sửa online ([Custom Assets Generator](https://github.com/78/xiaozhi-assets-generator))

## Phần cứng

### Hỗ trợ 138 thư mục board và 171 biến thể release (danh sách một phần)

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

## Phần mềm

### Nạp firmware

Với người mới bắt đầu, nên dùng bản firmware có thể nạp trực tiếp mà không cần dựng môi trường phát triển.

Firmware mặc định kết nối tới server chính thức [xiaozhi.me](https://xiaozhi.me). Người dùng cá nhân có thể đăng ký tài khoản để dùng miễn phí mô hình Qwen realtime.

### Môi trường phát triển

- Cursor hoặc VSCode
- Cài plugin ESP-IDF. Ưu tiên [ESP-IDF v6.0.2](https://github.com/espressif/esp-idf/releases/tag/v6.0.2); dùng bản ổn định v6.0 trở lên. ESP-IDF v5.5.2 chỉ giữ lại để tương thích board cũ
- Linux tốt hơn Windows: build nhanh hơn và ít vấn đề driver hơn
- Dự án dùng chuẩn code style Google C++, đảm bảo tuân thủ khi submit code

### Tài liệu dành cho developer

- [Hướng dẫn di chuyển ESP-IDF 6.0](docs/esp-idf-6-migration.md) - Tương thích SDK, thay đổi component, hỗ trợ phần cứng cũ, trạng thái kiểm chứng board
- [Hướng dẫn tạo custom board](docs/custom-board.md) - Cách tạo board tùy chỉnh
- [Cách dùng MCP để điều khiển IoT](docs/mcp-usage.md) - Cách điều khiển thiết bị IoT qua giao thức MCP
- [Luồng tương tác giao thức MCP](docs/mcp-protocol.md) - Cách triển khai giao thức MCP phía thiết bị
- [Tài liệu giao thức truyền thông lai MQTT + UDP](docs/mqtt-udp.md)
- [Tài liệu chi tiết giao thức truyền thông WebSocket](docs/websocket.md)

## Cấu hình mô hình lớn

Nếu anh/chị đã có thiết bị kết nối tới server chính thức, có thể đăng nhập console [xiaozhi.me](https://xiaozhi.me) để cấu hình.

## Các dự án mã nguồn mở liên quan

Firmware này nói giao thức truyền thông XiaoZhi. Để deploy server trên máy cá nhân, các dự án mã nguồn mở bên thứ ba sau đây tương thích:

- [xinnan-tech/xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server) Server Python
- [joey-zhou/xiaozhi-esp32-server-java](https://github.com/joey-zhou/xiaozhi-esp32-server-java) Server Java
- [AnimeAIChat/xiaozhi-server-go](https://github.com/AnimeAIChat/xiaozhi-server-go) Server Golang
- [hackers365/xiaozhi-esp32-server-golang](https://github.com/hackers365/xiaozhi-esp32-server-golang) Server Golang

Các dự án client khác dùng chung giao thức:

- [huangjunsen0406/py-xiaozhi](https://github.com/huangjunsen0406/py-xiaozhi) Client Python
- [TOM88812/xiaozhi-android-client](https://github.com/TOM88812/xiaozhi-android-client) Client Android
- [100askTeam/xiaozhi-linux](http://github.com/100askTeam/xiaozhi-linux) Client Linux của 100ask
- [78/xiaozhi-sf32](https://github.com/78/xiaozhi-sf32) Firmware chip Bluetooth của Sichuan
- [QuecPython/solution-xiaozhiAI](https://github.com/QuecPython/solution-xiaozhiAI) Firmware QuecPython của Quectel

Công cụ Custom Assets:

- [78/xiaozhi-assets-generator](https://github.com/78/xiaozhi-assets-generator) Custom Assets Generator (từ đánh thức, font, emoji, background)

## Về dự án này

Yana Robot phát hành theo giấy phép MIT, cho phép bất kỳ ai sử dụng miễn phí, kể cả cho mục đích thương mại. File LICENSE giữ nguyên thông báo bản quyền gốc từ dự án XiaoZhi thượng nguồn, theo đúng yêu cầu của giấy phép đó.

Firmware gốc, thiết kế giao thức, và hệ sinh thái board được tạo ra bởi [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) và các contributor của họ. Mọi thứ trong repo này từ thời điểm này trở đi — board mới, sửa lỗi, tính năng — được thiết kế và duy trì độc lập dưới tên Yana Robot, không còn liên kết với dự án thượng nguồn.

Nếu có ý tưởng hay góp ý, hãy mở Issue trên repo này.

## Lịch sử Star

<a href="https://star-history.com/#yanacuti1121/yana-robot&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=yanacuti1121/yana-robot&type=Date" />
 </picture>
</a>
