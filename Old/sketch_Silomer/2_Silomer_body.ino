

//Порты
//Если код компилируют под Mega (ВНИМАНИЕ!!! Mega работает крайне нестабильно! Часто зависате! Рекомендуется использовать Uno/Nano)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define rx_pin              10   //Для общения с плеером (RX ардуинки должен идти на TX плеера) (белый)
#define tx_pin              11   //Для общения с плеером (TX ардуинки должен идти на RX плеера) (желтый)
#define busy_pin            9   //Сигнал воспроизведения с плеера
#define select_button_pin   22  //Кнопка select
#define minus_button_pin    24  //Кнопка -
#define plus_button_pin     26  //Кнопка +
#define fet_pin             30   //CS пин мосфетов
#define disp_pin            31   //CS пин экрана
#define mosi_pin            51  //MOSI (DI пин на модуле) (общий для устройств на SPI)
#define miso_pin            50  //MISO (DO пин на модуле) (общий для устройств на SPI)
#define sck_pin             52  //SCK (_П_ пин на модуле) (общий для устройств на SPI)
#define piez_pin            54  //Пин пьезика (A0 на плате)
#define old_display_mo_pin  23  //Mosi пин старого дисплея  (желтый)    (4й от GND)(На уно А1)
#define old_display_mi_pin  25  //Miso пин старого дисплея  (оранжевый) (3й от GND)(На уно А2)
#define old_display_cs_pin  27  //CS пин старого дисплея    (зеленый)   (2й от GND)(На уно А3)
 
//Если код компилируют под UNO\Nano\Pro Mini
#elif defined(__AVR_ATmega328P__)

#define rx_pin              2   //Для общения с плеером (RX ардуинки должен идти на TX плеера) (белый)
#define tx_pin              3   //Для общения с плеером (TX ардуинки должен идти на RX плеера) (желтый)
#define busy_pin            4   //Сигнал воспроизведения с плеера
#define select_button_pin   5  //Кнопка select
#define minus_button_pin    6  //Кнопка -
#define plus_button_pin     7  //Кнопка +
#define fet_pin             8   //CS пин мосфетов
#define disp_pin            9   //CS пин экрана
#define mosi_pin            11  //MOSI (DI пин на модуле) (общий для устройств на SPI)
#define miso_pin            12  //MISO (DO пин на модуле) (общий для устройств на SPI)
#define sck_pin             13  //SCK (_П_ пин на модуле) (общий для устройств на SPI)
#define piez_pin            14  //Пин пьезика (A0 на плате)
#define old_display_mo_pin  15  //Mosi пин старого дисплея  (желтый)    (4й от GND)(На уно А1)
#define old_display_mi_pin  16  //Miso пин старого дисплея  (оранжевый) (3й от GND)(На уно А2)
#define old_display_cs_pin  17  //CS пин старого дисплея    (зеленый)   (2й от GND)(На уно А3) 

#endif


//Библиотеки
#include "SPI.h"                  //Библиотека для работы с модулями по SPI
#include "SoftwareSerial.h"       //Библиотека для работы с модулями по UART
#include "AmperkaFET.h"           //Библиотека мосфетов
#include "QuadDisplay2.h"         //Библиотека дисплея
#include "DFRobotDFPlayerMini.h"  //Библиотека плеера
#include "OneButton.h"            //Библиотека кнопок

//Объекты
SoftwareSerial mySoftwareSerial(rx_pin, tx_pin); //Дополнительные пины UART (rx, tx)
FET mosfet(fet_pin, 2);                          //CS пин мофсфета и количество модулей
QuadDisplay qd(disp_pin);                        //CS пин дисплея
DFRobotDFPlayerMini myDFPlayer;                  //Плеер
OneButton buttonSelect(select_button_pin, true); //Кнопка select (порт, подтяжка к земле)
OneButton buttonMinus(minus_button_pin, true);   //Кнопка - (порт, подтяжка к земле)
OneButton buttonPlus(plus_button_pin, true);     //Кнопка + (порт, подтяжка к земле)


//Переменные
boolean modeFlag = 0;               //Флажок для работы в функциях
boolean lightOnOff [2] = {0, 0};    //Мигалка для ледов сложности
byte dif = Dlevel;                  //Множитель уровня сложности (1-мин. сложность, 9-макс. сложность)
byte mode = 0;                      //Режим работы силомера (0-игра + сложность, 1 сигнал пьезика + автофильтры, 2 помехи + ручной фильтр помех, 3 макс. сигнал пьезика + ручная настройка этого сигнала)
int stray = 0;                      //Максимальный сигнал на пьезике во время последней калибровки (уровень помех)
int piez = 0;                       //Сигнал с пьезика
int disp = 0;                       //Число на дисплей
int exp_piez_max = hitHight;        //Ожидаемый реальный максимальный сигнал с пьезика (мин. = 1, макс. = 1023). Зависит от клея на датчике, наличия и объема конденсатора, еще миллиона факторов. Устанавливаеться на основе тестов (отладка по пьезику). В глобалах потому что возможно будет динамическое изменение. На данный момент кажеться автокаллибруеться в случае привышения значения по максимальной силе удара.
volatile unsigned long tm = 0;      //Таймер 1 применяеться для определения силы удара, работы с экраном, отображения уровня сложности
volatile unsigned long tm2 = 0;     //Таймер 2 применяеться для работы с лампочками, отображения уровня сложности
volatile unsigned long tm3 = 0;     //Таймер 3 для взаимодействия с функциями mode1-3


