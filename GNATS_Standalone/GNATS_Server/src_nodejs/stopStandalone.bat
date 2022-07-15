@echo off
taskkill /pid %1 /f > nul

set TARGET_PS=

for /f "tokens=5" %%a in ('netstat -aon ^| findstr :2019') do (
if [%TARGET_PS%] == [] set TARGET_PS=%%a
if [%TARGET_PS%] == [0] set TARGET_PS=
)

rem ********************

set TARGET_PS=

for /f "tokens=5" %%a in ('netstat -aon ^| findstr :3000') do (
if [%TARGET_PS%] == [] set TARGET_PS=%%a
if [%TARGET_PS%] == [0] set TARGET_PS=
)
@echo on