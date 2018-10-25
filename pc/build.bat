@REM Собираем утилиту bin2c:
@REM gcc -s -Os -I../libs -o bin2c.exe bin2c.c ../libs/file.c

@REM Генерируем образ загрузчика:
bin2c.exe ../mcu-loader/loader.bin loader > loader.c

@REM Компилируем ресурсы:
windres.exe resource.rc resource.o

@REM Сборка публичной версии утилиты:
@REM gcc -m32 -Wall -Wno-missing-braces -Os -s -DDEVELOPER_BUILD=0 -DGRAPH_SUPPORT=1 -I../libs -o lascon.exe main.c ../libs/hid.c ../libs/file.c ../libs/crc.c ../libs/solve.c ../libs/vector.c ../libs/gost.c ../libs/symbols.c graph.c ../libs/draw.c ../libs/printf.c ../libs/lowpass.c -lhid -lsetupapi -lGdi32 resource.o 

@REM Сборка полной версии утилиты:
gcc -Wall -Wno-missing-braces -Os -s -DDEVELOPER_BUILD=1 -DGRAPH_SUPPORT=1 -I../libs -o lascond.exe main.c ../libs/hid.c ../libs/file.c ../libs/crc.c ../libs/solve.c ../libs/vector.c ../libs/orientation.c ../libs/gost.c ../libs/symbols.c ../libs/draw.c ../libs/printf.c ../libs/lowpass.c ../libs/calibrate.c ../libs/lcd.c loader.c -lhid -lsetupapi -lGdi32 -lComdlg32 resource.o 
@REM -m32 
