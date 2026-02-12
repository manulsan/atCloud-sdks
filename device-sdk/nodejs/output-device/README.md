# atCloud365 Output Device - Node.js SDK

Node.js/TypeScript를 사용한 출력 디바이스(액추에이터) 예제 프로젝트입니다.

## 📋 기능

- 출력 제어 시뮬레이션
- WiFi/Ethernet 네트워크 연결
- HTTPS POST를 통한 디바이스 인증
- Socket.IO를 통한 실시간 명령 수신
- 개별/전체 출력 제어
- Blink 기능
- 상태 피드백 전송
- **sensorIds 지원** (V4 프로토콜)

## 🔧 요구사항

- **Node.js 16+**
- **npm** 또는 **yarn**

## 🚀 시작하기

### 1. Node.js 설치 확인

```bash
node --version
npm --version
```

### 2. 의존성 설치

```bash
npm install
```

### 3. 설정 파일 생성

`.env.example`을 `.env`로 복사 후 수정:

```bash
# Windows
copy .env.example .env

# Linux / macOS
cp .env.example .env
```

`.env` 수정:

```env
DEVICE_SN=03EB023C002601000000FF
CLIENT_SECRET_KEY=$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne

SERVER_URL=https://atcloud365.com
# SERVER_PORT=10020
API_PATH=/api/dev/io/

DEVICE_AUTH_URI=https://atcloud365.com/api/v3/devices/auth

SENSOR_COUNT=3
STATUS_REPORT_INTERVAL=60
BLINK_INTERVAL=500
```

### 4. 실행

#### 개발 모드 (TypeScript 직접 실행)

```bash
npm run dev
```

#### 개발 모드 (자동 재시작)

```bash
npm run watch
```

#### 프로덕션 모드

```bash
npm run build
npm start
```

## 📡 제어 명령

프로그램은 플랫폼으로부터 명령을 수신하여 출력을 제어합니다.

### 개별 출력 제어

```json
{
  "operation": {
    "customCmd": "output",
    "fieldIndex": 0,
    "fieldValue": 1
  }
}
```

### 전체 출력 제어

```json
{
  "operation": {
    "customCmd": "output-all",
    "fieldValue": 1
  }
}
```

### 출력 깜빡임

```json
{
  "operation": {
    "customCmd": "blinkLed",
    "fieldIndex": 0,
    "fieldValue": 5
  }
}
```

### 상태 동기화

```json
{
  "operation": {
    "customCmd": "sync"
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

`.env` 파일에서 변경:

```env
# 센서 개수 (출력 식별자로 사용)
SENSOR_COUNT=3

# 상태 보고 간격 (초)
STATUS_REPORT_INTERVAL=60

# 깜빡임 간격 (밀리초)
BLINK_INTERVAL=500
```

## 📚 코드 구조

```
output-device/
├── src/
│   └── index.ts         # 메인 프로그램
├── dist/                # 컴파일된 JavaScript
├── .env.example         # 환경변수 템플릿
├── .env                 # 실제 환경변수 (git ignore)
├── package.json
├── tsconfig.json
├── nodemon.json
├── README.md
└── .gitignore
```

## 💡 주요 기능

### HTTP POST 인증 (V4 프로토콜)

```typescript
const payload = {
  sn: DEVICE_SN,
  client_secret_key: CLIENT_SECRET_KEY,
  sensorIds: [0x0f1234, 0x0f1235, 0x0f1236]  // ⭐ 중요!
};
const response = await axios.post(DEVICE_AUTH_URI, payload);
```

### Socket.IO 연결 (sensorIds 포함)

```typescript
const socket = io(socketUrl, {
  path: API_PATH,
  auth: { token },
  query: {
    sn: DEVICE_SN,
    clientType: "device",
    sensorIds: JSON.stringify(sensorIds),  // ⭐ 중요!
    clientVersion: "V4"
  }
});
```

### app-cmd 처리

```typescript
socket.on("app-cmd", (data) => {
  const { customCmd, fieldIndex, fieldValue } = data.operation;
  
  if (customCmd === "output") {
    setOutputState(fieldIndex, fieldValue > 0);
  } else if (customCmd === "output-all") {
    setAllOutputs(fieldValue > 0);
  } else if (customCmd === "blinkLed") {
    setOutputBlink(fieldIndex, fieldValue);
  }
});
```

### 출력 상태 전송

```typescript
const content = outputPins.map(pin => pin.state ? 1 : 0);
socket.emit("dev-data", { content });
```

## 🐛 문제 해결

### 모듈을 찾을 수 없음

```bash
문제: Cannot find module 'socket.io-client'
해결: npm install
```

### 인증 실패

```bash
문제: Authentication failed
해결: .env에서 DEVICE_SN, CLIENT_SECRET_KEY 확인
     DEVICE_AUTH_URI 확인
     sensorIds 설정 확인
```

### Connection refused

```bash
문제: 서버에 연결 안됨
해결: SERVER_URL, SERVER_PORT 확인
     서버가 실행 중인지 확인
```

### 명령을 받지 못함

```bash
문제: app-cmd 이벤트가 수신되지 않음
해결: 디바이스가 올바른 room에 등록되었는지 확인
     sensorIds가 서버 설정과 일치하는지 확인
```

## 🔐 보안

- `.env` 파일은 Git에 커밋하지 마세요
- `.gitignore`에 자동으로 등록됨
- 인증 정보를 코드에 하드코딩하지 마세요

## 📞 지원

문제 발생 시:
1. `npm run dev`로 실행하여 상세 로그 확인
2. 서버 상태 확인
3. 네트워크 연결 확인

## 📄 라이센스

MIT License

---

**Happy Coding with Node.js! 🚀**
