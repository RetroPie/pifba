@echo off
set path=c:\gp2x\devkitGP2X\arm-linux\bin;c:\gp2x\devkitGP2X\bin\;c:\gp2x\devkitGP2X\libexec\gcc\arm-linux\4.0.2\
echo **** Start of make ****
make 2>errors
if errorlevel 2 goto a
if errorlevel 1 pause
goto b
:a
start c:\WINDOWS\notepad errors
goto c
:b
echo **** No errors! ****
copy ezxfba\fba2x_s \\192.168.1.161\gp2x\mnt\sd\fba2x
pause
:c
