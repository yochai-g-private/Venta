@echo off

set ProjectDir=%~dp0
set Lib=%ProjectDir%lib\

set Libraries=C:\Users\yochai.glauber\Documents\Arduino\Libraries\

::============================================
:: Create links
::============================================

call :Link_LIB      SevenSegmentFun     src
call :Link_LIB      TimeLib             .
call :Link_LIB      DS3231              .
call :Link_LIB      LiquidCrystal_I2C   .
call :Link_LIB      IRremote            .

call :Link_LIB      NYG

goto :eof

::---------------------------
:Link_LIB
::---------------------------
if exist %Lib%%1        rmdir  /s/q %Lib%%1
mklink /D %Lib%%1       %Libraries%%1

goto :eof
