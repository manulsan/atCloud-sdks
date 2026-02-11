# atCloud365 Input Device Example (Arduino IDE)

ESP32를 사용한 입력 디바이스(센서) 예제 프로젝트입니다. Arduino IDE로 개발합니다.

## 📋 기능

- ESP32 GPIO 입력 읽기 (3개 핀)
- WiFi 연결
- HTTPS를 통한 디바이스 인증
- Socket.IO를 통한 실시간 데이터 전송
- GPIO 상태 변경 감지 시 즉시 전송
- 변경 없을 시 1분 주기 전송

## 🔧 하드웨어 요구사항

- **ESP32 개발 보드** (ESP32-DevKit, ESP32-WROOM 등)
- **입력 센서/버튼** (3개)
  - GPIO 19, 21, 22
- **USB 케이블**

### 회로 연결

```
GPIO 19 ─── [Button/Sensor] ─── GND
GPIO 21 ─── [Button/Sensor] ─── GND
GPIO 22 ─── [Button/Sensor] ─── GND
```

> 💡 **참고**: GPIO 핀은 내부 풀업 저항으로 설정됩니다.

## 🚀 시작하기

### 1. Arduino IDE 설치 및 ESP32 보드 추가

**Arduino IDE 설치**
- [Arduino IDE 다운로드](https://www.arduino.cc/en/software)
- Arduino IDE 2.0 이상 권장

**ESP32 보드 추가**
1. Arduino IDE 실행
2. File → Preferences
3. "Additional Board Manager URLs"에 추가:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Tools → Board → Boards Manager
5. "esp32" 검색 후 "ESP32 by Espressif Systems" 설치

### 2. 라이브러리 설치

**필수 라이브러리**:
- **ArduinoJson** (by Benoit Blanchon)
- **WebSockets** (by Markus Sattler)

**설치 방법**:
1. Sketch → Include Library → Manage Libraries...
2. 검색 후 각각 설치:
   - "ArduinoJson" 버전 7.x
   - "WebSockets" 버전 2.x

### 3. 프로젝트 열기

```bash
# 저장소 클론
git clone https://github.com/your-org/atcloud365-sdks.git
```

Arduino IDE에서:
1. File → Open
2. `device-sdk/c/arduino-idf/input-device/input-device.ino` 선택

### 4. 설정 파일 생성

스케치 폴더에서 `config.example.h`를 복사하여 `config.h` 생성:

```bash
# Windows
copy config.example.h config.h

# Linux/Mac
cp config.example.h config.h
```

또는 Arduino IDE에서:
1. 프로젝트 열기
2. Sketch → Add File → `config.example.h` 복사본 만들기
3. 이름을 `config.h`로 변경

### 5. 인증 정보 입력

`config.h` 파일을 열어 수정:

```cpp
// WiFi 설정
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

// atCloud365 인증 정보
#define DEVICE_SN "03EB023C0026010000000F"
#define CLIENT_SECRET_KEY "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"
```

### 6. 보드 설정 및 업로드

**보드 선택**:
1. Tools → Board → ESP32 Arduino → ESP32 Dev Module

**포트 선택**:
1. Tools → Port → (ESP32가 연결된 COM 포트 선택)

**업로드**:
1. Sketch → Upload (또는 Ctrl+U)
2. 업로드 완료 대기

### 7. 시리얼 모니터로 확인

1. Tools → Serial Monitor (또는 Ctrl+Shift+M)
2. Baud rate: **115200** 선택
3. 로그 확인:

```
========================================
atCloud365 Input Device Example
Arduino IDE Version
========================================

[GPIO] Initializing input pins...
[WiFi] Connecting to Your_WiFi_SSID
...
[WiFi] Connected!
[AUTH] Token received successfully
[SOCKET] Connection info received
[STATUS] Emitted: Bootup & Ready
```

## 📡 통신 프로토콜

### Socket.IO 이벤트

#### 송신 (Emit)

| 이벤트 | 용도 | 데이터 형식 |
|--------|------|------------|
| `dev-data` | 센서 데이터 전송 | `{"content": [1, 0, 1]}` |
| `dev-status` | 디바이스 상태 | `"Bootup & Ready"` |

## 🔧 설정 옵션

`config.h`에서 설정 변경:

```cpp
// GPIO 핀 번호
#define GPIO_INPUT_1 19  // 다른 GPIO로 변경 가능
#define GPIO_INPUT_2 21
#define GPIO_INPUT_3 22

// 스캔 주기
#define GPIO_SCAN_INTERVAL 100  // 100ms

// 데이터 전송 주기
#define DATA_SEND_INTERVAL 60000  // 60초

// 디버그 출력
#define DEBUG_ENABLED 1  // 0: 비활성화
```

## 🐛 문제 해결

### 컴파일 에러: "config.h: No such file"

```
문제: config.h 파일을 찾을 수 없음
해결: config.example.h를 config.h로 복사
```

### 업로드 실패

```
문제: Failed to connect to ESP32
해결:
  - USB 케이블 확인
  - 올바른 COM 포트 선택
  - 업로드 중 ESP32의 BOOT 버튼 누르기
```

### WiFi 연결 실패

```
문제: WiFi에 연결되지 않음
해결:
  - SSID와 비밀번호 확인
  - 2.4GHz WiFi 사용 (ESP32는 5GHz 미지원)
```

### 라이브러리 에러

```
문제: WebSocketsClient.h: No such file
해결:
  - WebSockets 라이브러리 설치 확인
  - Arduino IDE 재시작
```

## 📚 코드 구조

```
input-device/
├── input-device.ino       # 메인 Arduino 스케치
├── config.example.h       # 설정 템플릿
├── config.h              # 실제 설정 (생성 필요)
├── README.md             # 이 문서
└── .gitignore
```

### Arduino 스케치 구조

- `setup()`: 초기화 함수
  - GPIO 설정
  - WiFi 연결
  - HTTPS 인증
  - Socket.IO 연결
  
- `loop()`: 메인 루프
  - WebSocket 이벤트 처리
  - GPIO 스캔
  - 데이터 전송

## 💡 Arduino IDE vs PlatformIO

이 프로젝트는 Arduino IDE와 PlatformIO 모두에서 사용 가능합니다:

- **Arduino IDE**: 초보자 친화적, 간단한 설치
- **PlatformIO**: 고급 기능, 빠른 컴파일, 프로젝트 관리 우수

동일한 코드베이스를 공유하므로 원하는 환경을 선택하세요.

## 🔐 보안

- `config.h` 파일은 절대 Git에 커밋하지 마세요
- `.gitignore`에 자동으로 제외되어 있습니다

## 🤝 기여하기

버그 리포트, 기능 제안 환영합니다!

## 📄 라이센스

MIT License

---

**Happy Coding with Arduino! 🚀**
