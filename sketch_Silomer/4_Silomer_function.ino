
//Функция калиборвки нижних помех
void piezo_calibrateLow() {
  //Ставми показатель помех на 0
  stray = 0;
  //Переменная читающая сигнал с пьезика
  int u;
  //Пишем StOP на экране, пока горит надпись на пьезик НЕЛЬЗЯ ДАВИТЬ!!!
  if (displayType == 0) {
    qd.displayDigits(QD_S, QD_t, QD_O, QD_P);
  }
  else if (displayType == 1) {
    //Делаем надпись StOP вручную
    digitalWrite(old_display_cs_pin, LOW);
    shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11001110); //p
    shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11111100); //o
    shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00011110); //t
    shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b10110110); //s
    digitalWrite(old_display_cs_pin, HIGH);
  }
  //В течении времени указанного в настройках (*1000, потому что там секунды), раз в 1 миллисекунду ищем и записываем максимальный сигнал с пьезика
  for (int i = 0; i < calibrate_tm * 1000; i++) {
    u = analogRead (piez_pin);
    if (stray < u) {
      stray = u;
    }
    delay(1);
  }
  //На всякий случай проверяем что бы помехи не вывалились за предпологаемую максимальную силу удара.
  if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
    exp_piez_max = stray + (stray * interferenceFilter / 100) + numberOfLight + 1;
    //Если вылетили за пределы датчика, ставим макс. значение датчика
    if (exp_piez_max > 1023) {
      exp_piez_max = 1023;
    }
  }

  //Показывает уровень откалиброванных помех
  displaySigns(stray);
  //Ждем, что бы человек мог прочитать цифры
  delay (3000);
  //На всякий случай сбрасываем экран на 0
  displaySigns(0);
}

//Функция калибровки верхнего предела
void piezo_calibrateHigh() {
  //В целом, все тоже самое что и функцией выше, но теперь вверх
  //Счетчик ударов
  byte hit = 0;
  //Цикл на 3 удара
  do {
    //Пишем (hItN)
    if (displayType == 0 ) {
      switch (hit) {
        case 0: qd.displayDigits(QD_H, QD_I, QD_t, QD_3); break;
        case 1: qd.displayDigits(QD_H, QD_I, QD_t, QD_2); break;
        case 2: qd.displayDigits(QD_H, QD_I, QD_t, QD_1); break;
      }
      digitalWrite (disp_pin, LOW);
    }
    //Пишем (hItN) вручную
    else if (displayType == 1) {
      digitalWrite(old_display_cs_pin, LOW);
      switch (hit) {
        case 0: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11110010); break;  //3
        case 1: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11011010); break;  //2
        case 2: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01100000); break;  //1
      }
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00011110); //t
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01100000); //I
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01101110); //H
      digitalWrite(old_display_cs_pin, HIGH);
    }
    //Ожидаем удар
    while (true) {
      //Если пришел удар, и он приодолел фильтры
      if (analogRead (piez_pin)  > stray + (stray * interferenceFilter / 100)) {
        //Отсечка таймера
        tm = millis();
        //Пока таймер работает ищем максимальный пришедший сигнал с пьезика и записываем его в макс. ожидаемый сигнал
        do {
          piez = analogRead(piez_pin);
          if (exp_piez_max < piez) {
            exp_piez_max = piez;
          }
        } while (millis() - tm < hit_tm);
        //Выходим из цикла по окончанию таймера
        break;
      }
    }
    //Выводим верхний предел силы удара на дисплей. Значение может не изменится, если удар был слабее максимального.
    displaySigns(exp_piez_max);
    //Ждем, что бы человек мог прочитать цифры
    delay (3000);
    //Добавляем счетчик ударов
    hit++;
    //На всякий случай проверяем что бы помехи не вывалились за предпологаемую максимальную силу удара.
    if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
      exp_piez_max = stray + (stray * interferenceFilter / 100) + numberOfLight + 1;
      //Если вылетили за пределы датчика, ставим макс. значение датчика
      if (exp_piez_max > 1023) {
        exp_piez_max = 1023;
      }
    }
    //Если пьезик и так стоит на максимум, отключаем каллибровку.
    if (exp_piez_max == 1023) {
      break;
    }
  } while (hit < 3);
}

