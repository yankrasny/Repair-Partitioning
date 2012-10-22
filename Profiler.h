/*

Can't nest currently

*/

#ifndef PROFILER_H
#define PROFILER_H

#include<fstream>
#include<vector>
#include<string>
#include<ctime>
#include"MapUtils.h"

class Profiler
{
private:
	//function names to a list of times for each
	std::map<std::string, std::vector<unsigned> > times;
	std::map<std::string, std::map<std::string, double> > stats;
	
	unsigned currStart, currEnd, diff;
	std::string currFunc;

	std::string inputSpec;

	static Profiler GlobalProfiler;
	Profiler()
	{
		times = std::map<std::string, std::vector<unsigned> >();
		stats = std::map<std::string, std::map<std::string, double> >();
	}

	void addTime(const std::string& func, unsigned t)
	{
		if (!mapExists(times, func))
		{
			times[func] = std::vector<unsigned>();
		}
		times[func].push_back(t);
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
		currFunc = func;
		currStart = clock();
	}

	/* Ends whichever one was started, so no funcID necessary */
	void end()
	{
		currEnd = clock();
		diff = currEnd - currStart;
		this->addTime(currFunc, diff);
	}

	void reset(const std::string& func)
	{
		times[func].clear(); //Do we need to clear the inner vectors? TODO
	}

	double checkKeyAndReturn(const std::string& func, const std::string& field);

	void calcStats();

	double getAvg(const std::string& func);

	double getNumCalls(const std::string& func);

	double getTotalTime(const std::string& func);

	void writeResults(const std::string& filename);
};

#endif