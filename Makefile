raylib := ./include/libraylib.a
raygui := ./include/raygui.so

default:
	g++ ./src/main.cpp -o  main -Iinclude $(raylib) $(raygui)