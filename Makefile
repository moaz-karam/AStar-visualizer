libraylib_web := ./include/raylib/src/libraylib_web.a
RAYLIB := ./include/raylib/src/
raylib_h := ./include/raylib/src/raylib.h
raylib_shell := ./include/raylib/src/shell.html

gnu:
	g++ -o main ./src/main.cpp -I $(RAYLIB) -L $(RAYLIB) -lraylib_gnu -lGL -lm -lpthread -ldl -lrt -lX11
web:
	emcc -o index.html ./src/main.cpp -Os -Wall $(libraylib_web) -I. -I$(raylib_h) -L. -L$(libraylib_web) -s USE_GLFW=3 -s ASYNCIFY --shell-file $(raylib_shell) -DPLATFORM_WEB -s ALLOW_MEMORY_GROWTH

all: gnu web
