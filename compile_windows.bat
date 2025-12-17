@echo off
echo ========================================
echo   RAPID TEXTER - Windows Compiler
echo ========================================
echo.

REM Create obj directory if not exists
if not exist obj mkdir obj

REM Compile all .cpp files
echo [1/4] Compiling Terminal.cpp...
g++ -std=c++17 -Wall -Iinclude -c src/Terminal.cpp -o obj/Terminal.o
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Terminal.cpp compilation failed!
    pause
    exit /b 1
)

echo [2/4] Compiling TextProvider.cpp...
g++ -std=c++17 -Wall -Iinclude -c src/TextProvider.cpp -o obj/TextProvider.o
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: TextProvider.cpp compilation failed!
    pause
    exit /b 1
)

echo [3/4] Compiling GameEngine.cpp...
g++ -std=c++17 -Wall -Iinclude -c src/GameEngine.cpp -o obj/GameEngine.o
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: GameEngine.cpp compilation failed!
    pause
    exit /b 1
)

echo [4/4] Compiling main.cpp...
g++ -std=c++17 -Wall -Iinclude -c src/main.cpp -o obj/main.o
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: main.cpp compilation failed!
    pause
    exit /b 1
)

REM Link all object files
echo.
echo Linking executable...
g++ -std=c++17 -o rapid-texter.exe obj/Terminal.o obj/TextProvider.o obj/GameEngine.o obj/main.o
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Linking failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   BUILD SUCCESS!
echo   Executable: rapid-texter.exe
echo ========================================
echo.
echo Run the program with: rapid-texter.exe
pause