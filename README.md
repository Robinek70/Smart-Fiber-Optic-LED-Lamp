# Smart-Fiber-Optic-LED-Lamp  
Smart version of very nice project  
A 3D printed Lamp with WS2812 LEDs and side glow fiber optic now in Smart version  

![icon](https://github.com/Robinek70/Smart-Fiber-Optic-LED-Lamp/raw/esp8266/icon.png) Smart version of decoration lamp based on D1 mini Wifi ESP8266 and powered by micro USB cable.  
Connecion:  
Power - USB  
LED Data In to module pin D4  
LED 5V to module 5V  
LED GND to module GND  
Switch between module pins D3 and D2  

Lamp can change all color, patterns and effects from web page or by url request.  
Examples:  
```
{{ip}}/update?state=ON  
{{ip}}/update?mode=Solid&colorCounter=Red  
```

Added support for MQTT, all settings can be changed. 
Example:  
```
FiberLamp/set/colorCounter Red  
FiberLamp/set/state ON  
```
Easy Home Assistant configuration or Auto Discovery   
```
- platform: mqtt  
  name: "Fiber Optic Lamp"  
  state_topic: "FiberLamp/state"  
  command_topic: "FiberLamp/set/state"  
  availability_topic: "FiberLamp/status"  
  icon: hass:lightbulb  
  qos: 1  
  retain: true  
```
Added auto change effects, brightness and speed.  
Added OTA support.  
Added MQTT support.  
Added Home Assistant MQTT Auto Discovery  
![stand](https://github.com/Robinek70/Smart-Fiber-Optic-LED-Lamp/raw/esp8266/images/empty-stand.jpg)
![stand](https://github.com/Robinek70/Smart-Fiber-Optic-LED-Lamp/raw/esp8266/images/d1-module.jpg)
![lamp](https://github.com/Robinek70/Smart-Fiber-Optic-LED-Lamp/raw/esp8266/images/lamp.jpg)

Attached modified stand to fit D1 module and another switch (6x6mm)

More info about original design on instructables.com - https://www.instructables.com/Fiber-Optic-LED-Lamp/  
