# `lasdev` repo

IMU and Laser distance meter for LPC13xx MCU (DistoX analog).

## `distox`

Программа-эмулятор для PocketTopo (заносит измерения в программу PocketTopo).

## `docs`

Папка с документацией.

## `hw`

Папка с проектом платы на LPC134x.

## `hw-pics`

Папка с картинками (аппаратная плата).

## `libs`

Файлы библиотек.

* `btproto.c` - фукнции протокола работы по последовательному интерфейсу.
* `calibrate.c` - Функции калибровки.
* `gost.c` - функция шифрования по ГОСТ 28147-89 (функция имеет ошибки в порядке бит, поэтому несовместима с данным стандартом).
* `lcd.c` - функция взаимодействия с LCD-экрном в дальномере Unit-T xxx.
* `orientation.c` - функция вычисления текущих углов.
* `solve.c` и `solve9.c` - функции решения системы переопределенных уравнений.
* `vector.c` - математика рассчетов на основе кватернионов.

## `mcu-cdc`

Прошивка моста USB-CDC. Для различных тестов.

## `mcu-frw`

Прошивка модуля встраивания. Имеет 3 конфигурации сборки:

* `LasModFRW (dbg)` - отладочная версия программы. Прямая заливка (самодостаточная микропрограмма).
* `LasModFRW (rel)` - рабочая сборка программы. Заливка через загрузчик (`mcu-loader`) и утилиту `lasdev.exe`. 
* `LasModFRW (bluetooth)` - сборка программы для работы через Bluetooth-модуль. Используется для получения отладочных данных и беспроводной отладки их алгоритмов обработки.

## `mcu-frw-test`

Тестовая рыба-пример (для работы через загрузчик).

## `mcu-loader`

Проект загрузчика (через него работает утилита обновления прошивки).

## `pc`

Проект для компьютера, утилита `lasdev.exe`. Загружает прошивки, формирует образа для загрузки, тестирует функционал, позволяет конфигурировать устройство.

## `pc-simple-bt`

Простой пример работы через Bluetooth.

## `pc-simple-hid`

Простой пример работы через USB HID-устройство.

## `pc-viz`

Пример чтения данных с Bluetooth-устройства с выводом графиков.

