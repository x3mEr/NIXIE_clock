void settingsTick() { 					// моргать настраиваемыми числами (часы или минуты)
  if (curMode == 1 || curMode == 2) { 	// в режиме настроек будильника или часов
    if (blinkTimer.isReady()) { 		// пришло время моргать
      sendTime(changeHrs, changeMins);  // пошлётся то, что настраиваем - будильник или часы
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
  }
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

  if (curMode == 1) { // настройка будильника
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
  else if (curMode == 0) {
    // переключение эффектов цифр
    if (btnR.isClick()) {
      if (++FLIP_EFFECT >= 4) FLIP_EFFECT = 0;
      for (byte i = 0; i < 4; i++) {
        indiDimm[i] = indiMaxBright;
      }
    }

    // переключение эффектов подсветки
    if (btnL.isClick()) {
      if (++BACKL_MODE >= 3) BACKL_MODE = 0;
    }

    // переключение глюков
    if (btnL.isHolded()) GLITCH_ALLOWED = !GLITCH_ALLOWED;
  }

  if (btnSet.isHolded()) {
    anodeStates[0] = 1;
    anodeStates[1] = 1;
    anodeStates[2] = 1;
    anodeStates[3] = 1;
    currentDigit = false; // настройка часов, TRUE - минут
    if (++curMode >= 3) curMode = 0;
    switch (curMode) {
      case 0: // из режима настройки часов в режим часов
        hrs = changeHrs;
        mins = changeMins;
        modeTimer.setInterval((long)CLOCK_TIME * 1000); // Чтобы после выхода из настроек не попасть на показ темп и влажн - можно запутаться
        rtc.adjust(DateTime(2019, 12, 05, hrs, mins, 0));
        changeBright();
        //sendTime(hrs, mins);
        break;
      case 1: // попадаем из режима часов в режим настройки будильника
        changeHrs = EEPROM.readByte(0);
	    changeMins = EEPROM.readByte(1);
		sendTime(changeHrs, changeMins);
        break;
	  case 2: // попадаем из режима настройки будильника в настройку часов
        EEPROM.updateByte(0, changeHrs);
        EEPROM.updateByte(1, changeMins);
        alm_hrs = changeHrs;
        alm_mins = changeMins;
        changeHrs = hrs;
        changeMins = mins;
        // если кто-то будет настраивать будильник более получаса)
        //DateTime now = rtc.now();
        //changeHrs = now.hour();
        //changeMins = now.minute();
        //sendTime(changeHrs, changeMins);
        break;
      /*case 3: // попадаем из режима настройки часов в показ темп и влажн. Если нужен такой режим, надо добавить таймер обновления показаний. При входе сюда настроить RTC. По выходу - обновить переменные времени
	    byte temp = dht.readTemperature();
        byte hum = dht.readHumidity();
        sendTime(temp, hum);
      */
    }
  }

  if (curMode == 1 && btnSet.isDouble()) { // из режима настройки будильника в режим часов
    anodeStates[0] = 1;
    anodeStates[1] = 1;
    anodeStates[2] = 1;
    anodeStates[3] = 1;
    currentDigit = false; // настройка часов, TRUE - минут
    curMode = 0;
    EEPROM.updateByte(0, changeHrs);
    EEPROM.updateByte(1, changeMins);
    alm_hrs = changeHrs;
    alm_mins = changeMins;
	modeTimer.setInterval((long)CLOCK_TIME * 1000);
    // если кто-то будет настраивать будильник более получаса)
    //DateTime now = rtc.now();
    //hrs = now.hour();
    //mins = now.minute();
    sendTime(hrs, mins);
  }

  if (btnSet.isClick()) {
    if (curMode == 1 || curMode == 2) currentDigit = !currentDigit; // выбор настройки часов или минут
    else if (!TUMBLER && alm_flag) flTurnAlarmOff = true; //будильник звенит, выключить кнопкой SET
  }
}
