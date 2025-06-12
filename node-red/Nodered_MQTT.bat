@echo off
cd /d C:\mosquitto
start mosquitto -v -c /mosquitto/mosquitto.conf
timeout /t 5 /nobreak >nul
start node-red
