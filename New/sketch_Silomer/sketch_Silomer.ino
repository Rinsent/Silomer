//Силомер.
//Rinsent. Москва 2019.

/*Вкладки сверху
 * 1_ SIlomer_settings - Настройки
 * 2_ SIlomer_body     - Константы, setup, loop.
 * 3_ SIlomer_hit      - Функция hit()
 * 4_ SIlomer_funcion  - Остальные функции
 */

 /*
  * Перед загрузкой скетча в плату убедитесь что в "инструменты -> платы -> менеджер плат в Arduino AVR Boards (скорее всего самая врехняя) стоит версия 1.6.21. 
  * Иначе код может не компилироваться из за работы плеера в функции hit().
  */

  /*
   * Сторонние библиотеки
   * Включены в скетч, однако на всякий случай.
   * В Документы > Arduino > libraries > Название библиотеки
   * QuadDisplay2         - https://github.com/amperka/QuadDisplay2
   * AmperkaFET           - https://github.com/amperka/AmperkaFET
   * DFRobotDFPlayerMini  - https://github.com/DFRobot/DFRobotDFPlayerMini
   * OneButton            - https://github.com/mathertel/OneButton
   */

   /*
    * Конвертировать треки в:
    * MP3
    * Постоянный битрейт
    * 24000 Гц
    * моно
    * 
    * Перед перезаписью треков ОБЯЗАТЕЛЬНО отформатировать sd карту!
    * Иначе треки могут начать воспроизводится в неверном порядке.
    * 
    * Из дататшита плеера:
    *  Support Mp3 and WMV decoding
    *  Support sampling rate of
    * 8KHz,11.025KHz,12KHz,16KHz,22.05KHz,24KHz,32KHz,44.1KHz,48KHz
    *  24-bit DAC output, dynamic range support 90dB, SNR supports 85dB
    *  Supports FAT16, FAT32 file system, maximum support 32GB TF card 
    */
