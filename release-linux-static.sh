mkdir -p bin/release
gcc -o bin/release/FlappyNavi-linux64 src/*.c -lSDL2 -lm -Wall -Wextra -DFLAPPY_STATIC_BUILD -DNDEBUG -Os -s -flto -no-pie
