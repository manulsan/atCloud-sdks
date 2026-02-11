# atCloud365 Output Device Example (PlatformIO)

ESP32를 사용한 출력 디바이스(액추에이터) 예제 프로젝트입니다. atCloud365 플랫폼으로부터 제어 명령을 받아 GPIO 출력을 제어합니다.

## 📋 기능

- ESP32 GPIO 출력 제어 (3개 핀)
- WiFi 연결
- HTTPS를 통한 디바이스 인증
- Socket.IO를 통한 실시간 명령 수신
- 개별 GPIO On/Off 제어
- 전체 GPIO 일괄 제어
- GPIO Blink 기능
- 상태 변경 시 플랫폼에 피드백 전송

## 🔧 하드웨어 요구사항

- **ESP32 개발 보드** (ESP32-DevKit, ESP32-WROOM 등)
- **출력 장치** (3개)
  - 릴레이 모듈
  - LED
  - 모터 드라이버
  - 솔레노이드 등
- **USB 케이블** (프로그래밍 및 전원용)

### 회로 연결

```
        ESP32                    Relay Module
GPIO 19 ────────────────────── IN1
GPIO 21 ────────────────────── IN2
GPIO 22 ────────────────────── IN3
GND ────────────────────────── GND
5V  ────────────────────────── VCC
```

> ⚠️ **주의**: 큰 전류를 소비하는 장치는 릴레이를 통해 연결하세요. ESP32 GPIO는 최대 40mA까지만 공급 가능합니다.

## 🚀 시작하기

### 1. 개발 환경 설정

**PlatformIO 설치**

- [VS Code](https://code.visualstudio.com/) 설치
- VS Code에서 PlatformIO IDE 확장 프로그램 설치

### 2. 프로젝트 클론 및 설정

```bash
# 저장소 클론
git clone https://github.com/your-org/atcloud365-sdks.git
cd atcloud365-sdks/device-sdk/c/platformio/output-device
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
#define DEVICE_SN "03EB023C0026010000000F"  // atCloud365에서 발급받은 시리얼 번호
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"  // 비밀 키
```

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
atCloud365 Output Device Example
========================================

[GPIO] Initializing output pins...
  GPIO 19: OFF
  GPIO 21: OFF
  GPIO 22: OFF
[WiFi] Connecting to Your_WiFi_SSID
.....
[WiFi] Connected!
[AUTH] Authenticating with atCloud365...
[AUTH] Token received successfully
[SOCKET] Connecting to Socket.IO...
[SOCKET] Connection info received
[STATUS] Emitted: Bootup & Ready
[CMD] Processing command: {"operation":{"fieldIndex":0,"fieldValue":1}}
[GPIO] Pin 19 set to ON
[DATA] Emitted: 42["dev-data",{"content":[1,0,0]}]
```

## 📡 통신 프로토콜

### Socket.IO 이벤트

#### 수신 (On)

| 이벤트 | 용도 | 데이터 형식 | 설명 |
|--------|------|------------|------|
| `app-cmd` | 제어 명령 수신 | JSON | 플랫폼에서 전송한 제어 명령 |

**명령 데이터 형식**:

```json
{
  "operation": {
    "fieldIndex": 0,     // GPIO 인덱스 (0, 1, 2)
    "fieldValue": 1,     // 값 (0=OFF, 1=ON)
    "customCmd": ""      // 커스텀 명령 (선택)
  }
}
```

#### 송신 (Emit)

| 이벤트 | 용도 | 데이터 형식 |
|--------|------|------------|
| `dev-data` | 현재 상태 전송 | `{"content": [1, 0, 1]}` |
| `dev-status` | 디바이스 상태 | `"Bootup & Ready"` |

**상태 전송 조건**:
- GPIO 상태 변경 시 즉시 전송
- 60초마다 주기적 전송

## 🎮 제어 명령

### 1. 개별 GPIO 제어

**단일 핀 On/Off**:
```json
{
  "operation": {
    "fieldIndex": 0,
    "fieldValue": 1
  }
}
```
- `fieldIndex`: 0, 1, 2 (GPIO 인덱스)
- `fieldValue`: 0 (OFF), 1 (ON)

### 2. 커스텀 명령

#### output (개별 제어)
```json
{
  "operation": {
    "customCmd": "output",
    "fieldIndex": 1,
    "fieldValue": 1
  }
}
```

#### output-all (전체 제어)
```json
{
  "operation": {
    "customCmd": "output-all",
    "fieldValue": 1
  }
}
```
모든 GPIO를 동시에 On/Off

#### blinkLed (깜빡임)
```json
{
  "operation": {
    "customCmd": "blinkLed",
    "fieldIndex": 0,
    "fieldValue": 5
  }
}
```
지정된 GPIO를 5회 깜빡임 (500ms 간격)

#### sync (상태 동기화)
```json
{
  "operation": {
    "customCmd": "sync"
  }
}
```
현재 상태를 즉시 플랫폼에 전송

#### reboot (재부팅)
```json
{
  "operation": {
    "customCmd": "reboot"
  }
}
```
디바이스 재부팅

## 🔧 설정 옵션

`include/config.h`에서 다양한 설정을 변경할 수 있습니다:

```cpp
// GPIO 핀 번호 변경
#define GPIO_OUTPUT_1 19  // 다른 핀으로 변경 가능
#define GPIO_OUTPUT_2 21
#define GPIO_OUTPUT_3 22

// 상태 보고 주기 (밀리초)
#define STATUS_REPORT_INTERVAL 60000  // 60초

// 깜빡임 간격 (밀리초)
#define BLINK_INTERVAL 500  // 500ms

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

### 명령이 작동하지 않음
```
문제: app-cmd를 받아도 GPIO가 동작하지 않음
해결:
  - 하드웨어 연결 확인
  - 릴레이 모듈의 전원 확인
  - 시리얼 모니터에서 명령 수신 확인
  - JSON 형식 확인
```

### GPIO 제어 불안정
```
문제: GPIO가 예상과 다르게 동작
해결:
  - GPIO 핀 번호 확인
  - 릴레이 모듈의 Trigger 타입 확인 (High/Low)
  - 전원 공급 안정성 확인
```

## 📚 코드 구조

```
output-device/
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
- `loop()`: 메인 루프 (명령 수신, 깜빡임 처리, 상태 보고)
- `processAppCmd()`: 제어 명령 처리
- `setState()`: 개별 GPIO 제어
- `setStateAll()`: 전체 GPIO 제어
- `setStateBlink()`: GPIO 깜빡임 설정
- `handleBlinkLogic()`: 깜빡임 로직 처리

## 💡 활용 예시

### 1. 스마트 홈 제어
- 조명 제어 (릴레이)
- 팬/에어컨 제어
- 도어락 제어

### 2. 자동화 시스템
- 관개 시스템 (솔레노이드 밸브)
- 창문 개폐 (모터)
- 경보 시스템 (사이렌, 경광등)

### 3. 원격 모니터링
- 상태 LED 표시
- 경고 알람 출력

## 🔐 보안 고려사항

- `config.h` 파일은 절대 Git에 커밋하지 마세요
- HTTPS/TLS로 안전하게 통신합니다
- 민감한 데이터는 환경 변수로 관리하세요

## 🤝 기여하기

버그 리포트, 기능 제안, Pull Request를 환영합니다!

## 📄 라이센스

MIT License - 자유롭게 사용 및 수정 가능

---

**Happy Coding! 🚀**
