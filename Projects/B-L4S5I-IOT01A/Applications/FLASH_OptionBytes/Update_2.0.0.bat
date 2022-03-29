@echo off

set BOARD=DIS_L4S5VI
set file_name=".\EWARM\X-CUBE-AWS_2.0.0\Exe\X_CUBE_AWS_2.0.0_FLASH_OptionBytes.bin"
set myd=""

for /f %%d in ('wmic volume get driveletter^, label ^| findstr "%BOARD%"') do set myd=%%d

if %myd%=="" goto :BOARD_NOT_FOUND

if exist %file_name% (
    echo Updating %BOARD%
    xcopy %file_name% %myd% /Y
) else (
    echo file doesn't exist
)

goto :END

:BOARD_NOT_FOUND
echo Please make sure your %BOARD% board is connected

:END
