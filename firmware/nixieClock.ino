/*
  Based on https://alexgyver.ru/nixieclock_v2/
  Bugs fixed, alarm function and DHT22 support added.
*/
/*
    - Hold "SET" - switch between modes: "clock", "set alarm", "set clock".
    - Click "SET" in "set alarm" and "set clock" modes - change between setting hours and minutes.
    - Double click "SET" in "set alarm" mode - exit to "show clock" mode.
    - Click "SET" while alarm ring - turn alarm off.
Effects:
	- Click "-" in "clock" mode - change backlight mode: breath, always on, off.
	- Click "+" in "clock" mode - change effects of digits appearance: no effect, smooth fading, rewind in order of number, rewind in order of cathode.
    - Hold "-" - turn the "glitches" on/off.
*/
// если часы отстают, после синхронизации с RTC (раз в полчаса) какое-то время проскакивается - в этот промежуток может быть будильник - тогда он не сработает

// ************************** НАСТРОЙКИ **************************
#define BOARD_TYPE 2
// тип платы часов:
// 0 - IN-12 turned (индикаторы стоят правильно)
// 1 - IN-12 (индикаторы перевёрнуты)
// 2 - IN-14 (обычная и neon dot)
// 3 другие индикаторы
#define TUMBLER 0 // есть ли тумблер на плате

#define DUTY 200        // скважность ШИМ. От скважности зависит напряжение! у меня 175 вольт при значении 180 и 145 вольт при 120

// ---------- ЭФФЕКТЫ ----------
// эффекты перелистывания часов
byte FLIP_EFFECT = 1;
// Выбранный активен при запуске и меняется кнопками
// 0 - нет эффекта
// 1 - плавное угасание и появление (рекомендуемая скорость: 100-150)
// 2 - перемотка по порядку числа (рекомендуемая скорость: 50-80)
// 3 - перемотка по порядку катодов в лампе (рекомендуемая скорость: 30-50)

#define FLIP_SPEED_1 130    // скорость эффекта 1, мс
#define FLIP_SPEED_2 50     // скорость эффекта 2, мс
#define FLIP_SPEED_3 40     // скорость эффекта 3, мс

// эффекты подсветки
byte BACKL_MODE = 0;
// Выбранный активен при запуске и меняется кнопками
// 0 - дыхание
// 1 - постоянный свет
// 2 - выключена

// ---------- ЯРКОСТЬ ----------
#define NIGHT_LIGHT 1		// менять яркость от времени суток (1 вкл, 0 выкл)
#define NIGHT_START 23		// час перехода на ночную подсветку (BRIGHT_N)
#define NIGHT_END 7			// час перехода на дневную подсветку (BRIGHT)

#define INDI_BRIGHT 23		// яркость цифр дневная (0 - 24) !на 24 могут быть фантомные цифры!
#define INDI_BRIGHT_N 2		// яркость ночная (0 - 24)

#define DOT_BRIGHT 10		// яркость точки дневная (0 - 255)
#define DOT_BRIGHT_N 3		// яркость точки ночная (0 - 255)

#define BACKL_BRIGHT 180	// яркость подсветки ламп дневная (0 - 255)
#define BACKL_BRIGHT_N 30	// яркость подсветки ламп ночная (0 - 255)
#define BACKL_PAUSE 600		// пауза "темноты" между вспышками подсветки, мс

// ----------- ГЛЮКИ -----------
boolean GLITCH_ALLOWED = 1;	// 1 - включить, 0 - выключить глюки. Управляется кнопкой
#define GLITCH_MIN 30		// минимальное время между глюками, с
#define GLITCH_MAX 120		// максимальное время между глюками, с

// ---------- МИГАНИЕ ----------
#define DOT_TIME 500		// время мигания точки, мс
#define DOT_TIMER 100		// время горения одного шага яркости точки, мс

#define BACKL_STEP 2		// шаг мигания подсветки
#define BACKL_TIME 5000		// период подсветки, мс

// --------- БУДИЛЬНИК ---------
#define ALM_TIMEOUT 30		// таймаут будильника, с
#define FREQ 900			// частота писка будильника

// --------- DHT ---------
#define SHOW_TEMP_HUM 1		// 0 - не показывать температуру и вл., 1 - показывать
#define CLOCK_TIME 10		// время (с), которое отображаются часы
#define TEMP_TIME 3			// время (с), которое отображается температура и влажность

// --------- ДРУГОЕ --------
#define BURN_TIME 1			// период обхода в режиме очистки, мс

// пины
#define ALARM_SW 1	// тумблер будильника (при 1 - выкл (подтянут внутренним резистором к +5), 0 - вкл (заземлён))
#define PIEZO 2		// пищалка
#define KEY0 3		// часы
#define KEY1 4		// часы 
#define KEY2 5		// минуты
#define KEY3 6		// минуты
#define BTN1 7		// кнопка 1, SET
#define BTN2 8		// кнопка 2, L, -
#define GEN 9		// генератор
#define DOT 10		// точка
#define BACKL 11	// подсветка
#define BTN3 12		// кнопка 3, R, +
#define DHT_DATA 13 // DHT data pin

