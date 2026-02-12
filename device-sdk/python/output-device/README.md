# atCloud365 Output Device - Python SDK

Python을 사용한 출력 디바이스(액추에이터) 예제 프로젝트입니다. Raspberry Pi, PC 등에서 실행할 수 있습니다.

## 📋 기능

- 출력 제어 시뮬레이션 (GPIO/Relay)
- WiFi/Ethernet 네트워크 연결
- HTTPS POST를 통한 디바이스 인증
- Socket.IO를 통한 실시간 명령 수신
- 개별/전체 출력 제어
- Blink 기능
- 상태 피드백 전송
- **sensorIds 지원** (V4 프로토콜)

## 🔧 요구사항

- **Python 3.7+**
- **pip** (패키지 관리자)
- **네트워크 연결**

## 🚀 시작하기

### 1. Python 설치 확인

```bash
python --version
# 또는
python3 --version
```

### 2. 가상 환경 생성 (권장)

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# Linux / macOS
python3 -m venv venv
source venv/bin/activate
```

> 자동 설치 스크립트 사용(권장):
>
> - Linux/macOS: `./install.sh`
> - Windows (PowerShell): `.\install.ps1` (PowerShell 실행정책에 따라 `Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned` 필요할 수 있음)
> - Windows (cmd): `install.bat`

### 3. 의존성 설치

```bash
pip install -r requirements.txt
```

> 참고: 서버에서 WebSocket 전송을 요구하는 경우 `websocket-client` 패키지가 필요합니다. `pip install websocket-client` 또는 `requirements.txt`에 포함되어 있는지 확인하세요.

### 4. 설정 파일 생성

`config.example.py`를 `config.py`로 복사 후 수정:

```bash
# Windows
copy config.example.py config.py

# Linux / macOS
cp config.example.py config.py
```

`config.py` 수정:

```python
DEVICE_SN = "03EB023C002601000000FF"
CLIENT_SECRET_KEY = "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"

SERVER_URL = "https://atcloud365.com"
# SERVER_PORT = 10020
DEVICE_AUTH_URI = "https://atcloud365.com/api/v3/devices/auth"

# 센서 ID 배열 (출력 식별자로 사용)
SENSOR_IDS = [0x0f1234, 0x0f1235, 0x0f1236]
```

### 5. 실행

```bash
python main.py
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

`config.py`에서 변경:

```python
# 센서 ID 배열 (출력 식별자)
SENSOR_IDS = [0x0f1234, 0x0f1235]

# 상태 보고 간격 (초)
STATUS_REPORT_INTERVAL = 60

# 깜빡임 간격 (초)
BLINK_INTERVAL = 0.5

# 디버그 모드
DEBUG_ENABLED = True
```

## 📚 코드 구조

```
output-device/
├── main.py              # 메인 프로그램
├── config.example.py    # 설정 템플릿
├── config.py           # 실제 설정 (git ignore)
├── requirements.txt    # Python 의존성
├── README.md
└── .gitignore
```

## 💡 주요 기능

### HTTP POST 인증 (V4 프로토콜)

```python
payload = {
    "sn": DEVICE_SN,
    "client_secret_key": CLIENT_SECRET_KEY,
    "sensorIds": SENSOR_IDS  # ⭐ 중요!
}
response = requests.post(DEVICE_AUTH_URI, json=payload)
```

### Socket.IO 연결 (sensorIds 포함)

```python
sio.connect(
    socket_url,
    auth={"token": auth_token},
    query={
        "sn": DEVICE_SN,
        "clientType": "device",
        "sensorIds": json.dumps(SENSOR_IDS),  # ⭐ 중요!
        "clientVersion": "V4"
    }
)
```

### app-cmd 처리

```python
@sio.on('app-cmd')
def handle_app_cmd(data):
    operation = data["operation"]
    custom_cmd = operation.get("customCmd", "")
    field_index = operation.get("fieldIndex", -1)
    field_value = operation.get("fieldValue", -1)
    # Process command...
```

### 출력 상태 전송

```python
content = [1 if pin.state else 0 for pin in output_pins]
payload = {"content": content}
sio.emit("dev-data", payload)
```

## 🐛 문제 해결

### ModuleNotFoundError

```bash
문제: socketio 모듈 없음
해결: pip install -r requirements.txt
```

### 인증 실패

```bash
문제: Authentication failed
해결: config.py에서 DEVICE_SN, CLIENT_SECRET_KEY 확인
     SENSOR_IDS 설정 확인
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

- `config.py` 파일은 Git에 커밋하지 마세요
- `.gitignore`에 자동으로 등록됨
- 인증 정보를 코드에 하드코딩하지 마세요

## 🎯 실제 GPIO 제어

Raspberry Pi에서 실제 GPIO를 제어하려면:

```python
# RPi.GPIO 라이브러리 설치
pip install RPi.GPIO

# main.py에서 set_output_state() 함수 수정
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.OUT)  # GPIO 17번 핀

def set_output_state(index, state):
    # 실제 GPIO 제어
    GPIO.output(17, GPIO.HIGH if state else GPIO.LOW)
```

## 📞 지원

문제 발생 시:
1. 디버그 모드 활성화: `DEBUG_ENABLED = True`
2. 로그 확인
3. 서버 상태 확인

## 📄 라이센스

MIT License

---

**Happy Coding with Python! 🐍**
