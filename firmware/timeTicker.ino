void calculateTime() {
  dotFlag = !dotFlag;
  if (dotFlag) {
    dotBrightFlag = true;
    dotBrightDirection = true;
    dotBrightCounter = 0;
    secs++;
    if (secs > 59) {
      newTimeFlag = true;   // флаг что нужно поменять время
      secs = 0;
      mins++;
      if (mins == 1 || mins == 30) {    // каждые полчаса
        burnIndicators();               // чистим чистим!
        DateTime now = rtc.now();       // синхронизация с RTC
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
      }
    }

    if (mins > 59) { //возможно, надо перенести в if (secs > 59), но только если минуты не могут поменяться сами (не по превышению секунд > 59, например, при настройке времени)
      mins = 0;
      hrs++;
      if (hrs > 23) hrs = 0;
      changeBright();
    }

    if (newTimeFlag) {
      setNewTime();         // обновляем массив времени
      if ((TUMBLER && !digitalRead(ALARM_SW) && !alm_flag && alm_mins == mins && alm_hrs == hrs) // есть тумблер, он в положении ВКЛ, будильник не звенит и ему пора звенеть
         || (!TUMBLER && !alm_flag && alm_mins == mins && alm_hrs == hrs)) { // нет тумблера, будильник не звенит и ему пора звенеть
        //mode = 0;
        alm_flag = true;
        almTimer.start();
        almTimer.reset();
      }
    }
    
    //if (mode == 0) sendTime(hrs, mins);
    if (alm_flag) { // будильник звенит. Проверка для выключения
      if ((TUMBLER && (almTimer.isReady() || digitalRead(ALARM_SW)))
         || (!TUMBLER && (almTimer.isReady() || flTurnAlarmOff))) { // таймаут будильника или выключили тумблером вручную
        alm_flag = false;
		flTurnAlarmOff = false;
        almTimer.stop();
        //mode = 0;
        noNewTone(PIEZO);
        TCCR1B = TCCR1B & 0b11111000 | 1;		// ставим делитель 1
        // включаем ШИМ  
        setPWM(9, DUTY);
        sendTime(hrs, mins);
		for (byte i = 0; i < 4; i++) anodeStates[i] = 1;
      }
    }
  }

  // мигать на будильнике
  if (alm_flag) { // возможно, надо перенести в if (dotFlag)
    if (!dotFlag) {
      noNewTone(PIEZO);
      TCCR1B = TCCR1B & 0b11111000 | 1;		// ставим делитель 1
      // включаем ШИМ  
      setPWM(9, DUTY);
      for (byte i = 0; i < 4; i++) anodeStates[i] = 1; // выкл индикаторы - ничего не горит, когда число 10
	  // или через anodeStates[i] = 0
    } else {
      NewTone(PIEZO, FREQ);
      for (byte i = 0; i < 4; i++) anodeStates[i] = 0;
    }
  }
}