//Функция для отображения счета.
void displaySigns(int val) {
  //Если указан дисплей от амперки
  if (displayType == 0 ) {
    //Просто пользуемся их библиотекой
    qd.displayInt(val, true);
    //Не забываем перевести cs пин в LOW, что бы небыло проблем с отображением данных (конфликт с платами реле)
    digitalWrite (disp_pin, LOW);
  }
  //Если же стоит старый дисплей, или любой дисплей на каскаде сдвиговых регистров 74HC595.
  else if (displayType == 1) {
    //Создаем массив на 4 символа (по кол-ву цифр на дисплее)
    byte sign [4];
    //Переводим число в ячейки массива методом деления на порядок и нахождением остатка от 10. Обратите внимание, что тут они забиваются задом наперед. Тоесть число 1234 по факту превращается в число 4321. Связанно это с тем, что данные в исплей тоже идут задом наперед, начиная с последней цифры.
    sign[3] = val / 1000 % 10;
    sign[2] = val / 100 % 10;
    sign[1] = val / 10 % 10;
    sign[0] = val % 10;
    //Включаем передачу данных на дисплей
    digitalWrite(old_display_cs_pin, LOW);
    //4 раза (по кол-ву цифр на экране) передаем байты
    for (int i = 0; i < 4; i++) {
      switch (sign[i]) {
        case 0: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11111100); break;
        case 1: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01100000); break;
        case 2: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11011010); break;
        case 3: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11110010); break;
        case 4: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01100110); break;
        case 5: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b10110110); break;
        case 6: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b10111110); break;
        case 7: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11100000); break;
        case 8: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11111110); break;
        case 9: shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b11110110); break;
      }
    }
    //Останавливаем передачу на дисплей
    digitalWrite(old_display_cs_pin, HIGH);
  }
}

//Фунция отображения уровня сложности
void difLvl () {
  //Таймер для редкого мигания на 1м, 4м, и 7м уровнях
  if (millis() - tm > 1000) {
    lightOnOff [0] = !lightOnOff [0];
    tm = millis();
  }
  //Таймер для частого мигания на 2м, 5м и 8м уровнях
  if (millis() - tm2 > 150) {
    lightOnOff [1] = !lightOnOff [1];
    tm2 = millis();
  }
  //В зависимости от уровня сложности выставляем разные положения релюшек на плате.
  switch (dif) {
    case 1: mosfet.digitalWrite (1, 5, lightOnOff [0]); mosfet.digitalWrite (1, 6, LOW); mosfet.digitalWrite (1, 7, LOW); break;
    case 2: mosfet.digitalWrite (1, 5, lightOnOff [1]); mosfet.digitalWrite (1, 6, LOW); mosfet.digitalWrite (1, 7, LOW); break;
    case 3: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, LOW); mosfet.digitalWrite (1, 7, LOW); break;
    case 4: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, lightOnOff [0]); mosfet.digitalWrite (1, 7, LOW); break;
    case 5: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, lightOnOff [1]); mosfet.digitalWrite (1, 7, LOW); break;
    case 6: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, HIGH); mosfet.digitalWrite (1, 7, LOW); break;
    case 7: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, HIGH); mosfet.digitalWrite (1, 7, lightOnOff [0]); break;
    case 8: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, HIGH); mosfet.digitalWrite (1, 7, lightOnOff [1]); break;
    case 9: mosfet.digitalWrite (1, 5, HIGH); mosfet.digitalWrite (1, 6, HIGH); mosfet.digitalWrite (1, 7, HIGH); break;
  }
}

//Функции кнопок

//Кнопка Select 1 клик
void clickSelect() {
  //Циклически меняет режимы с 0 по 3
  (mode < 3) ? mode++ : mode = 0;
  //Обнуляем флаг для первой функции
  modeFlag = 0;
  //Сбрасывает таймер для корректного отображения символов на дисплее при перелючении режима
  tm3 = millis();
}

