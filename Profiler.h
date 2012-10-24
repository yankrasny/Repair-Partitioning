/*

Can't nest currently

*/

#ifndef PROFILER_H
#define PROFILER_H

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include "MapUtils.h"

class FunctionCall
{
private:
	unsigned start;
	unsigned end;
	std::string funcName;
public:
	FunctionCall(const std::string& funcName, unsigned start, unsigned end) : funcName(funcName), start(start), end(end) {}
	FunctionCall(const std::string& funcName, unsigned start) : funcName(funcName), start(start), end(0) {}

	void setEnd(unsigned end)
	{
		this->end = end;
	}

	unsigned getElapsed()
	{
		if (end > start)
			return end - start;
		return 0;
	}
};

class Profiler
{
private:
	//function name -> list of function call objects
	std::map<std::string, std::vector<FunctionCall> > calls;

	//function name -> list of statistics (an alphanumeric key, and a double value)
	std::map<std::string, std::map<std::string, double> > stats;
	
	unsigned currStart;
	unsigned currEnd;

	std::string inputSpec;

	static Profiler GlobalProfiler;
	Profiler()
	{
		calls = std::map<std::string, std::vector<FunctionCall> >();
		stats = std::map<std::string, std::map<std::string, double> >();
	}

	void addCall(const std::string& func, unsigned start, unsigned end = 0)
	{
		if (!mapExists(calls, func))
		{
			calls[func] = std::vector<FunctionCall>();
		}
		// std::cerr << "Func: " << func << ", numCalls: " << calls[func].size() << std::endl;
		// system("pause");
		calls[func].push_back(FunctionCall(func, start, end));
	}

public:
	static inline Profiler& getInstance()
	{
		return GlobalProfiler;
	}

	void setInputSpec(const std::string& inputSpec)
	{
		this->inputSpec = inputSpec;
	}

	void start(const std::string& func)
	{
		currStart = clock();
		this->addCall(func, currStart);
	}

	/* Ends whichever one was started, so no funcID necessary */
	void end(const std::string& func)
	{
		currEnd = clock();
		calls[func].back().setEnd(currEnd);
	}

	void reset(const std::string& func)
	{
		calls[func].clear(); //Do we need to clear the inner vectors? TODO, but don't think so, since there aren't any pointers to worry about.
	}

	double checkKeyAndReturn(const std::string& func, const std::string& field);

	void calcStats();

	double getAvg(const std::string& func);

	double getNumCalls(const std::string& func);

	double getTotalTime(const std::string& func);

	void writeResults(const std::string& filename);
};

#endif