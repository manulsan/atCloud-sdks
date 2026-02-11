# atCloud365 Input Device - Python SDK

Python을 사용한 입력 디바이스(센서) 예제 프로젝트입니다. Raspberry Pi, PC 등에서 실행할 수 있습니다.

## 📋 기능

- 센서 데이터 시뮬레이션 (키보드 입력)
- WiFi/Ethernet 네트워크 연결
- HTTPS POST를 통한 디바이스 인증
- Socket.IO를 통한 실시간 통신
- 센서 데이터 전송 (dev-data)
- 상태 보고 (dev-status)
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

### 3. 의존성 설치

```bash
pip install -r requirements.txt
```

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
DEVICE_SN = "03EB023C0026010000000F"
CLIENT_SECRET_KEY = "$2b$10$MTQ9AXjbWxckfbCPzVDpkOtpRrSP2z.KyRhtPvhVuaAcmyBiPZXne"

SERVER_URL = "http://localhost"
SERVER_PORT = 10020
DEVICE_AUTH_URI = "http://localhost:10030/api/v3/devices/auth"

# 센서 ID 배열 (중요!)
SENSOR_IDS = [0x0f1234, 0x0f1235, 0x0f1236]
```

### 5. 실행

```bash
python main.py
```

## 📡 사용법

### 센서 데이터 입력

프로그램 실행 후 `index,value` 형식으로 입력:

```
Enter sensor value (index,value): 0,100
[INPUT] ✅ Sensor 0 set to 100
[INPUT] Current values: [100, 0, 0]

Enter sensor value (index,value): 1,50
[INPUT] ✅ Sensor 1 set to 50
[INPUT] Current values: [100, 50, 0]
```

### 종료

- `quit` 또는 `exit` 입력
- `Ctrl+C` 키 입력

## 🔧 설정 옵션

`config.py`에서 변경:

```python
# 센서 ID 배열 (16진수)
SENSOR_IDS = [0x0f1234, 0x0f1235]

# 데이터 업로드 간격 (초)
DATA_UPLOAD_INTERVAL = 10

# 상태 보고 간격 (초)
STATUS_REPORT_INTERVAL = 60

# 디버그 모드
DEBUG_ENABLED = True
```

## 📚 코드 구조

```
input-device/
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

### 데이터 전송

```python
payload = {"content": [100, 50, 25]}
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

## 🔐 보안

- `config.py` 파일은 Git에 커밋하지 마세요
- `.gitignore`에 자동으로 등록됨
- 인증 정보를 코드에 하드코딩하지 마세요

## 📞 지원

문제 발생 시:
1. 디버그 모드 활성화: `DEBUG_ENABLED = True`
2. 로그 확인
3. 서버 상태 확인

## 📄 라이센스

MIT License

---

**Happy Coding with Python! 🐍**