//Кнопка минус 1 клик
void clickMinus() {
  switch (mode) {
    //В игровом режиме циклически понижает сложность
    case 0: (dif > 1) ? dif-- : dif = 9; break;
    //В отладочном режиме запускает автокаллибровку помех и сбрасываем флажок надписи, что бы после окончания калибровки опять были буквы
    case 1: modeFlag = 0; piezo_calibrateLow(); break;
    //В режиме помех, если значение помехи больше 0, уменьшает ее на 1. ВНИМАИЕ! На дисплее в этот момент отображеться значение ПОМЕХИ + ПРОЦЕНТНОГО ФИЛЬТРА! Отображаемый шаг может быть больше 1!
    case 2: if (stray > 0) {
        stray--;
        //Ставим таймер что бы пропустить заставку с буквами и сразу читать значение
        tm3 = millis() - 3001;
      }
      break;
    //В режиме верхнего предела силы удара уменьшаем этот предел. Тоже что и выше.
    case 3: if (exp_piez_max > 0) {
        exp_piez_max--;
        //Если верхний преедл подошел к помехе на помеху+процентный фильтр+ количество лампочек, не даем ему спуститься ниже во избежаение циклического срабатывания hit()
        if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
          //Да, тут все верно считается! Напоминаю что Lbrd отображает значение УЖЕ С ФИЛЬТРОМ! Соответвенно разница между Lbrd и Hbrd должна быть равна кол-ву лампочек + 1
          exp_piez_max = stray + (stray * interferenceFilter / 100) + numberOfLight + 1;
        }
        //Ставим таймер что бы пропустить заставку с буквами и сразу читать значение
        tm3 = millis() - 3001;
      }
      break;
  }

}

//Кнопка плюс 1 клик
void clickPlus() {
  //Тоже самое что и в минусе, только не выпадаем за верхний (1023) предел датчика.
  switch (mode) {
    case 0: (dif < 9) ? dif++ : dif = 1; break;
    case 1: modeFlag = 0; piezo_calibrateHigh(); break;
    case 2: if (stray <= 1023) {
        stray++;
        //Защита от перевала нижней помехи за максимальную силу удара. Если ее не ставить, то может сложится ситуация когда силомер просто войдет в цикл hit().
        //Если макс. ожидаемая сила удара оказалась меньше или равна помехи + фильтра помех + кол-во лампочек (от лампочек считаются деления шакалы. Если делений будет меньше чем лампочек, могут возникнуть ошибки)
        if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
          //То помеха равна макс. ожидаемая сила удара - ФИЛЬТР ПОМЕХ - кол-во лампочек.
          stray = exp_piez_max - (stray * interferenceFilter / 100) - numberOfLight;
        }
        tm3 = millis() - 3001;
      }
      break;
    case 3: if (exp_piez_max < 1023) {
        exp_piez_max++;
        tm3 = millis() - 3001;
      }
      break;
  }

}

//Кнопка минус зажатая
void longPressMinus() {
  //Тоже что и в обычном клике но раз в n времени и без первого мода
  switch (mode) {
    case 2: if (stray > 0) {
        stray--;
        //ПЛОХОЙ КОСТЫЛЬ!!! Позволяет не достигать крайних значений переменной за долю секунды, облегчает управление. Тут нужна паралельная работа, но на таймер почему то повесить не получаеться.
        delay(50);
      }
      break;
    case 3: if (exp_piez_max > 0) {
        exp_piez_max--;
        tm3 = millis() - 3001;
        if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
          exp_piez_max = stray + (stray * interferenceFilter / 100) + numberOfLight;
        }
        tm3 = millis() - 3001;
        //ПЛОХОЙ КОСТЫЛЬ!!! Позволяет не достигать крайних значений переменной за долю секунды, облегчает управление. Тут нужна паралельная работа, но на таймер почему то повесить не получаеться.
        delay(30);
      }
      break;
  }

}

