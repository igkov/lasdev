@REM TEST VISIO
gcc -m32 -Wall -Wno-missing-braces -D__TEST__ -I../libs -Os -s -o test.exe visualization.c ../libs/pid.c ../libs/lowpass.c -lhid -lsetupapi -lGdi32 -lComdlg32 
break
@REM USB VERSION
gcc -m32 -Wall -Wno-missing-braces -DUSB -I../libs -Os -s -o simple.exe simple.c visualization.c ../libs/hid.c ../libs/calibrate.c ../libs/vector.c ../libs/solve.c ../libs/solve9.c ../libs/lowpass.c -lhid -lsetupapi -lGdi32 -lComdlg32 
@REM BT VERSION
gcc -m32 -Wno-missing-braces -DBT -I../libs -Os -s -o simplebt.exe simple.c visualization.c ../libs/btproto.c ../libs/com.c ../libs/crc.c ../libs/calibrate.c ../libs/vector.c ../libs/solve.c ../libs/solve9.c ../libs/lowpass.c -lhid -lsetupapi -lGdi32 -lComdlg32 
@REM -Wall 

