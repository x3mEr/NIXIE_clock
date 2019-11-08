*2019.10.05*
	- settings are stored in EEPROM now;
	- added 2 flip effects: train and elastic band) ;
	- mode ("show time" and "show temp") changes after flip effect ends;
	- fixed bug with speed of flip effect after changing it - previoulsy the speed remained the same. It turned out it was hardcoded.
	- now backlMaxBright and 0 apply only once (while changing with middle button) - moved from bright.ino to buttonsSettings.ino
