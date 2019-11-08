**2019.10.06**

- settings are stored in EEPROM now;
- added 2 flip effects: train and elastic band) ;
- mode ("show time" and "show temp") changes after flip effect ends;
- fixed bug with speed of flip effect after changing it - previoulsy the speed remained the same. It turned out it was hardcoded.
- now backlMaxBright and 0 apply only once (while changing with middle button) - moved from bright.ino to buttonsSettings.ino

**2019.09.25**

- "show temp" mode can be switched on/off with hold "+" button

**2019.09.12**

- added DHT22 support (data pin - pin 13 by default)

**2019.09.09**

- Alarm added. 2 variants are available - software switch and toggle switch (GND - toggle switch - pin 1 by default).