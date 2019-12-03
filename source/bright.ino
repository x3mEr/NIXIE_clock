void backlBrightTick() {
  if (BACKL_MODE == 0 && backlBrightTimer.isReady()) {
    if (backlBrightDirection) {
      if (!backlBrightFlag) {
        backlBrightFlag = true;
        backlBrightTimer.setInterval(backlInterval);
      }
      backlBrightCounter += backlMaxStep;
      if (backlBrightCounter >= backlMaxBright) {
        backlBrightDirection = false;
        backlBrightCounter = backlMaxBright;
      }
    } else {
      backlBrightCounter -= backlMaxStep;
      if (backlBrightCounter <= backlMinBright) {
        backlBrightDirection = true;
        backlBrightCounter = backlMinBright;
        backlBrightTimer.setInterval(BACKL_PAUSE);
        backlBrightFlag = false;
      }
    }
    analogWrite(BACKL, backlBrightCounter); //setPWM does not turn off the PWM, so backl is always on
  } /* else if (BACKL_MODE == 1) {
    setPWM(BACKL, backlMaxBright);
  } else if (BACKL_MODE == 2) {
    digitalWrite(BACKL, 0);
  } */
}

void dotBrightTick() {
  if (dotBrightFlag && dotBrightTimer.isReady()) {
    if (dotBrightDirection) {
      dotBrightCounter += dotBrightStep;
      if (dotBrightCounter >= dotMaxBright) {
        dotBrightDirection = false;
        dotBrightCounter = dotMaxBright;
      }
    } else {
      dotBrightCounter -= dotBrightStep;
      if (dotBrightCounter <= 0) {
        dotBrightDirection = true;
        dotBrightFlag = false;
        dotBrightCounter = 0;
      }
    }
    setPWM(DOT, dotBrightCounter);
  }
}

void changeBright() { //change brightness of indicators, dot and backl according to time
#if (NIGHT_LIGHT == 1)
  if ( (hrs >= NIGHT_START && hrs <= 23)
       || (hrs >= 0 && hrs < NIGHT_END) ) {
    indiMaxBright = INDI_BRIGHT_N;
    dotMaxBright = DOT_BRIGHT_N;
    backlMaxBright = BACKL_BRIGHT_N;
    backlMinBright = BACKL_BRIGHT_MINIMUM_N;
    backlMaxTime = BACKL_TIME_N;
    backlMaxStep = BACKL_STEP_N;
  } else {
    indiMaxBright = INDI_BRIGHT;
    dotMaxBright = DOT_BRIGHT;
    backlMaxBright = BACKL_BRIGHT;
    backlMinBright = BACKL_BRIGHT_MINIMUM;
    backlMaxTime = BACKL_TIME;
    backlMaxStep = BACKL_STEP;
  }
  for (byte i = 0; i < 4; i++) {
    indiDimm[i] = indiMaxBright;
  }

  // расчёт шага яркости точки
  dotBrightStep = ceil((float)dotMaxBright * 2 / DOT_TIME * DOT_TIMER); //*2 - because we need to light and extinguish the dot
  if (dotBrightStep == 0) dotBrightStep = 1;

  // дыхание подсветки
  backlInterval = (float)backlMaxStep / (backlMaxBright-backlMinBright) / 2 * backlMaxTime; //*2 - because we need to light and extinguish the backl
  backlBrightTimer.setInterval(backlInterval);
  indiBrightCounter = indiMaxBright;
  
  /*to apply brightness changes to backl immediately
  if (backlBrightDirection) backlBrightCounter = backlMinBright;
  else backlBrightCounter = backlMaxBright; */
  
  //change PWM to apply backlMaxBright in case of maximum bright mode,
  //since in case of BACKL_MODE == 1, setPWM is executed only by clicking button "-" (in case of mode changing)
  if (BACKL_MODE == 1) setPWM(BACKL, backlMaxBright);
#endif
}
