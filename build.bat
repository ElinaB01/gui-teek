@echo off
CLS

set COMPILER=tcc

:: Compile the library (guilib.dll)
%COMPILER% -shared -o guilib.dll guilib.c scrollbar.c label.c button.c slider.c input.c checkbox.c radiobutton.c progressbar.c listbox.c -L. -Iinclude -lSDL2 -lSDL2_ttf -lSDL2_gfx -DBUILD_GUILIB

:: End script on error
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

:: Compile a test program (demo.exe)
%COMPILER% -o demo.exe main.c elements.c -L. -lkernel32 -lguilib -Iinclude -lSDL2 -lSDL2_ttf

:: End script on error
IF %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%

demo.exe