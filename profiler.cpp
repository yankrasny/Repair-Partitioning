#include"profiler.h"

Profiler Profiler::GlobalProfiler;

void Profiler::calcStats()
{
	std::map<std::string, std::vector<unsigned> >::iterator it;
	std::map<std::string, double> currentMap;
	for (it = times.begin(); it != times.end(); it++)
	{
		double sum(0.0);
		double numCalls = it->second.size();
		currentMap["numCalls"] = numCalls;
		for (size_t i = 0; i < numCalls; i++)
		{
			sum += it->second[i];
		}
		currentMap["totalTime"] = sum;
		currentMap["average"] = sum / numCalls;
		stats[it->first] = currentMap;
	}
}

double Profiler::checkKeyAndReturn(const std::string& func, const std::string& field)
{
	if (mapExists(stats, func))
		if (mapExists(stats[func], field))
			return stats[func][field];
	return 0.0;
}

double Profiler::getNumCalls(const std::string& func)
{
	return checkKeyAndReturn(func, "numCalls");
}

double Profiler::getAvg(const std::string& func)
{
	return checkKeyAndReturn(func, "average");
}

double Profiler::getTotalTime(const std::string& func)
{
	return checkKeyAndReturn(func, "totalTime");
}

void Profiler::writeResults(const std::string& filename)
{
	this->calcStats();
	std::ofstream os(filename.c_str(), std::ios::out | std::ios::app );
	std::map<std::string, std::vector<unsigned> >::iterator it;
	double avg;
	double numCalls;
	double totalTime;
	os << std::endl << "---------------------------------------" << std::endl << std::endl;
	os << this->inputSpec << std::endl;
	for (it = times.begin(); it != times.end(); it++)
	{
		avg = this->getAvg(it->first);
		numCalls = this->getNumCalls(it->first);
		totalTime = this->getTotalTime(it->first);
		os << "[" << it->first << "] average time: " << avg << "ms, numCalls: " << numCalls << ", total time: " << totalTime << "ms" << std::endl;
	}
	//os << std::endl;
	os.close();
}