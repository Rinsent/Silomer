/*
   Убедится что в "инструменты -> платы -> менеджер плат в Arduino AVR Boards (скорее всего самая врехняя) стоит версия 1.6.21. Иначе код может не компелироваться из за работы плеера в функции hit()
*/
//Настройки
//Общие
//#define port          0   //Отладка по СОМ порту (0-выкл, 1-вкл) ОБЯЗАТЕЛЬНО выключать перед загрузкой в силомер предназначенный для выездов
//Различные модули
#define displayType         1    //Тип установленного дисплея (0 - дисплей от амперки, 1 - дисплей от силомеров)
#define numberOfLight       12   //Колличество лампочек на силовой шкале силомера (1-13) ВНИМАНИЕ!!!!! Кол-во треков в плеере должно соответсвовать кол-ву лампочек в этой настройке!!! Если лампочек 5 то 5й трек будет последним который можно вызвать!
//Проверки и автокаллибровки
#define chek                1    //Проверка модулей на старте программы (0-выкл, 1-вкл)
#define calibrateLow        1    //Автокалибровка НИЖНЕГО предела силы удара НА СТАРТЕ программы (0-выкл, 1-вкл). Фильтрация помех при простое силомера.
#define calibrate_tm        5    //Время автаколибровки НИЖНЕГО предела в секундах (1 - 32)
#define interferenceFilter  100  //Значение в процентах (0-999, 0 - выкл) от макс. помехи выявленной при каллибровке. Прибавляется к уже установленному порогу помех.  Результат округляеться вниз до целого числа, так что если выставить значение 4 при помехе 20, доп. фильтр окажеться выключенным (20 * 0,04 = 0,8).
#define hitHight            450 //(0 - 1023) Верхний предел силы удара. По умолчанию 450.
#define calibrateHigh       1    //Автокалибровка ВЕРХНЕГО предела силы удара НА СТАРТЕ программы (0-выкл, 1-вкл).
#define calibrateHighInWork 0    //Автокалибровка ВЕРХНЕГО предела силы удара ПРИ РАБОТЕ программы (0-выкл, 1-вкл). Фильтрация зависимости счета от датчика, клея, наводок, наковальни, уклона асфальта, настроения менеджера, Нептуна в доме Юпитера и прочих непредсказуемых факторов. ОДНАКО, в случае особо сильных ударов, это может приводить к повышению уровня сложности игры без изменения настроек.
#define hit_tm              500  //Время в МИЛИСЕКУНДАХ (1с = 1000мс) для нахождения пиковой силы удара. Слишком маленькое значение не даст определить максимальную силу удара, слишком большое может давать помехи. Подбирать экспериментально, но точно не больше секунды.
//Звук
#define vol                 30   //Настройки громкости (0-30). Рекомендуется сначало увеличивать громкость тут в настройках, а только потом на самих колонках. Будет меньше треска и шума.
#define eql                 0    //Настройки эквалайзера (0—Normal, 1—Pop, 2—Rock, 3—Jazz, 4—Classic, 5—Bass)
//Работа экрана и лампочек
#define result_tm           5    //Время в секундах (1 - 32), которое уходит на то, что бы показать значение силы удара от 0 до выбитого.
#define step_tm             50   //Время в МИЛИСЕКУНДАХ (1с = 1000мс). Определяет время за которое экран при показе очков изменяет свое значение на 1 шаг. Значение не должно быть >= result_tm*1000!!! При слишком маленьком значении цифры будут мыльными, при слишком большом будет казаться что автомат лагает. По умолчанию стоит 50.
#define blinkEf             2    //(0-2) Мигает лампочками и цифрами во время озвучки результат удара (0 - не мигает, 1 - мигает только при счете 9999, 2 - мигает при любом счете)
#define blinkEfTm           500  //Время в МИЛЛИСЕКУНДАХ (1с = 1000мс). Частота мигания эффектов.
#define counterEfMax        3    // N раз мигания лампочек и дисплея (если 0 и эффекты включены, мигает пока идет трек)
//Остальное
#define rndScore            1    //Изменение счета на +1% от реального. (0-выкл, 1-вкл) Нужно для того, что бы создать впечатление разнообразия и избежать кратных чисел на некоторых ур. сложности.
#define Dlevel              5    //(1-9) Уровень сложности при запуски силомера
