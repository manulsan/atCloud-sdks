# atCloud
By generalizing IoT devices, all devices are managed as a single unit object. Devices are managed with the same method, and the view widget is different according to the user's viewing method.

## Live Demo : https://damosys.com:9000
---
## Basic network
<img src="./assets/img/main-1.png">

---
## IoT Devices

 Devices are defined in three types as follows. the following combination, one device have 10 10 pieces of information which can be gathered.
### Gathering Devices  
It gathers input data and publish to the platforms
  ```
  Input signal sensing( 0 or , max 10 inputs)    
  Sensor input( max 10 sensors, ex> Temperature, Humidity, etc...)
  ``` 

### Output device
It subscribe the commands from platform and issues event at the devices like Realy controller or signal controller
```
Output signal( Relay Control, max 10 inputs)
```
### GPS Tracker
It gather GPS data and publish to platform and is used as assets tracking device
```
GPS data
```
### CCTV Streaming channel
It is working based on out "Media-Server" that does streaming to somewhere. It is somehow different from above

---

## Device Data Protocol
The device uploads data in a standardized format. The following syntax applies. It is designed to upload "DATA" and "STATUS OF DEVICE" <br>
### Protocol Syntax
- [number1,...,number-10], number can be "integer" and "float" type
- It can be one data like [number] and also can be be two like [number,number], max 10 number can be applied 
- [Arnuino Sample Cde Here](https://github.com/manulsan/atCloud/blob/main/sdks/arduino/esp8266/InputOutput/InputOutput.ino)
- ex>
 ```js
    void publishData(uint32_t now)
    {
        char szBuf[128];
        sprintf(szBuf, "[%d,%d,%d,%d,%d,%d]",
                _portMap[0].state, _portMap[1].state, _portMap[2].state,
                _portMap[3].state, _portMap[4].state, _portMap[5].state);        
        publish("dev-data", szBuf);    
 ```

  
### STATUS Syntax
- Any string 
- ex>
```
    publish("dev-data", "System is Ready");    
```

## Device Data And App View expression
- Device has "Serial Number" only.
- The data is defined at the platform by the user.( if not, default property is applied)
- <bold> "Data Field"</bold> is one of device data expression and uer can define it name or properties
- All information can be made with QR code ( commercial device only )

<img src="./assets/img/menu-add-device.png">