


## 1. Sample Project
### InputOutput
GPIO control with atCloud
- file "defs.h"
```
	// #define _IO_CONTROL_
	#ifdef _IO_CONTROL_
	#define _BULB_CONTROL_
	#endif
	#define DEBUG

/**************************************************************
    #if defined ( !_IO_CONTROL_) 
        then  it works sensor readers
    #if (defined _IO_CONTROL_ && ! defined _BULB_CONTROL_) 
        then code works generic IO operations
    #if (defined _IO_CONTROL_ && defined _BULB_CONTROL_)  
        then  code works bulb controller with interrupts
**************************************************************/
```

### SensorReader
- Sensor read data with atCloud
- for easy understanings

---
## 2. Install ESP board lib
- http://arduino.esp8266.com/stable/package_esp8266com_index.json,https://dl.espressif.com/dl/package_esp32_index.json

---
## 3. Install Library
- ArduinoJson
- NTPClient
- WebSockets_Generic