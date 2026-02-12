# atCloud365 Output Device Example (Arduino IDE)

ESP32를 사용한 출력 디바이스(액추에이터) 예제 프로젝트입니다. Arduino IDE로 개발합니다.

## 📋 기능

- ESP32 GPIO 출력 제어 (3개 핀)
- WiFi 연결
- HTTPS를 통한 디바이스 인증
- Socket.IO를 통한 실시간 명령 수신
- 개별/전체 GPIO 제어
- GPIO Blink 기능
- 상태 피드백 전송

## 🔧 하드웨어 요구사항

- **ESP32 개발 보드**
- **출력 장치** (릴레이, LED 등)
  - GPIO 19, 21, 22
- **USB 케이블**

### 회로 연결

```
        ESP32                    Relay Module
GPIO 19 ────────────────────── IN1
GPIO 21 ────────────────────── IN2
GPIO 22 ────────────────────── IN3
GND ────────────────────────── GND
5V  ────────────────────────── VCC
```

## 🚀 시작하기

### 1. Arduino IDE 설치 및 ESP32 보드 추가

**Arduino IDE 설치**
- [Arduino IDE 다운로드](https://www.arduino.cc/en/software)

**ESP32 보드 추가**
1. File → Preferences
2. "Additional Board Manager URLs"에 추가:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager → "esp32" 설치

### 2. 라이브러리 설치

Sketch → Include Library → Manage Libraries에서 설치:
- **ArduinoJson** 버전 7.x
- **WebSockets** 버전 2.x

### 3. 프로젝트 열기

Arduino IDE에서 `output-device.ino` 파일 열기

### 4. 설정 파일 생성

`config.example.h`를 `config.h`로 복사 후 수정:

```cpp
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define DEVICE_SN "03EB023C002601000000FC"
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"
```

참고: `config.example.h`에 `BASE_SENSOR_ID` 및 `SENSOR_COUNT`가 추가되어 있으며, 장치는 인증 시 다음과 같은 POST JSON을 전송합니다:

```json
{"sn":"<DEVICE_SN>","client_secret_key":"<SECRET>","sensorIds":[0x0f1234,0x0f1235,...]}
```

## 🧭 코딩 스타일 (Coding style)
- **Arduino‑IDE / Arduino‑ESP32 (Arduino‑IDF) 프로젝트:** C 스타일 기반으로 코딩합니다 (`.ino` / `.c` / `.h`).
- **권장:** 단순·명확한 절차적 코드, 헤더 가드 사용, 설정은 `config.h`에 보관합니다.

> 참고: 복잡한 로직은 `.c/.h` 모듈로 분리해 테스트와 재사용성을 높이세요.

`BASE_SENSOR_ID`/`SENSOR_COUNT`를 변경하면 전송되는 `sensorIds` 배열이 바뀝니다.

### 5. 업로드

1. Tools → Board → ESP32 Dev Module
2. Tools → Port → (COM 포트 선택)
3. Sketch → Upload

### 6. 시리얼 모니터

Tools → Serial Monitor (115200 baud)

## 📡 제어 명령

### 개별 GPIO 제어
```json
{
  "operation": {
    "fieldIndex": 0,
    "fieldValue": 1
  }
}
```

### 전체 GPIO 제어
```json
{
  "operation": {
    "customCmd": "output-all",
    "fieldValue": 1
  }
}
```

### GPIO 깜빡임
```json
{
  "operation": {
    "customCmd": "blinkLed",
    "fieldIndex": 0,
    "fieldValue": 5
  }
}
```

### 재부팅
```json
{
  "operation": {
    "customCmd": "reboot"
  }
}
```

## 🔧 설정 옵션

`config.h`에서 변경:

```cpp
// GPIO 핀
#define GPIO_OUTPUT_1 19
#define GPIO_OUTPUT_2 21
#define GPIO_OUTPUT_3 22

// 상태 보고 주기
#define STATUS_REPORT_INTERVAL 60000  // 60초

// 깜빡임 간격
#define BLINK_INTERVAL 500  // 500ms

// 디버그
#define DEBUG_ENABLED 1
```

## 🐛 문제 해결

### 컴파일 에러
```
문제: config.h 없음
해결: config.example.h를 config.h로 복사
```

### 업로드 실패
```
문제: 업로드 안됨
해결: 올바른 COM 포트 선택, BOOT 버튼 누르기
```

### GPIO 제어 안됨
```
문제: 릴레이가 작동하지 않음
해결: 하드웨어 연결 확인, 전원 확인
```

## 📚 코드 구조

```
output-device/
├── output-device.ino      # 메인 스케치
├── config.example.h       # 설정 템플릿
├── config.h              # 실제 설정
├── README.md
└── .gitignore
```

## 💡 주요 함수

- `setup()`: 초기화
- `loop()`: 메인 루프
- `processAppCmd()`: 명령 처리
- `setState()`: GPIO 제어
- `handleBlinkLogic()`: 깜빡임 처리

## 🔐 보안

`config.h` 파일은 Git에 커밋하지 마세요 (.gitignore에 등록됨)

## 📄 라이센스

MIT License

---

**Happy Coding with Arduino! 🚀**
