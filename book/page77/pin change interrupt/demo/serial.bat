@echo off
dwdebug ls,q > serial.txt
for /f "delims=" %%a in ('type "serial.txt" ^| find "ATmega328P on COM"') do (
        set "str=%%a"
)
set "str=%str:*ATmega328P on COM=%"
for /f "tokens=1" %%a in ("%str%") do (
        set str=%%a
)
echo %str%
