@echo off
REM Initialize Emscripten environment
CALL path\to\emsdk\emsdk_env.bat

REM Change to project directory
cd e:\Raylib_Cpp_1

REM Compile the game using custom shell file
emcc main.cpp -o pong.html ^
  -Os ^
  -s USE_GLFW=3 ^
  -s FULL_ES2=1 ^
  -s ASYNCIFY ^
  -s WASM=1 ^
  -s NO_EXIT_RUNTIME=1 ^
  -I"C:/raylib/raylib/src" ^
  "C:/raylib/raylib/src/libraylib.web.a" ^
  -s TOTAL_MEMORY=67108864 ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  --shell-file shell.html

REM If compilation was successful
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful! Game compiled to pong.html
  echo Use a web server to run the game: python -m http.server
) else (
  echo Compilation failed with error code %ERRORLEVEL%
)
