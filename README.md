## Retro clock with alarm and DHT22 (temperature and humidity sensor) based on Soviet Union IN-14 (NIXIE in USA) indicators. Firmware and PCB.

First of all, my project is based on two Alex Gyver's projects: [NixieClock](https://github.com/AlexGyver/NixieClock) and [NixieClock_v2](https://github.com/AlexGyver/NixieClock_v2).
Which in turn are based on [Andrey Zhelezniakov's project](https://itworkclub.ru/arduino-%D1%87%D0%B0%D1%81%D1%8B-%D0%BD%D0%B0-%D0%B3%D0%B0%D0%B7%D0%BE%D1%80%D0%B0%D0%B7%D1%80%D1%8F%D0%B4%D0%BD%D1%8B%D1%85-%D0%B8%D0%BD%D0%B4%D0%B8%D0%BA%D0%B0%D1%82%D0%BE%D1%80%D0%B0%D1%85/).

*A lot of useful information could be found [there](https://alexgyver.ru/nixieclock_v2/)*


## About

The clock is driven by Arduino Nano v3.0 with ATmega328. It would be great to replace it with microcontroller (*but I'm lazy*).
The board power supply: 5V.
Antioxidation procedure every 30 mins.
2 modes of brightness: day and night modes. The brightness of digits and backlight changes depending on time.
3 modes of indicators backlight.
4 modes of digit change.
2 variants of alarm: with and without alarm switcher. In the latter case, if you do not need the alarm, it should be set to 24 hour (not 00, exactly 24).
2 types of buzzer: active and passive. While passive buzzer being used, the frequency could be generated with PWM or with main loop (more details are in source code).


## Controls

	- Hold "SET" - switch between clock modes:
		0. initial mode "show clock",
		1. "set alarm",
		2. "set clock".

	- Click "SET" in "set alarm" and "set clock" modes - change between setting hours and minutes.
	- Double click "SET" in "set alarm" mode - exit to "show clock" mode, skipping "set clock" mode.
	- Click "SET" while alarm ring (in case of alarm switcher absence) - turn alarm off.

**Effects:**

Settings should be done in "clock" mode.

	- Click "-" - change backlight mode:
		1. breath,
		2. always on,
		3. off.
	- Click "+" - change effects of digits' appearance:
		1. no effect,
		2. smooth fading,
		3. rewind in order of number,
		4. rewind in order of cathode.
	- Hold "-" - turn the "glitches" on/off.
	- Hold "+" - turn the "show temp/hum" on/off.