// дешифратор
#define DECODER0 A0
#define DECODER1 A1
#define DECODER2 A2
#define DECODER3 A3

// распиновка ламп
#if (BOARD_TYPE == 0)
byte digitMask[] = {7, 3, 6, 4, 1, 9, 8, 0, 5, 2};   // маска дешифратора платы in12_turned (цифры нормальные)
byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // порядок индикаторов слева направо
byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 1)
byte digitMask[] = {2, 8, 1, 9, 6, 4, 3, 5, 0, 7};   // маска дешифратора платы in12 (цифры вверх ногами)
byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned и ин-14)
byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 2)
byte digitMask[] = {9, 8, 0, 5, 4, 7, 3, 6, 2, 1};   // маска дешифратора платы in14
byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned и ин-14)
byte cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6}; // порядок катодов in14

#elif (BOARD_TYPE == 3)
byte digitMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};   // тут вводим свой порядок пинов
byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // свой порядок индикаторов
byte cathodeMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; // и свой порядок катодов

#endif

// библиотеки
#include <GyverHacks.h>
#include <GyverTimer.h>
#include <GyverButton.h>
#include <Wire.h>
#include <RTClib.h>
#include "EEPROMex.h"
#include "DHT.h"
#include "NewTone.h"

RTC_DS3231 rtc;
DHT dht(DHT_DATA, DHT22);

// таймеры
GTimer_ms dotTimer(500);                // полсекундный таймер для часов
GTimer_ms dotBrightTimer(DOT_TIMER);    // таймер шага яркости точки
GTimer_ms backlBrightTimer(30);         // таймер шага яркости подсветки
GTimer_ms almTimer((long)ALM_TIMEOUT * 1000);
GTimer_ms flipTimer(FLIP_SPEED_1);
GTimer_ms glitchTimer(1000);
GTimer_ms blinkTimer(500);
GTimer_ms modeTimer((long)CLOCK_TIME * 1000);

// кнопки
GButton btnSet(BTN1, HIGH_PULL, NORM_OPEN);
GButton btnL(BTN2, HIGH_PULL, NORM_OPEN);
GButton btnR(BTN3, HIGH_PULL, NORM_OPEN);

// переменные
volatile int8_t indiDimm[4];      // величина диммирования (яркость свечения индикатора) (0-24)
volatile int8_t indiCounter[4];   // счётчик каждого индикатора (0-24)
volatile int8_t indiDigits[4];    // цифры, которые должны показать индикаторы (0-10)
volatile int8_t curIndi;          // текущий индикатор (0-3)

int8_t hrs, mins, secs;
int8_t alm_hrs = 24, alm_mins = 0; // 24 - alarm is OFF
// int8_t mode = 0;    // 0 часы, 3 температура, 1 настройка будильника, 2 настройка часов, 4 аларм
bool blinkFlag;
byte indiMaxBright = INDI_BRIGHT, dotMaxBright = DOT_BRIGHT, backlMaxBright = BACKL_BRIGHT;
bool dotFlag = true, alm_flag = false, flTurnAlarmOff = false;
bool dotBrightFlag, dotBrightDirection, backlBrightFlag, backlBrightDirection, indiBrightDirection;
int dotBrightCounter, backlBrightCounter, indiBrightCounter;
byte dotBrightStep;		// один шаг (длительностью DOT_TIMER) изменения яркости точки из одного цикла полного зажигания/угасания DOT_TIME в течение DOT_TIMER
bool newTimeFlag;
bool flipIndics[4];
byte newTime[4];
bool flipInit;
byte startCathode[4], endCathode[4];
byte glitchCounter, glitchMax, glitchIndic;
bool glitchFlag, indiState;
byte curMode = 0; // 0 - часы, 1 - настройка будильника, 2 - настройка часов, 3 - темп и влажность
bool currentDigit = false; // настройка часов (false) или минут (true)
int8_t changeHrs, changeMins;
bool lampState = false;
bool anodeStates[] = {1, 1, 1, 1};
bool sendTone;

void setDig(byte digit) {
  digit = digitMask[digit];
  setPin(DECODER3, bitRead(digit, 0));
  setPin(DECODER1, bitRead(digit, 1));
  setPin(DECODER0, bitRead(digit, 2));
  setPin(DECODER2, bitRead(digit, 3));
}

