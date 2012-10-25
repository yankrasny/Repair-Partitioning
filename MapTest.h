#include <unordered_map>
#include <iostream>

int main()
{
	std::unordered_map<int, int> myMap = std::unordered_map<int, int>();
	myMap[3] = 5;
	std::cout << myMap[3];
}