void setup() {

  //Настройки

  //Связь по UART
  Serial.begin(115200);                          //Порт для отладки
  mySoftwareSerial.begin(9600);                  //Связь с плеером

  //Предотвращает перезагрузку при работе плеера (что б я знал почему это работает)
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    pinMode (13, OUTPUT);
    digitalWrite (13, HIGH);
    Serial.println ("Проблемы с мп3 модулем");
    Serial.println ("Ну или перепутанно RX и TX на вход в плату");
    Serial.println ("Или ты забыл вставить карточку");
    Serial.println ("Или где то что то не контачит");
    Serial.println ("Или что то еще...");
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }

  //Инициализируем порты
  pinMode (piez_pin,  INPUT);
  pinMode (busy_pin,  INPUT);
  pinMode (old_display_mi_pin, OUTPUT);
  pinMode (old_display_mo_pin, OUTPUT);
  pinMode (old_display_cs_pin, OUTPUT);

  //Включаем модули
  mosfet.begin();                                              //Модуль фетов
  qd.begin();                                                  //Модуль дисплея
  myDFPlayer.volume(vol);                                      //Тянем из настроек громкость
  switch (eql) {                                               //Тянем из настроек эквалайзер
    case 0: myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);  break;
    case 1: myDFPlayer.EQ(DFPLAYER_EQ_POP);     break;
    case 2: myDFPlayer.EQ(DFPLAYER_EQ_ROCK);    break;
    case 3: myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);    break;
    case 4: myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC); break;
    case 5: myDFPlayer.EQ(DFPLAYER_EQ_BASS);    break;
  }
  //Настройки кнопок
  //Уменьшаем время между одиночными кликами, что бы не проскакивало двойное нажатие.
  buttonSelect.setClickTicks (50);
  buttonMinus.setClickTicks (50);
  buttonPlus.setClickTicks (50);
  //Ставим функции на клик
  buttonSelect.attachClick(clickSelect);
  buttonMinus.attachClick(clickMinus);
  buttonPlus.attachClick(clickPlus);
  //Ставим функции на удержание. Только для - и +.
  buttonMinus.attachDuringLongPress(longPressMinus);
  buttonPlus.attachDuringLongPress(longPressPlus);


  if (chek) {
    //Включаем леды уровня сложности
    mosfet.digitalWrite (1, 5, HIGH);
    mosfet.digitalWrite (1, 6, HIGH);
    mosfet.digitalWrite (1, 7, HIGH);

    //Поочередно включаем лампочки на шкале силы удара и паралельно просматриваем на дисплее все числа (без точек)
    //Отсечка для таймера
    tm = millis();
    //Номер включаемой лампочки
    byte lightNow = 0;
    //Раз в пол секунды прибавляем лампочку и счет
    while (true) {
      //Если лампочек больше 8 (кол-во выходов на модуле) Перехдим на новый модуль и вычитаем кол-во уже зажженых лампочек из номера включаемого порта
      (lightNow < 8) ? mosfet.digitalWrite (0, lightNow, HIGH) : mosfet.digitalWrite (1, (lightNow - 8), HIGH);
      if (millis() - tm > 500) {
        //- 1 потому что кол-во лампочек считаеться от 1, а порты от 0
        if (numberOfLight - 1 > lightNow) {
          lightNow++;
        }
        if (disp != 9999) {
          disp += 1111;
        }
        tm = millis();
      }
      displaySigns(disp);
      //Если лампочки и дисплей набрали свои макс. значения, завершаем проверку и гасим лампочки
      if (numberOfLight - 1 <= lightNow && disp == 9999) {
        (lightNow < 8) ? mosfet.digitalWrite (0, lightNow, HIGH) : mosfet.digitalWrite (1, (lightNow - 8), HIGH);
        delay (500);
        break;
      }
    }

    //Проигрываем вступительную фразу
    while (true) {
      //Включаем трек
      if (digitalRead (busy_pin)) {
        myDFPlayer.play(13);
      }
      //Ставим паузу >=50 что бы плеер корректно передал сигнал
      delay(50);
      //По завершению трека продолжаем программу
      if (digitalRead (busy_pin) == 1) {
        break;
      }
    }

    //Гасим все леды
    mosfet.digitalWrite (ALL, ALL, LOW);
    delay (1000);
  }
  //Запускаем калибровку, если включена в настройках
  //Помехи
  if (calibrateLow) {
    piezo_calibrateLow();
  }
  //Верхний предел
  if (calibrateHigh) {
    piezo_calibrateHigh();
  }
}


void loop() {
  //Ищем срабатывание кнопки
  buttonSelect.tick();
  buttonMinus.tick();
  buttonPlus.tick();
  //Действуем в соответсвии с режимом, везде ставим функцию уровня сложности, в игровом (нулевом) режиме все время сбрасываем дисплей на 0.
  switch (mode) {
    case 0: hit(); difLvl(); displaySigns(0); break;
    case 1: mode1(); difLvl(); break;
    case 2: mode2(); difLvl(); break;
    case 3: mode3(); difLvl(); break;

  }
}
