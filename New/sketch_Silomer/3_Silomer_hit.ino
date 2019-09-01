
//Функция удара
void hit() {
  //Если удар сильнее помех + заданный % от помех
  if (analogRead (piez_pin)  > stray + (stray * interferenceFilter / 100)) {
    //Обнуляем счет на дисплее
    disp = 0;
    //Устанавливаем последний МАКСИМАЛЬНЫЙ синал с пьезика на 0 (даем цифру обязательно!). Unsigned long потому что далее формулы работают со сбоями если стоит тип int. Сюдаже вписываем нужное число (0-1023), если занимаемся отладкой.
    unsigned long piez_max = 0;
    //Отсечка для таймера
    tm = millis();
    //Пока таймер работает ищем максимальный пришедший сигнал с пьезика
    do {
      piez = analogRead(piez_pin);
      if (piez_max < piez) {
        piez_max = piez;
      }
    } while (millis() - tm < hit_tm);
    //ВНИМАНИЕ! Важный кусок автокаллибровки максимальной силы удара. Если нанесен удар выше выставленной граници то граница поднимаеться до показателя силы удара
    if (calibrateHighInWork && exp_piez_max < piez_max) {
      exp_piez_max = piez_max;
    }
    //КОСТЫЛЬ! Я хз почему, но просто вставить число 100 не получаеться, надо обязательно присваивать ему тип! Короч, это увеличивает точность подсчетов.
    unsigned long k = 100;
    //Расчет максимальной силы для этого уровня сложности
    unsigned long hit_max = exp_piez_max * k / 9 * dif / k;
    //Расчет максимального счета отображаемого на дисплее
    unsigned long disp_max = piez_max * 10000 / hit_max;
    //Функция создания красивых чисел, прибавляем рандомное число в пределах 1% к счету
    if (rndScore) {
      disp_max = disp_max + random(disp_max / 100);
    }
    //Убираем перевал за счет 9999
    if (disp_max > 9999) {
      disp_max = 9999;
    }
    //Шаг прибавления счета на дисплее
    unsigned long disp_step = disp_max / (result_tm * 1000 / step_tm);
    //Расчет числа включаемых лампочек
    unsigned long light_max = piez_max / (hit_max / (numberOfLight));
    //Убираем перевалы лампочек за numberOfLight и заодно гарантируем что при макс. счете будут макс. лампочки (есть случаи когда это не так из за округлений в расчетах)
    if (light_max > numberOfLight || disp_max == 9999) {
      light_max = numberOfLight;
    }
    //А теперь убираем баг, когда при крайних к максимум значениях счет еще не дошле до 9999, но горит уже макс. лампочек
    if (disp_max < 9999 && light_max == numberOfLight) {
      light_max = numberOfLight - 1;
    }
    //Шаг времени для включения лампочек
    unsigned long lightTm = result_tm * 1000 / light_max;
    //Отсечка для таймеров
    tm = millis();
    tm2 = millis();
    //Номер включаемой лампочки
    byte lightHigh = 0;
    do {
      if (millis() - tm > step_tm) {
        //Увеличиваем следующее число на экране на 1 шаг
        disp += disp_step;
        //Бегущие цифорки примерно отображающие правильный счет
        (disp > 9999) ? displaySigns(9999) : displaySigns(disp);
        //КОСТЫЛЬ! Библиотека фетов и дисплея совместно работают криво, вручную закрываем прием сигнала на дисплее
        digitalWrite (disp_pin, LOW);
        tm = millis();
      }
      if (millis() - tm2 > lightTm) {
        //Зажигаем определенную лампочку, если значение меньше 8 (кол-во реле на модеуле), то на первом модуле, если больше то на втором
        (lightHigh < 8) ? mosfet.digitalWrite (0, lightHigh, HIGH) : mosfet.digitalWrite (1, (lightHigh - 8), HIGH);
        //Переключаемся на следующее реле
        lightHigh++;
        tm2 = millis();
      }
    } while (disp <= disp_max);
    //Корректно отображаем финальный счет
    (disp_max > 9999) ? displaySigns(9999) : displaySigns(disp_max);
    //Включаем трек по количеству лампочек
    myDFPlayer.play(light_max);
    //Флаг цикла эффектов
    boolean flagEf = true;
    //Счетчик эфектов
    byte counterEf = 0;
    //Ждем окончания трека и мигаем эффектами если они есть
    while (true) {
      //Задержка для корректной работы плеера
      delay (50);
      //Эффекты в зависимсоти от настроек
      //Если эффекты еще не запускались (нужно если трек идет дольше эфектов)
      if (flagEf) {
        do {
          //Первый режим и счет 9999
          if (blinkEf == 1 && disp_max == 9999) {
            //Очищаем дисплей
            if (displayType == 0) {
              qd.displayClear();
              //Закрываем прием сигнала вручную (привет конфликт библиотек фетов и дисплея)
              digitalWrite (disp_pin, LOW);
            }
            else if (displayType == 1) {
              digitalWrite(old_display_cs_pin, LOW);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              digitalWrite(old_display_cs_pin, HIGH);
            }
            //Гасим лампы по кол-ву по одной но очень быстро
            for (byte b; b < light_max; b++) {
              (b < 8) ? mosfet.digitalWrite (0, b, LOW) : mosfet.digitalWrite (1, b - 8, LOW);
            }
            //Ждем указанное в настройках время
            delay (blinkEfTm);
            //Повторяем все тоже самое на включение
            displaySigns(disp_max);
            for (byte b; b < light_max; b++) {
              (b < 8) ? mosfet.digitalWrite (0, b, HIGH) : mosfet.digitalWrite (1, b - 8, HIGH);
            }
            //Ждем время указанное в настройках эффектов
            delay (blinkEfTm);
          }
          //Второй режим, внутри все так же как и в первом
          else if (blinkEf == 2) {
            if (displayType == 0) {
              qd.displayClear();
              digitalWrite (disp_pin, LOW);
            }
            else if (displayType == 1) {
              digitalWrite(old_display_cs_pin, LOW);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000000);
              digitalWrite(old_display_cs_pin, HIGH);
            }
            for (byte b = 0; b < light_max; b++) {
              (b < 8) ? mosfet.digitalWrite (0, b, LOW) : mosfet.digitalWrite (1, b - 8, LOW);
            }
            delay (blinkEfTm);
            displaySigns(disp_max);
            digitalWrite (disp_pin, LOW);
            for (byte b = 0; b < light_max; b++) {
              (b < 8) ? mosfet.digitalWrite (0, b, HIGH) : mosfet.digitalWrite (1, b - 8, HIGH);
            }
            delay (blinkEfTm);
            //Прибавляем счетчик эфектов, если он привышает установленную в настройках величину, больше не мигаем
            counterEf++;
            //Если кол-во срабатывания эфектов в настройках 0, но они включены, работаем пока идет трек. Тут просто не поднимаем флаг отработки эффектов.
            if (counterEfMax > 0) {
              flagEf = false;
            }
          }
        } while (counterEf <= counterEfMax - 1);
      }
      //Если все выше законченно, и пришел сигнал о том, что плеер доиграл трек
      if (digitalRead (busy_pin)) {
        delay (500);
        if (digitalRead (busy_pin)) {
          break;
        }
      }
    }
    //Задержка между отработкой всей программы и началом нового цикла приема сигнала
    delay (500);
    //Ставим дисплей на 0 и гасим лампочки
    displaySigns(0);
    //КОСТЫЛЬ!Вручную управляем CS дисплея, так как при работе через библоиотеку в буфер лезет лишний кусок от мосфетов.
    digitalWrite (disp_pin, LOW);
    mosfet.digitalWrite (ALL, ALL, LOW);

    //Маленький доп.
    //Возможность переключить режим в случае цикличиского срабатывания удара
    //Отсечка таймера
    tm = millis();
    //В течении N времени
    do {
      //Пишем надпись
      if (displayType == 0) {
        qd.displayDigits(QD_MINUS, QD_MINUS, QD_MINUS, QD_MINUS);
      }
      else if (displayType == 1) {
        //Делаем надпись ---- вручную
        digitalWrite(old_display_cs_pin, LOW);
        shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000010); //-
        shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000010); //-
        shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000010); //-
        shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00000010); //-
        digitalWrite(old_display_cs_pin, HIGH);

      }
      //Опрашиваем кнопки
      buttonSelect.tick();
      buttonMinus.tick();
      buttonPlus.tick();
    } while (millis() - tm < 1000);
  }
}
