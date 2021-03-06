/*  Copyright (C) 2019  Pavel Smelov
	This program is licensed under GNU GPLv3
	The full notice can be found in the nixieClock.ino */

void flipTick() {
  if (FLIP_EFFECT == 0) {
    sendTime(hrs, mins);
    newTimeFlag = false;
  }
// --- smooth fading --- //
  else if (FLIP_EFFECT == 1) {
    if (!flipInit) {
      flipInit = true;
      // запоминаем, какие цифры поменялись и будем менять их яркость
      for (byte i = 0; i < 4; i++) {
        if (indiDigits[i] != newTime[i]) flipIndics[i] = true;
        else flipIndics[i] = false;
      }
      flipTimer.reset();
    }
    if (flipTimer.isReady()) {
      if (!indiBrightDirection) {
        indiBrightCounter--;            // уменьшаем яркость
        if (indiBrightCounter <= 0) {   // если яроксть меньше нуля
          indiBrightDirection = true;   // меняем направление изменения
          indiBrightCounter = 0;        // обнуляем яркость
          sendTime(hrs, mins);          // меняем цифры
        }
      } else {
        indiBrightCounter++;                        // увеличиваем яркость
        if (indiBrightCounter >= indiMaxBright) {   // достигли предела
          indiBrightDirection = false;              // меняем направление
          indiBrightCounter = indiMaxBright;        // устанавливаем максимум
          // выходим из цикла изменения
          flipInit = false;
          newTimeFlag = false;
          #if TEMP_HUM_SENSOR
            modeTimer.reset();
          #endif
        }
      }
      for (byte i = 0; i < 4; i++)
        if (flipIndics[i]) indiDimm[i] = indiBrightCounter;   // применяем яркость
    }
  }

// --- rewind in order of number --- //
  else if (FLIP_EFFECT == 2) {
    if (!flipInit) {
      flipInit = true;
      // запоминаем, какие цифры поменялись и будем менять их
      for (byte i = 0; i < 4; i++) {
        if (indiDigits[i] != newTime[i]) flipIndics[i] = true;
        else flipIndics[i] = false;
      }
      flipTimer.reset();
    }

    if (flipTimer.isReady()) {
      byte flipCounter = 0;
      for (byte i = 0; i < 4; i++) {
        if (flipIndics[i]) {
          indiDigits[i]--;
          if (indiDigits[i] < 0) indiDigits[i] = 9;
          if (indiDigits[i] == newTime[i]) flipIndics[i] = false;
        } else {
          flipCounter++;        // счётчик цифр, которые не надо менять
        }
      }
      if (flipCounter == 4) {   // если ни одну из 4 цифр менять не нужно
        // выходим из цикла изменения
        flipInit = false;
        newTimeFlag = false;
        #if TEMP_HUM_SENSOR
          modeTimer.reset();
        #endif
      }
    }

    //byte cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6};  // порядок катодов in14
  }

// --- rewind in order of cathode --- //
  else if (FLIP_EFFECT == 3) {
    if (!flipInit) {
      flipInit = true;
      // запоминаем, какие цифры поменялись и будем менять их
      for (byte i = 0; i < 4; i++) {
        if (indiDigits[i] != newTime[i]) {
          flipIndics[i] = true;
          for (byte c = 0; c < 10; c++) {
            if (cathodeMask[c] == indiDigits[i]) startCathode[i] = c;
            if (cathodeMask[c] == newTime[i]) endCathode[i] = c;
          }
        }
        else flipIndics[i] = false;
      }
      flipTimer.reset();
    }

    if (flipTimer.isReady()) {
      byte flipCounter = 0;
      for (byte i = 0; i < 4; i++) {
        if (flipIndics[i]) {
          if (startCathode[i] > endCathode[i]) {
            startCathode[i]--;
            indiDigits[i] = cathodeMask[startCathode[i]];
          } else if (startCathode[i] < endCathode[i]) {
            startCathode[i]++;
            indiDigits[i] = cathodeMask[startCathode[i]];
          } else {
            flipIndics[i] = false;
          }
        } else {
          flipCounter++;
        }
      }
      if (flipCounter == 4) {   // если ни одну из 4 цифр менять не нужно
        // выходим из цикла изменения
        flipInit = false;
        newTimeFlag = false;
        #if TEMP_HUM_SENSOR
          modeTimer.reset();
        #endif
      }
    }
  }

// --- train --- //
  else if (FLIP_EFFECT == 4) {
    if (!flipInit) {
      flipInit = true;
      currentLamp = 0;
      trainLeaving = true;
      flipTimer.reset();
    }
    if (flipTimer.isReady()) {
      if (trainLeaving) {
        for (byte i = 3; i > currentLamp; i--) {
          indiDigits[i] = indiDigits[i-1];
        }
        anodeStates[currentLamp] = 0;
        currentLamp++;
        if (currentLamp >= 4) {
          trainLeaving = false; //coming
          currentLamp = 0;
          //sendTime(hrs, mins);
        }
      }
      else { //trainLeaving == false
        for (byte i = currentLamp; i > 0; i--) {
          indiDigits[i] = indiDigits[i-1];
        }
        indiDigits[0] = newTime[3-currentLamp];
        anodeStates[currentLamp] = 1;
        currentLamp++;
        if (currentLamp >= 4) {
          flipInit = false;
          newTimeFlag = false;
          #if TEMP_HUM_SENSOR
            modeTimer.reset();
          #endif
        }
      }
    }
  }

// --- elastic band --- //
  else if (FLIP_EFFECT == 5) {
    if (!flipInit) {
      flipInit = true;
      flipEffectStages = 0;
      flipTimer.reset();
    }
    if (flipTimer.isReady()) {
      switch (flipEffectStages++) {
        case 1:
          anodeStates[3] = 0; break;
        case 2:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[2];
          anodeStates[3] = 1; break;
        case 3:
          anodeStates[3] = 0; break;
        case 4:
          anodeStates[1] = 0;
          indiDigits[2] = indiDigits[1];
          anodeStates[2] = 1; break;
        case 5:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[1];
          anodeStates[3] = 1; break;
        case 6:
          anodeStates[3] = 0; break;
        case 7:
          anodeStates[0] = 0;
          indiDigits[1] = indiDigits[0];
          anodeStates[1] = 1; break;
        case 8:
          anodeStates[1] = 0;
          indiDigits[2] = indiDigits[0];
          anodeStates[2] = 1; break;
        case 9:
          anodeStates[2] = 0;
          indiDigits[3] = indiDigits[0];
          anodeStates[3] = 1; break;
        case 10:
          anodeStates[3] = 0;
          //sendTime(hrs,mins);
		  break;
        case 11:
          indiDigits[0] = newTime[3];
          anodeStates[0] = 1; break;
        case 12:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[3];
          anodeStates[1] = 1; break;
        case 13:
          anodeStates[1] = 0;
          indiDigits[2] = newTime[3];
          anodeStates[2] = 1; break;
        case 14:
          anodeStates[2] = 0;
          indiDigits[3] = newTime[3];
          anodeStates[3] = 1; break;
        case 15:
          indiDigits[0] = newTime[2];
          anodeStates[0] = 1; break;
        case 16:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[2];
          anodeStates[1] = 1; break;
        case 17:
          anodeStates[1] = 0;
          indiDigits[2] = newTime[2];
          anodeStates[2] = 1; break;
        case 18:
          indiDigits[0] = newTime[1];
          anodeStates[0] = 1; break;
        case 19:
          anodeStates[0] = 0;
          indiDigits[1] = newTime[1];
          anodeStates[1] = 1; break;
        case 20:
          indiDigits[0] = newTime[0];
          anodeStates[0] = 1; break;
        case 21:
          flipInit = false;
          newTimeFlag = false;
          #if TEMP_HUM_SENSOR
            modeTimer.reset();
          #endif
      }
    }
  }

// --- glitchy flip --- //
  else if (FLIP_EFFECT == 6) {
    if (!flipInit) {
      flipInit = true;
      // запоминаем, какие цифры поменялись и будем менять их яркость
      for (byte i = 0; i < 4; i++) {
        oldTime[i] = indiDigits[i];
        if (indiDigits[i] != newTime[i]) flipIndics[i] = true;
        else flipIndics[i] = false;
      }
      indiBrightDirection = 0;
      indiBrightCounter = 0;
      flipGlitchMax = random(3, 7);
      flipTimer.setInterval(random(1, 6) * FLIP_SPEED[FLIP_EFFECT]);
      flipTimer.reset();
    }
    if (flipTimer.isReady()) {
      for (byte i = 0; i < 4; i++)
        if (flipIndics[i]) {
          if (indiBrightDirection) indiDigits[i] = newTime[i];
          else indiDigits[i] = oldTime[i];
        }
      indiBrightDirection = !indiBrightDirection;
      flipTimer.setInterval(random(1, 6) * FLIP_SPEED[FLIP_EFFECT]);
      indiBrightCounter++;
      if (indiBrightCounter > flipGlitchMax) {
        //flipTimer.setInterval(random(GLITCH_MIN * 1000L, GLITCH_MAX * 1000L));
	  	flipInit = false;
        newTimeFlag = false;
		sendTime(hrs, mins); //effect can stop at indiDigits[i] = oldTime[i], so newTime should be send
        #if TEMP_HUM_SENSOR
          modeTimer.reset();
        #endif
      }
    }
  }

/*// --- binary like elastic band --- //
  else if (FLIP_EFFECT == 6) {
    if (!flipInit) {
      flipInit = true;
      flipEffectStages = 0;
      flipTimer.reset();
    }
    if (flipTimer.isReady()) {
      switch (flipEffectStages++) {
        case 1:
          anodeStates[3] = 0; break;
        case 2:
          anodeStates[2] = 0;
          anodeStates[3] = 1; break;
        case 3:
          anodeStates[3] = 0;
          anodeStates[1] = 0;
          anodeStates[2] = 1; break;
        case 4:
          anodeStates[2] = 0;
          anodeStates[3] = 1;
          anodeStates[0] = 0;
          anodeStates[1] = 1; break;
        case 5:
          anodeStates[3] = 0;
          anodeStates[1] = 0;
          anodeStates[2] = 1; break;
        case 6:
          anodeStates[2] = 0;
          anodeStates[3] = 1; break;
        case 7:
          anodeStates[3] = 0;
          sendTime(hrs,mins);
          anodeStates[0] = 1; break;
        case 8:
          anodeStates[0] = 0;
          anodeStates[1] = 1; break;
        case 9:
          anodeStates[1] = 0;
          anodeStates[2] = 1;
          anodeStates[0] = 1; break;
        case 10:
          anodeStates[2] = 0;
          anodeStates[3] = 1;
          anodeStates[0] = 0;
          anodeStates[1] = 1; break;
        case 11:
          anodeStates[1] = 0;
          anodeStates[2] = 1;
          anodeStates[0] = 1; break;
        case 12:
          anodeStates[0] = 0;
          anodeStates[1] = 1; break;
        case 13:
          anodeStates[0] = 1; break;
        case 14:
          flipInit = false;
          newTimeFlag = false;
          #if TEMP_HUM_SENSOR
            modeTimer.reset();
          #endif
      }
    }
  }
*/

}
