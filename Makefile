program: main.cpp 
	g++  -g -std=c++11  -o program main.cpp -lncurses -pthread

clean: 
	rm program
