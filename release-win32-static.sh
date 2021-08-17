mkdir -p bin/release
mkdir -p bin/o/win32
PATH=$PATH:~/c/mxe/usr/bin
i686-w64-mingw32.static-windres src/icon.rc -o bin/o/win32/icon.o
i686-w64-mingw32.static-gcc -o bin/release/FlappyNavi-win32.exe src/*.c bin/o/win32/icon.o `~/c/mxe/usr/bin/i686-w64-mingw32.static-pkg-config --libs --cflags sdl2` -lm -Wall -Wextra -DFLAPPY_STATIC_BUILD -DNDEBUG -Os -s