void setup() {
  //Serial.begin(9600);
  // случайное зерно для генератора случайных чисел
  randomSeed(analogRead(6) + analogRead(7));

  // настройка пинов
  pinMode(DECODER0, OUTPUT);
  pinMode(DECODER1, OUTPUT);
  pinMode(DECODER2, OUTPUT);
  pinMode(DECODER3, OUTPUT);
  pinMode(KEY0, OUTPUT);
  pinMode(KEY1, OUTPUT);
  pinMode(KEY2, OUTPUT);
  pinMode(KEY3, OUTPUT);
  pinMode(PIEZO, OUTPUT);
  pinMode(GEN, OUTPUT);
  pinMode(DOT, OUTPUT);
  pinMode(BACKL, OUTPUT);
  pinMode(ALARM_SW, INPUT_PULLUP);

  // задаем частоту ШИМ на 9 и 10 выводах 31 кГц
  TCCR1B = TCCR1B & 0b11111000 | 1;		// ставим делитель 1
  // включаем ШИМ  
  setPWM(9, DUTY);
  // перенастраиваем частоту ШИМ на пинах 3 и 11 на 7.8 кГц и разрешаем прерывания COMPA
  TCCR2B = (TCCR2B & B11111000) | 2;    // делитель 8
  TCCR2A |= (1 << WGM21);   // включить CTC режим для COMPA
  TIMSK2 |= (1 << OCIE2A);  // включить прерывания по совпадению COMPA

  // ---------- DHT ----------
  dht.begin();

  // ---------- RTC -----------
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  almTimer.stop();

  if (EEPROM.readByte(100) != 77) {   // проверка на первый запуск. 77 от балды
    EEPROM.writeByte(100, 77);
    EEPROM.writeByte(0, 24);     // часы будильника - 24 = будильник выкл
    EEPROM.writeByte(1, 30);     // минуты будильника
    }
  alm_hrs = EEPROM.readByte(0);
  alm_mins = EEPROM.readByte(1);

  sendTime(hrs, mins);  // отправить время на индикаторы
  if (NIGHT_LIGHT == 0) {
    // установить яркость на индикаторы
    for (byte i = 0; i < 4; i++)
      indiDimm[i] = indiMaxBright;
    // расчёт шага яркости точки
    dotBrightStep = ceil((float)dotMaxBright * 2 / DOT_TIME * DOT_TIMER);
    if (dotBrightStep == 0) dotBrightStep = 1;
    // дыхание подсветки
    backlBrightTimer.setInterval((float)BACKL_STEP / backlMaxBright / 2 * BACKL_TIME);
    indiBrightCounter = indiMaxBright;
   }
   else changeBright();       // всё то же самое, но в функции, которая компилится только если NIGHT_LIGHT - изменить яркость согласно времени суток

  // стартовый период глюков
  glitchTimer.setInterval(random(GLITCH_MIN * 1000L, GLITCH_MAX * 1000L));

  // скорость режима при запуске
  switch (FLIP_EFFECT) {
    case 0:
      break;
    case 1: flipTimer.setInterval(FLIP_SPEED_1);
      break;
    case 2: flipTimer.setInterval(FLIP_SPEED_2);
      break;
    case 3: flipTimer.setInterval(FLIP_SPEED_3);
      break;
  }
}

void loop() {
  if (dotTimer.isReady() /* && (curMode == 0 || curMode == 3) */) calculateTime();	// каждые 500 мс пересчёт и отправка времени
  if (newTimeFlag && curMode == 0) flipTick();						// перелистывание цифр. Устанавливает ноое время - можно только при режиме часов
  dotBrightTick();													// плавное мигание точки
  backlBrightTick();												// плавное мигание подсветки ламп
  if (GLITCH_ALLOWED && (curMode == 0 || curMode == 3) ) glitchTick();	// глюки
  buttonsTick();													// кнопки
  settingsTick();													// настройки
  if (SHOW_TEMP_HUM && modeTimer.isReady()) modeTick();
  if (alm_flag && dotFlag) { //в данном случае частоту нельзя регулировать. Она зависит от скорости выполнения loop(). Или использовать NewTone и каждый раз перенастраивать ШИМ для генератора, т. к. библиотека использует тоже использует Timer1 и перенастраивает его под себя.
	sendTone = !sendTone;
	digitalWrite(PIEZO, sendTone);
  }
}

void modeTick() {
#if SHOW_TEMP_HUM
  if (curMode == 0 && !alm_flag) {
    curMode = 3;
    byte temp = dht.readTemperature();
    byte hum = dht.readHumidity();
    sendTime(temp, hum);
	//for (byte i = 0; i < 4; i++) {indiDimm[i] = indiMaxBright;
    modeTimer.setInterval((long)TEMP_TIME * 1000);
  }
  else if (curMode == 3) {
    curMode = 0;
    sendTime(hrs,mins);
    modeTimer.setInterval((long)CLOCK_TIME * 1000);
  }
#endif
}

void burnIndicators() {
  for (byte d = 0; d < 10; d++) {
    for (byte i = 0; i < 4; i++) {
      indiDigits[i]--;
      if (indiDigits[i] < 0) indiDigits[i] = 9;
    }
    delay(BURN_TIME);
  }
}
//проверить звонок будильника - звенит ли с analogWrite?

/*
  ард ног ном
  А0  7   4
  А1  6   2
  А2  4   8
  А3  3   1
*/
