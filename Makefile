SOURCES = $(wildcard *.cpp) $(wildcard util/*.cpp) $(wildcard repair-algorithm/*.cpp) $(wildcard random-heap/*.cpp) $(wildcard partitioning/*.cpp) $(wildcard prototype/*.cpp)

all:
	g++ $(SOURCES) -o repair -g -std=c++0x