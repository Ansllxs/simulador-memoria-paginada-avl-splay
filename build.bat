@echo off
REM ====================================================================
REM Script de compilacion para Windows (usa g++ de MinGW/MSYS2/Cygwin)
REM ====================================================================
echo Compilando simulador...
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src\main.cpp -o simulador.exe
if %ERRORLEVEL%==0 (
    echo.
    echo [OK] Compilacion exitosa: simulador.exe
    echo Pruebe:  simulador.exe input\test1.txt
) else (
    echo.
    echo [ERROR] La compilacion fallo.
)
