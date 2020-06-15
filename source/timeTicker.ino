void calculateTime() {
  dotFlag = !dotFlag;
  if (dotFlag) {
    dotBrightFlag = true;
    dotBrightDirection = true;
    dotBrightCounter = 0;
    secs++;
    #if (CHECK_EFFECTS)
      secs += 9;
    #endif
    if (secs > 59) {
      newTimeFlag = true;   // flag means that time shown on display should be changed
      secs = 0;
      mins++;
      if (mins == 1 && !fl_syncedAt1) {	// every half an hour
        burnIndicators();				// чистим чистим!
        DateTime now = rtc.now();		// sync with RTC
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
        fl_syncedAt1 = true;
        fl_syncedAt31 = false;
      }
      else if (mins == 31 && !fl_syncedAt31) {	// every half an hour
        burnIndicators();						// чистим чистим!
        DateTime now = rtc.now();				// sync with RTC
        secs = now.second();
        mins = now.minute();
        hrs = now.hour();
        fl_syncedAt31 = true;
        fl_syncedAt1 = false;
      }
    }

    if (mins > 59) { //возможно, надо перенести в if (secs > 59), но только если минуты не могут поменяться сами (не по превышению секунд > 59, например, при настройке времени)
      mins = 0;
      hrs++;
      if (hrs > 23) hrs = 0;
      changeBright();
    }

    if (newTimeFlag) {
      setNewTime();         // update time digits array
      if ((TUMBLER && !digitalRead(ALARM_SW) && !alm_flag && alm_mins == mins && alm_hrs == hrs) // there is the tumbler, it is toggled ON, alarm is not ringing and it's time to ring
         || (!TUMBLER && !alm_flag && alm_mins == mins && alm_hrs == hrs)) { // there is not the tumbler, alarm is not ringing and it's time to ring
        //curMode = 0;
		newTimeFlag = false; // if curMode==3, alarm will ring, will show current time, but won't call flipTick and 'newTimeFlag' won't be reset. I. e., if turn alarm off with the button during modeTimer (while curMode==3) or while flip effect, alarm will ring again, as all conditions are fulfilled: current time == alarm time and newTimeFlag==True
        sendTime(hrs,mins); // if temp and humidity are on display (lol, display)
        alm_flag = true;
        almTimer.start();
        almTimer.reset();
      }
    }

    if (alm_flag) { // alarm is ringing. Check for turning it off.
      if ((TUMBLER && (almTimer.isReady() || digitalRead(ALARM_SW) || flTurnAlarmOff))
         || (!TUMBLER && (almTimer.isReady() || flTurnAlarmOff))) { // alarm timed out or was turned off with the tumbler
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
        alm_flag = false;
        flTurnAlarmOff = false;
        almTimer.stop();
        //curMode = 0;
        sendTime(hrs, mins);
        for (byte i = 0; i < 4; i++) anodeStates[i] = 1;
        #if TEMP_HUM_SENSOR
          modeTimer.setInterval((long)CLOCK_TIME * 1000); // Not to be confused with temp and humidity on display after exiting settings
        #endif
      }
    }
  }

  // to ring and blink
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