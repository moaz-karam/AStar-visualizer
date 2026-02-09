libraylib := ../include/raylib/src/libraylib.a
raylib_h := ../include/raylib/src/raylib.h
raylib_shell := ../include/raylib/src/shell.html

default:
	emcc -o index.html ../src/main.cpp -Os -Wall $(libraylib) -I. -I$(raylib_h) -L. -L$(libraylib) -s USE_GLFW=3 -s ASYNCIFY --shell-file $(raylib_shell) -DPLATFORM_WEB -s ALLOW_MEMORY_GROWTH