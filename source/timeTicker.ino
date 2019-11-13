void calculateTime() {
  dotFlag = !dotFlag;
  if (dotFlag) {
    dotBrightFlag = true;
    dotBrightDirection = true;
    dotBrightCounter = 0;
    secs++;
    #if (CHECK_EFFECTS)
    if (secs % 10 == 0) newTimeFlag = true;
    #endif
    if (secs > 59) {
      newTimeFlag = true;   // флаг что нужно поменять время
      secs = 0;
      mins++;
      if (mins == 1 || mins == 31) {    // каждые полчаса
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
        //curMode = 0;
		newTimeFlag = false; // если curMode==3, зазвенит, покажет текущее время, но не вызовет flipTick и не сбросит флаг. Т. е. если выключить будильник кнопкой в течение modeTimer (пока длится curMode==3), он  снова зазвенит, т. к. выполнятся все условия: время и newTimeFlag
        sendTime(hrs,mins); // если сейчас показывает темп и влажн
        alm_flag = true;
        almTimer.start();
        almTimer.reset();
      }
    }
    
    if (alm_flag) { // будильник звенит. Проверка для выключения
      if ((TUMBLER && (almTimer.isReady() || digitalRead(ALARM_SW) || flTurnAlarmOff))
         || (!TUMBLER && (almTimer.isReady() || flTurnAlarmOff))) { // таймаут будильника или выключили тумблером вручную
        alm_flag = false;
        flTurnAlarmOff = false;
        almTimer.stop();
        //curMode = 0;
        #if !BUZZER_PASSIVE
          setPin(PIEZO,0);
        #endif
        #if BUZZER_PASSIVE
          /* 1st method
          noNewTone(PIEZO);
          TCCR1B = TCCR1B & 0b11111000 | 1; // bring PWM frequency settings back for generator (indicators)
          setPWM(9, DUTY);
          */
          setPin(PIEZO,0); //2nd method
        #endif
        sendTime(hrs, mins);
        for (byte i = 0; i < 4; i++) anodeStates[i] = 1;
        modeTimer.setInterval((long)CLOCK_TIME * 1000); // Чтобы после выхода из настроек не попасть на показ темп и влажн - можно запутаться
      }
    }
  }

  // to ring and to blink
  if (alm_flag) { // возможно, надо перенести в if (dotFlag)
    if (!dotFlag) {
      for (byte i = 0; i < 4; i++) anodeStates[i] = 0;
	  #if !BUZZER_PASSIVE
        setPin(PIEZO,1);
      #endif
      #if BUZZER_PASSIVE
        /* 1st method
        NewTone(PIEZO, FREQ); // buzzer on, indicators off. They could not be used simultaneously because of Timer1 using
        */
        // 2nd method rings in main loop
      #endif
    } else {
      #if !BUZZER_PASSIVE
        setPin(PIEZO,0);
      #endif
      #if BUZZER_PASSIVE
        /* 1st method
        noNewTone(PIEZO); // now PWM frequency should be re-set for generator (indicators)
        TCCR1B = TCCR1B & 0b11111000 | 1; // bring PWM frequency settings back for indicators
        setPWM(9, DUTY);
        */
        // 2nd method rings in main loop
      #endif
      for (byte i = 0; i < 4; i++) anodeStates[i] = 1;
    }
  }
}