//Кнопка плюс зажатая
void longPressPlus () {
  //Тоже что в минус зажатом
  switch (mode) {
    case 2: if (stray <= 1023) {
        stray++;
        if (exp_piez_max <= stray + (stray * interferenceFilter / 100) + numberOfLight) {
          //То помеха равна макс. ожидаемая сила удара - ФИЛЬТР ПОМЕХ - кол-во лампочек.
          stray = exp_piez_max - (stray * interferenceFilter / 100) - numberOfLight;
        }
        tm3 = millis() - 3001;
        delay(50);
      }
      break;
    case 3: if (exp_piez_max < 1023) {
        exp_piez_max++;
        tm3 = millis() - 3001;
        //ПЛОХОЙ КОСТЫЛЬ!!! Позволяет не достигать крайних значений переменной за долю секунды, облегчает управление. Тут нужна паралельная работа, но на таймер почему то повесить не получаеться.
        delay(30);
      }
      break;
  }
}

//Фукции модов

//Мод отладки. Смотрим показание с пьезика, на кнопках автоотладка.
void mode1() {
  //переменныу для работы в функции
  //Помехи
  static int interferenceShow;
  //Емли при переходе в цикл не показывалась надпись, то показать ее
  if (!modeFlag) {
    //Ставим флаг на 1, что бы надпись показалась только 1 раз
    modeFlag = 1;
    //Текст
    if (displayType == 0) {
      qd.displayDigits(QD_L, QD_b, QD_r, QD_d);
    }
    else if (displayType == 1) {
      digitalWrite(old_display_cs_pin, LOW);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00001010); //r
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00011110); //t
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00101010); //n
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01100000); //I
      digitalWrite(old_display_cs_pin, HIGH);
    }
    //Ждем, что бы человек увидел надпись
    delay (3000);
  }
  //Пока время меньше N ищем максимальную помеху и записываем ее
  if (millis() - tm3 < 500 && interferenceShow < analogRead (piez_pin)) {
    interferenceShow = analogRead (piez_pin);
  }
  //Раз в N времени показываем максимальный результат за пердидущий цикл
  else if (millis() - tm3 > 500) {
    displaySigns(interferenceShow);
    //и сбрасываем значение переменной на 0 (она static)
    interferenceShow = 0;
    tm3 = millis();
  }
}

//Функция помехи. Смотрим велечину нижней помехи, на кнопках изменение ее велечины
void mode2() {
  //Первые 3 секундв из 10 пишем слово заставку Lbrd (Low border)
  if (millis() - tm3 < 3000) {
    //Пишем библиотекой
    if (displayType == 0) {
      qd.displayDigits(QD_L, QD_b, QD_r, QD_d);
      digitalWrite (disp_pin, LOW);
    }
    //Пишем вручную
    else if (displayType == 1) {
      digitalWrite(old_display_cs_pin, LOW);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01111010); //d
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00001010); //r
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00111110); //b
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00011100); //L
      digitalWrite(old_display_cs_pin, HIGH);
    }
  }
  //С 3 по 10 секунду показываем ПОМЕХУ + ФИЛЬТР ПОМЕХ!!!!!!!!!!!!!!! ОЧЕНЬ ВНИМАТЕЛЬНО И ОСТОРОЖНО!!!!!!
  else if (millis() - tm3 > 3000 && millis() - tm3 < 10000) {
    displaySigns(stray + (stray * interferenceFilter / 100));
  }
  //После 10 секунд обнуляем таймер
  else if (millis() - tm3 > 10000) {
    tm3 = millis();
  }
}

//Функция верхнего предела силы удара
void mode3() {
  //Все тоже что и в функции помехи
  if (millis() - tm3 < 3000) {
    if (displayType == 0) {
      qd.displayDigits(QD_H, QD_b, QD_r, QD_d);
      digitalWrite (disp_pin, LOW);
    }
    else if (displayType == 1) {
      digitalWrite(old_display_cs_pin, LOW);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01111010);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00001010);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b00111110);
      shiftOut(old_display_mo_pin, old_display_mi_pin, LSBFIRST, 0b01101110);
      digitalWrite(old_display_cs_pin, HIGH);
    }
  }
  else if (millis() - tm3 > 3000 && millis() - tm3 < 10000) {
    //Показываем верхний предел силы удара.
    displaySigns(exp_piez_max);
  }
  else if (millis() - tm3 > 10000) {
    tm3 = millis();
  }
}
