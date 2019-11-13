/*
  Based on https://github.com/AlexGyver/NixieClock_v2
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
    - Hold "+" - turn the "show temp" on/off.
*/

/* EEPROM map
0 - alm_hrs
1 - alm_mins
2 - FLIP_EFFECT
3 - BACKL_MODE
4 - GLITCH_ALLOWED
5 - TEMPHUM_ALLOWED
*/

// ************************** SETTINGS **************************
#define BOARD_TYPE 2
// тип платы часов:
// 0 - IN-12 turned (индикаторы стоят правильно)
// 1 - IN-12 (индикаторы перевёрнуты)
// 2 - IN-14 (обычная и neon dot)
// 3 другие индикаторы

#define DUTY 180        // PWM duty. Voltage depends on it. It should be ~175 V on electrolytic capacitor after connecting the load

// ---------- EFFECTS ----------
// 0 - no effect,
// 1 - smooth fading (recommended speed: 100-150)
// 2 - rewind in order of number (recommended speed: 50-80)
// 3 - rewind in order of cathode (recommended speed: 30-50)
// 4 - train (recommended speed: 110-150)
// 5 - elastic band (recommended speed: 70-120)
#define CHECK_EFFECTS 0 //timeTicker.ino - resets newTimeFlag every 10 secs in lines 8-10
byte FLIP_EFFECT = 1; // effects of digits appearance
const byte FLIP_SPEED[] = {0, 130, 50, 40, 130, 80}; //ms //, 120};
const byte FLIP_EFFECT_NUM = 6; // the quantity of effect. Should be equal to size of FLIP_SPEED array

byte BACKL_MODE = 0; 		//backlight mode: 0 - breath, 1 - always on, 2 - off
#define BACKL_STEP 2		//for breath mode: brightness step
#define BACKL_TIME 5000		//for breath mode: backlight period, ms

// ---------- BRIGHTNESS ----------
#define NIGHT_LIGHT 1		// night mode: 1 - on, 0 - off
#define NIGHT_START 23		// hour, when night mode switches on
#define NIGHT_END 7			// hour, when night mode switches off

#define INDI_BRIGHT 17		// daytime indicators brightness (0 - 23) - at 24 different glitches appear
#define INDI_BRIGHT_N 7		// nighttime indicators brightness  (0 - 23)

#define DOT_BRIGHT 10		// daytime dot brightness (0 - 255)
#define DOT_BRIGHT_N 3		// nighttime dot brightness (0 - 255)

#define BACKL_BRIGHT 180	// daytime backlight brightness (0 - 255)
#define BACKL_BRIGHT_N 30	// nighttime backlight brightness (0 - 255))
#define BACKL_PAUSE 600		// delay (=dark) between backlight flashes, ms

// ----------- GLITCHES -----------
boolean GLITCH_ALLOWED = 1;	// glitches: 1 - on, 0 - off. Could be changed by holding L/-
#define GLITCH_MIN 30		// min time between glitches, s
#define GLITCH_MAX 120		// max time between glitches, s

// ---------- DOT FLASHING ----------
#define DOT_TIME 500		// time of dot duty cycle, ms
#define DOT_TIMER 100		// time of one brightness step of dot duty cycle, ms. So step = DOT_TIME / DOT_TIMER

// --------- ALARM ---------
#define ALM_TIMEOUT 30		// alarm timeout, s
#define FREQ 900			// buzzer frequency, is applicable only if buzzer is passive and NewTone library is used.
#define TUMBLER 0		// is there tumbler on board
#define BUZZER_PASSIVE 1	// 0 - buzzer is active, 1 - is passive. There are 2 methods of alarming in case of passive buzzer: using NewTone library with ability to control frequency FREQ (in this case every time PWM on Timer1 should be reset as pins 3 and 9 use Timer1) and using main loop as "frequency generator", so frequency could not be adjusted and depends on main loop execution time

// --------- DHT ---------
#define TEMP_HUM_SENSOR 1		// is there a DHT22 sensor on board. IN CASE OF CHANGING, EEPROM SHOULD BE REINIT - directly coded or by setting with Hold "+" (turn the "show temp" on/off).
bool TEMPHUM_ALLOWED = TEMP_HUM_SENSOR;	// "show temp/hum" mode: 1 - on, 0 - off. Could be changed by holding R/+
#define CLOCK_TIME 10		// "show time" mode duration, s
#define TEMP_TIME 3			// "show temp" mode duration, s - min 2 s - DHT sensor could be read only once in 2 seconds

// --------- OTHER --------
#define BURN_TIME 5			// период обхода в режиме очистки, мс

// пины
#define ALARM_SW 1	// alarm switcher (tumbler): 1 - off (pulled up internally), 0 - on (grounded)
#define PIEZO 2		// buzzer
#define KEY0 3		// hours
#define KEY1 4		// hours
#define KEY2 5		// minutes
#define KEY3 6		// minutes
#define BTN1 7		// button 1, SET
#define BTN2 8		// button 2, L, -
#define GEN 9		// generator
#define DOT 10		// dot
#define BACKL 11	// backlight
#define BTN3 12		// button 3, R, +
#define DHT_DATA 13 // DHT data pin

// дешифратор
#define DECODER0 A0
#define DECODER1 A1
#define DECODER2 A2
#define DECODER3 A3

