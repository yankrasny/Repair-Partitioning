SOURCES = $(wildcard *.cpp) $(wildcard util/*.cpp) $(wildcard repair-algorithm/*.cpp) $(wildcard indexed-heap/*.cpp) $(wildcard partitioning/*.cpp) $(wildcard prototype/*.cpp)

all:
	g++ $(SOURCES) -o repair -g -std=c++0x

optimized:
	g++ $(SOURCES) -o repair -O3 -std=c++0x

profile:
	g++ $(SOURCES) -o repair -pg -std=c++0x

clean:
	rm -rvf repair repair.dSYM

clean_win:
	rm -rvf repair.exe
