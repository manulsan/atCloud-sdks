# atCloud365 Input Device Example (PlatformIO)

ESP32를 사용한 입력 디바이스(센서) 예제 프로젝트입니다. GPIO 입력 상태를 읽어 atCloud365 플랫폼으로 실시간 전송합니다.

## 📋 기능

- ESP32 GPIO 입력 읽기 (3개 핀)
- WiFi 연결
- HTTPS를 통한 디바이스 인증
- Socket.IO를 통한 실시간 데이터 전송
- GPIO 상태 변경 감지 시 즉시 전송
- 변경 없을 시 1분 주기 전송

## 🧭 코딩 스타일 (Coding style)
- **PlatformIO 프로젝트:** C++ 스타일(`.cpp`, 클래스 사용 허용). 모듈화된 C++ 코드, RAII/클래스 캡슐화 권장.
- **목표:** 가독성 높은 C++ 코드, `.cpp` / `.h` 파일 구조 유지, `include/config.h`로 구성값 분리.

> 예: 플랫폼별 구현은 `*.cpp`에서 작성하고, 하드웨어 설정 및 매크로는 `include/config.h`에서 관리하세요.

## 🔧 하드웨어 요구사항


- **ESP32 개발 보드** (ESP32-DevKit, ESP32-WROOM 등)
- **입력 센서/버튼** (3개)
  - GPIO 32
  - GPIO 33
  - GPIO 25
- **LCD 모듈 (ST7789P3, 240x320)** — 기본 핀맵은 아래 참조
- **USB 케이블** (프로그래밍 및 전원용)

### 회로 연결

```
GPIO 32 ─── [Button/Sensor] ─── GND
GPIO 33 ─── [Button/Sensor] ─── GND
GPIO 25 ─── [Button/Sensor] ─── GND
```

### LCD (ST7789P3) — 기본 핀맵 (모듈: ESP32-32E-7789)
- `LCD_SCK_PIN` = IO14  (SPI SCK)
- `LCD_MOSI_PIN` = IO13 (SPI MOSI)
- `LCD_MISO_PIN` = IO12 (SPI MISO)
- `LCD_CS_PIN`   = IO15  (Chip Select)
- `LCD_DC_PIN`   = IO2   (Data/Command)
- `LCD_RST_PIN`  = IO22  (Reset, 모듈 리셋 공유)
- `LCD_BL_PIN`   = IO21  (Backlight PWM)

> 참조: https://www.alibaba.com/product-detail/Support-NerdMiner-V2-ESP32-2-8_1601486878318.html?spm=a2700.prosearch.normal_offer.d_image.1f6967af2tr1Gl&priceId=7e207ed910ad4a4aabf9b7327edcf9f4
 
> 위 값은 `include/config.h`의 `LCD_*` 매크로에 반영되어 있습니다.

![alt text](image.png)

> 💡 **참고**: GPIO 핀은 내부 풀업 저항으로 설정됩니다. 버튼을 누르면 LOW(0), 떼면 HIGH(1)

## 🚀 시작하기

### 1. 개발 환경 설정

**PlatformIO 설치**

