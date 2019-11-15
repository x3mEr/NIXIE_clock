**2019.10.15**

- fix: if RTC is slower than time computation on arduino with dotTimer, this caused freezes (and even crashes), repeatedly running synchronization (in some sense, it is similar to limits in mathematical analysis). Now synchronization is limited to once every 30 min.

**2019.10.13**

- fix: backlight brightness changes according to daytime (in case of night mode is activated). Bug appeared after 2019.10.06 update, when backlBrightCounter = backlMaxBright or 0 was moved from bright.ino to buttonsSettings.ino.
- fix: flipTimer resets at effect initialization.
- upd: effects 4 (train) and 5 (elastic band) improved: now digits, not anodes, move along the indicators.
- upd: for alarm beeper in timeTicker.ino setPin() from GyverHacks lib is used (insted of digitalWrite()).

**2019.10.06**

- add: settings are stored in EEPROM now.
- add: added 2 flip effects: train and elastic band).
- upd: mode ("show time" and "show temp") changes after flip effect ends.
- fix: fixed bug with speed of flip effect after changing it - previoulsy the speed remained the same. It turned out it was hardcoded.
- upd: now backlMaxBright and 0 apply only once (while changing with middle button) - moved from bright.ino to buttonsSettings.ino.

**2019.09.25**

- add: "show temp" mode can be switched on/off with hold "+" button.

**2019.09.12**

- add: added DHT22 support (data pin - pin 13 by default).

**2019.09.09**

- fix: condition of brightness change: hrs < NIGHT_END (strictly less).
- add: alarm added. 2 variants are available - software switch and toggle switch (GND - toggle switch - pin 1 by default).