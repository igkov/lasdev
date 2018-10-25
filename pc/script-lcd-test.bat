@echo off
SET /a i=0

:loop
IF %i%==369 GOTO END
ECHO Iteration %i%
lascond.exe -laser lcd -start %i%
@REM PAUSE
SET /a i=%i%+1
GOTO LOOP

:end