- [VS Code](https://code.visualstudio.com/) 설치
- VS Code에서 PlatformIO IDE 확장 프로그램 설치

### 2. 프로젝트 클론 및 설정

```bash
# 저장소 클론
git clone https://github.com/your-org/atcloud365-sdks.git
cd atcloud365-sdks/device-sdk/c/platformio/input-device
```

### 3. 설정 파일 생성

`include` 폴더에서 `config.example.h`를 복사하여 `config.h` 생성:

```bash
# Windows
copy include\config.example.h include\config.h

# Linux/Mac
cp include/config.example.h include/config.h
```

### 4. 인증 정보 입력

`include/config.h` 파일을 열어 다음 정보를 입력:

```cpp
// WiFi 설정
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

// atCloud365 인증 정보
#define DEVICE_SN "03EB023C002601000000FC"  // atCloud365에서 발급받은 시리얼 번호
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"  // 비밀 키
```

> ⚠️ **주의**: `config.h` 파일은 `.gitignore`에 등록되어 있으므로 Git에 커밋되지 않습니다.

### 5. 빌드 및 업로드

VS Code에서:

1. PlatformIO 탭 열기
2. "Build" 클릭하여 컴파일
3. ESP32를 USB로 연결
4. "Upload" 클릭하여 업로드

또는 터미널에서:

```bash
# 빌드
pio run

# 업로드
pio run -t upload

# 시리얼 모니터
pio device monitor
```

### 6. 동작 확인

시리얼 모니터(115200 baud)에서 다음과 같은 로그를 확인:

```
========================================
atCloud365 Input Device Example
========================================

[GPIO] Initializing input pins...
  GPIO 19: 1
  GPIO 21: 1
  GPIO 22: 1
[WiFi] Connecting to Your_WiFi_SSID
.....
[WiFi] Connected!
[WiFi] IP Address: 192.168.1.100
[AUTH] Authenticating with atCloud365...
[AUTH] Token received successfully
[SOCKET] Connecting to Socket.IO...
[SOCKET] WebSocket connected
[SOCKET] Connection info received
[SOCKET] SID: abc123xyz...
[STATUS] Emitted: Bootup & Ready
[GPIO] Pin 19 changed: 1 -> 0
[DATA] Emitted: 42["dev-data",{"content":[1,0,0]}]
```

## 📡 통신 프로토콜

### 인증 플로우

```
1. ESP32 Boot → WiFi 연결
2. HTTPS POST /api/v3/devices/auth with JSON body: {"sn":"<sn>", "client_secret_key":"<secret>", "sensorIds":[0x0f1234,0x0f1235,...]} (sensorIds are generated from BASE_SENSOR_ID and SENSOR_COUNT in `include/config.h`)
3. Server Response: {"token": "..."}
4. Socket.IO 연결 (token 포함)
5. 실시간 데이터 통신 시작
```

### Socket.IO 이벤트

#### 송신 (Emit)

| 이벤트 | 용도 | 데이터 형식 |
|--------|------|------------|
| `dev-data` | 센서 데이터 전송 | `{"content": [1, 0, 1]}` |
| `dev-status` | 디바이스 상태 전송 | `"Bootup & Ready"` |

**데이터 전송 조건**:
- GPIO 상태 변경 감지 시 즉시 전송
- 변경 없을 경우 60초마다 주기적 전송

#### 수신 (On)

Input Device는 주로 데이터를 전송하며, 서버로부터의 `connected` 확인 이벤트를 수신합니다.

## 🔧 설정 옵션

`include/config.h`에서 다양한 설정을 변경할 수 있습니다:

```cpp
// GPIO 핀 번호 변경 (기본값이 LCD 신호와 겹치지 않도록 업데이트됨)
#define GPIO_INPUT_1 32  // 다른 핀으로 변경 가능
#define GPIO_INPUT_2 33
#define GPIO_INPUT_3 25

// 스캔 주기 (밀리초)
#define GPIO_SCAN_INTERVAL 100  // GPIO 상태 체크 간격

// 데이터 전송 주기 (밀리초)
#define DATA_SEND_INTERVAL 60000  // 60초

// 디버그 출력 활성화/비활성화
#define DEBUG_ENABLED 1  // 0으로 설정 시 시리얼 출력 비활성화
```

## 🐛 문제 해결

### WiFi 연결 실패

```
문제: WiFi에 연결되지 않음
해결: 
  - SSID와 비밀번호 확인
  - 2.4GHz WiFi인지 확인 (ESP32는 5GHz 미지원)
  - 라우터와의 거리 확인
```

### 인증 실패

```
문제: "[AUTH] HTTP Error: 401"
해결:
  - DEVICE_SN 확인
  - CLIENT_SECRET_KEY 확인
  - 서버 URL 확인 (https://atcloud365.com)
```

### Socket.IO 연결 끊김

```
문제: 연결이 자주 끊어짐
해결:
  - WiFi 신호 강도 확인
  - 라우터 재부팅
  - HTTP_TIMEOUT 값 증가
```

### GPIO 읽기 오류

```
문제: GPIO 상태가 정확하지 않음
해결:
  - 회로 연결 확인
  - 풀업 저항 확인
  - 디바운스 로직 추가 필요 시 코드 수정
```

## 📚 코드 구조

```
input-device/
├── platformio.ini          # PlatformIO 설정
├── include/
│   ├── config.example.h   # 설정 템플릿
│   └── config.h           # 실제 설정 (생성 필요, gitignore됨)
├── src/
│   └── main.cpp           # 메인 소스 코드
└── README.md              # 이 문서
```

### 주요 함수

- `setup()`: 초기화 (WiFi, GPIO, 인증, Socket.IO)
- `loop()`: 메인 루프 (GPIO 스캔, 데이터 전송)
- `setupWiFi()`: WiFi 연결
- `authenticateDevice()`: HTTPS 인증
- `connectSocketIO()`: Socket.IO 연결
- `scanGpioInputs()`: GPIO 입력 읽기
- `emitDevData()`: 센서 데이터 전송

## 🔐 보안 고려사항

- `config.h` 파일은 절대 Git에 커밋하지 마세요
- 프로덕션 환경에서는 HTTPS/TLS만 사용하세요
- 디바이스 시리얼 번호와 비밀 키를 안전하게 보관하세요
- 가능하면 환경 변수나 보안 저장소를 사용하세요

## 🤝 기여하기

버그 리포트, 기능 제안, Pull Request를 환영합니다!

## 📄 라이센스

MIT License - 자유롭게 사용 및 수정 가능

## 📞 지원

- GitHub Issues: [이슈 등록](https://github.com/your-org/atcloud365-sdks/issues)
- 공식 웹사이트: [atCloud365.com](https://atcloud365.com)

---

**Happy Coding! 🚀**
