/*  Copyright (C) 2019  Pavel Smelov
	This program is licensed under GNU GPLv3
	The full notice can be found in the nixieClock.ino */

void settingsTick() { 					// моргать настраиваемыми числами (часы или минуты)
  //if (curMode == 1 || curMode == 2) { 	// в режиме настроек будильника или часов //this state is checked in main loop right before execution this function
    if (blinkTimer.isReady()) { 		// пришло время моргать
      //sendTime(changeHrs, changeMins);  // пошлётся то, что настраиваем - будильник или часы
      lampState = !lampState;
      if (lampState) {
        anodeStates[0] = 1;
        anodeStates[1] = 1;
        anodeStates[2] = 1;
        anodeStates[3] = 1;
      }
	  else { // погасить (моргнуть) часы или минуты в зависимости от того, что сейчас настраиваемаиваем
        if (!currentDigit) {
          anodeStates[0] = 0;
          anodeStates[1] = 0;
        } else {
          anodeStates[2] = 0;
          anodeStates[3] = 0;
        }
      }
    }
  //}
}

void buttonsTick() {
/* для настроек часов и будильника используются
одинаковые переменные: changeHrs и changeMins,
т. к. они используются в settingsTick().
Иначе пришлось бы делать две ф-ии settingsTick():
для случая настройки будильника, и для случая настройки часов */
  btnSet.tick();
  btnL.tick();
  btnR.tick();

/*According to GyverTimer lib, flag of pressed btn resets only after reading its state[1].
So all presses will be stored in memory until appropriate conditions.
For example, if btnL holded (turn glitches on/off) while "set alarm" mode,
glitches will be turned on/off after leaving "set alarm" mode, in "show clock" mode.
*/
//[1] Holding a button resets variable last_counter, thereby reseting isDouble()
  if (alm_flag) { //disable unwanted clicks while alarming (all clicks, but btnSet.isClick)
    //btnSet.isClick() turns tha alarm off
    btnSet.isHolded();
    //btnSet.isDouble(); //includes isClick(), which turn the alarm off. Nothing more will happen.
    btnL.isClick();
    btnL.isHolded();
    btnR.isClick();
    btnR.isHolded();
  }
  else if (curMode == 0 || curMode == 3) { //disable unwanted clicks while "show time" or "show temp"
    btnSet.isClick(); //it's not ALARM now as it was checked in first "if"
    btnSet.isDouble();
  }
  else if (curMode == 1) { //disable unwanted clicks while setting alarm
    btnL.isHolded();
    btnR.isHolded();
  }
  else if (curMode == 2) { //disable unwanted clicks while setting clock
    btnL.isHolded();
    btnR.isHolded();
    btnSet.isDouble();
  }   

  if ((curMode == 0) || (curMode == 3)) {
    // переключение эффектов подсветки
    if (btnL.isClick()) {
      if (++BACKL_MODE >= 3) BACKL_MODE = 0;
      EEPROM.updateByte(3, BACKL_MODE);
      if (BACKL_MODE == 1) {
        setPWM(BACKL, backlMaxBright);
      } else if (BACKL_MODE == 2) {
        digitalWrite(BACKL, 0);
      }
    }

    // переключение эффектов цифр
    if (btnR.isClick()) {
      if (++FLIP_EFFECT >= FLIP_EFFECT_NUM) FLIP_EFFECT = 0;
      EEPROM.updateByte(2, FLIP_EFFECT);
      flipTimer.setInterval(FLIP_SPEED[FLIP_EFFECT]);
      for (byte i = 0; i < 4; i++) {
        indiDimm[i] = indiMaxBright;
        anodeStates[i] = 1;
      }
    }

    // переключение глюков
    if (btnL.isHolded()) {
      GLITCH_ALLOWED = !GLITCH_ALLOWED;
      EEPROM.updateByte(4, GLITCH_ALLOWED);
    }

    //разрешить/запретить показ темп и влажн
    if (btnR.isHolded()) {
      TEMPHUM_ALLOWED = !TEMPHUM_ALLOWED;
      EEPROM.updateByte(5, TEMPHUM_ALLOWED);
      curMode = 0;
      sendTime(hrs,mins);
    }
  }
  else if (curMode == 1) { // set alarm
    if (btnR.isClick()) {
      if (!currentDigit) {
        changeHrs++;
        if (changeHrs > 23+1-TUMBLER) changeHrs = 0; // 24 - alm is OFF
      } else {
        changeMins++;
        if (changeMins > 59) changeMins = 0;
      }
      sendTime(changeHrs, changeMins);
    }
    if (btnL.isClick()) {
      if (!currentDigit) {
        changeHrs--;
        if (changeHrs < 0) changeHrs = 23+1-TUMBLER; // 24 - alm is OFF
      } else {
        changeMins--;
        if (changeMins < 0) changeMins = 59;
      }
      sendTime(changeHrs, changeMins);
    }
    if (btnSet.isDouble()) { // из режима настройки будильника в режим часов
      anodeStates[0] = 1;
      anodeStates[1] = 1;
      anodeStates[2] = 1;
      anodeStates[3] = 1;
      currentDigit = false; // настройка часов, TRUE - минут
      alm_hrs = changeHrs;
      alm_mins = changeMins;
      EEPROM.updateByte(0, alm_hrs);
      EEPROM.updateByte(1, alm_mins);
      #if TEMP_HUM_SENSOR
        modeTimer.setInterval((long)CLOCK_TIME * 1000);//or reset()? // Чтобы после выхода из настроек не попасть на показ темп и влажн - можно запутаться
      #endif
      // если кто-то будет настраивать будильник более получаса)
      //DateTime now = rtc.now();
      //hrs = now.hour();
      //mins = now.minute();
      curMode = 0;
      sendTime(hrs, mins);
    }
  }
  else if (curMode == 2) { // настройка часов
    if (btnR.isClick()) {
      if (!currentDigit) {
        changeHrs++;
        if (changeHrs > 23) changeHrs = 0;
      } else {
        changeMins++;
        if (changeMins > 59) changeMins = 0;
      }
      sendTime(changeHrs, changeMins);
    }
    if (btnL.isClick()) {
      if (!currentDigit) {
        changeHrs--;
        if (changeHrs < 0) changeHrs = 23;
      } else {
        changeMins--;
        if (changeMins < 0) changeMins = 59;
      }
      sendTime(changeHrs, changeMins);
    }
  }

  if (btnSet.isHolded()) {
    anodeStates[0] = 1;
    anodeStates[1] = 1;
    anodeStates[2] = 1;
    anodeStates[3] = 1;
    currentDigit = false; //настройка часов, TRUE - минут
	curMode++;
    if (curMode == 4) curMode = 1; //если попали в настройки из "show temp"
	else if (curMode >= 3) curMode = 0;
    switch (curMode) {
      case 0: //из режима настройки часов в режим часов
        hrs = changeHrs;
        mins = changeMins;
		secs = 0;
        rtc.adjust(DateTime(2019, 12, 05, hrs, mins, secs));
        changeBright();
		#if TEMP_HUM_SENSOR
          modeTimer.setInterval((long)CLOCK_TIME * 1000); //чтобы после выхода из настроек не попасть на показ темп и влажн - можно запутаться
        #endif
        sendTime(hrs, mins);
        break;
      case 1: //попадаем из режима часов в режим настройки будильника
        changeHrs = EEPROM.readByte(0);
	    changeMins = EEPROM.readByte(1);
		sendTime(changeHrs, changeMins);
        break;
	  case 2: //попадаем из режима настройки будильника в настройку часов
        EEPROM.updateByte(0, changeHrs);
        EEPROM.updateByte(1, changeMins);
        alm_hrs = changeHrs;
        alm_mins = changeMins;
        changeHrs = hrs;
        changeMins = mins;
        sendTime(changeHrs, changeMins);
        break;
      /*case 3: //попадаем из режима настройки часов в показ темп и влажн. Если нужен такой режим, надо добавить таймер обновления показаний. При входе сюда настроить RTC. По выходу - обновить переменные времени
	    byte temp = dht.readTemperature();
        byte hum = dht.readHumidity();
        sendTime(temp, hum);
      */
    }
  }

  if (btnSet.isClick()) {
    if (curMode == 1 || curMode == 2) currentDigit = !currentDigit; // выбор настройки часов или минут
    /*#if !TUMBLER
	  else if (!TUMBLER && alm_flag) flTurnAlarmOff = true; //будильник звенит, выключить кнопкой SET
    #endif
	*/
	else if (alm_flag) flTurnAlarmOff = true; //будильник звенит, выключить кнопкой SET
  }
}