// распиновка ламп
#if (BOARD_TYPE == 0)
const byte digitMask[] = {7, 3, 6, 4, 1, 9, 8, 0, 5, 2};   // маска дешифратора платы in12_turned (цифры нормальные)
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // порядок индикаторов слева направо
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 1)
const byte digitMask[] = {2, 8, 1, 9, 6, 4, 3, 5, 0, 7};   // маска дешифратора платы in12 (цифры вверх ногами)
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned и ин-14)
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // порядок катодов in12

#elif (BOARD_TYPE == 2)
const byte digitMask[] = {9, 8, 0, 5, 4, 7, 3, 6, 2, 1};   // маска дешифратора платы in14
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // порядок индикаторов справа налево (для IN-12 turned и ин-14)
const byte cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6}; // порядок катодов in14

#elif (BOARD_TYPE == 3)
const byte digitMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};   // тут вводим свой порядок пинов
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // свой порядок индикаторов
const byte cathodeMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; // и свой порядок катодов

#endif

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

// timers
GTimer_ms dotTimer(500);                // полсекундный таймер для часов
GTimer_ms dotBrightTimer(DOT_TIMER);    // таймер шага яркости точки
GTimer_ms backlBrightTimer(30);         // таймер шага яркости подсветки
GTimer_ms almTimer((long)ALM_TIMEOUT * 1000);
GTimer_ms flipTimer(FLIP_SPEED[FLIP_EFFECT]);
GTimer_ms glitchTimer(1000);
GTimer_ms blinkTimer(500);
GTimer_ms modeTimer((long)CLOCK_TIME * 1000);

// buttons
GButton btnSet(BTN1, HIGH_PULL, NORM_OPEN);
GButton btnL(BTN2, HIGH_PULL, NORM_OPEN);
GButton btnR(BTN3, HIGH_PULL, NORM_OPEN);

volatile int8_t indiDimm[4];      // величина диммирования (яркость свечения индикатора) (0-24)
volatile int8_t indiCounter[4];   // счётчик каждого индикатора (0-24)
volatile int8_t indiDigits[4];    // цифры, которые должны показать индикаторы (0-10)
volatile int8_t curIndi;          // текущий индикатор (0-3)

int8_t hrs, mins, secs;
int8_t alm_hrs, alm_mins; //alm_hrs = 24 - alarm is OFF
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
byte currentLamp, flipEffectStages;
bool trainLeaving;

void setDig(byte digit) {
  digit = digitMask[digit];
  setPin(DECODER3, bitRead(digit, 0));
  setPin(DECODER1, bitRead(digit, 1));
  setPin(DECODER0, bitRead(digit, 2));
  setPin(DECODER2, bitRead(digit, 3));
}

void setup() {
  //Serial.begin(9600);
  randomSeed(analogRead(6) + analogRead(7)); //for glitches

  //pins setup
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
    EEPROM.writeByte(2, FLIP_EFFECT);
    EEPROM.writeByte(3, BACKL_MODE);
    EEPROM.writeByte(4, GLITCH_ALLOWED);
	EEPROM.writeByte(5, TEMPHUM_ALLOWED);
  }
    
  alm_hrs = EEPROM.readByte(0);
  alm_mins = EEPROM.readByte(1);
  FLIP_EFFECT = EEPROM.readByte(2);
  BACKL_MODE = EEPROM.readByte(3);
  GLITCH_ALLOWED = EEPROM.readByte(4);
  TEMPHUM_ALLOWED = EEPROM.readByte(5);

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
  flipTimer.setInterval(FLIP_SPEED[FLIP_EFFECT]);
}

void loop() {
  if (dotTimer.isReady()) calculateTime();							// каждые 500 мс пересчёт и отправка времени
  if (newTimeFlag && curMode == 0) flipTick();						// перелистывание цифр. Устанавливает новое время - можно только при режиме часов
  dotBrightTick();													// плавное мигание точки
  backlBrightTick();												// плавное мигание подсветки ламп
  if (GLITCH_ALLOWED && (curMode == 0 || curMode == 3) ) glitchTick();	// глюки
  buttonsTick();													// кнопки
  settingsTick();													// настройки
#if TEMP_HUM_SENSOR
  if (!flipInit) {  // wait the end of flip effect. First, check flag and only after this check timer, otherwise timer will be resetted, but condition may be not satisfied (flipInit == true)
    if (TEMPHUM_ALLOWED && modeTimer.isReady()) modeTick();
  }
#endif
#if BUZZER_PASSIVE
  if (alm_flag && !dotFlag) {
    sendTone = !sendTone; // buzzer frequency is equal to loop frequency
    setPin(PIEZO, sendTone);
  }
#endif
}

#if TEMP_HUM_SENSOR
void modeTick() {
  if (curMode == 0 && !alm_flag) {
    curMode = 3;
    byte temp = dht.readTemperature();
    byte hum = dht.readHumidity();
    sendTime(temp, hum);
	//for (byte i = 0; i < 4; i++) {indiDimm[i] = indiMaxBright;}
    modeTimer.setInterval((long)TEMP_TIME * 1000);
  }
  else if (curMode == 3) {
    curMode = 0;
    sendTime(hrs,mins);
    modeTimer.setInterval((long)CLOCK_TIME * 1000);
  }
}
#endif

void burnIndicators() {
  for (byte d = 0; d < 10; d++) {
    for (byte i = 0; i < 4; i++) {
      indiDigits[i]--;
      if (indiDigits[i] < 0) indiDigits[i] = 9;
    }
    delay(BURN_TIME);
  }
}
