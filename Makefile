all:
	g++ -Wall -Wextra -std=c++11 -Iinclude demo/main.cpp src/*.cpp -pthread
