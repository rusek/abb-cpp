all:
	g++ -Wall -Wextra -Wfatal-errors -std=c++11 -Iinclude demo/main.cpp src/*.cpp -pthread
