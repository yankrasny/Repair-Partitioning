#include "repair-algorithm/Repair.h"
#include "repair-algorithm/UndoRepair.h"
#include "partitioning/Partitioning.h"
#include "util/Util.h"
using namespace std;

/*

Add a comment

We must be able to re-extract for the algorithm to be correct
Associations must be monotonically decreasing for the algorithm to be optimal
*/
bool checkOutput(vector<Association> associations, vector<unsigned> wordIDs)
{
	//Check to see whether it's correct
	vector<unsigned> extractedWordIDs = undoRepair(associations);
	for (size_t i = 0; i < extractedWordIDs.size(); i++)
	{
		//cerr << "Orig: " << wordIDs[i] << ", compare: " << extractedWordIDs[i] << endl; 
		if (extractedWordIDs[i] != wordIDs[i])
		{
			return false;
		}
	}

	cerr << "Passed correctness check..." << endl;

	//If it's correct, check to see whether it's optimal
	for (size_t i = 0; i < associations.size() - 1; i++)
	{
		// cerr << "i: " << i << " freq[i]: " << associations[i].freq << ", freq[i+1]: " << associations[i+1].freq << endl;
		// system("pause");
		if (associations[i].freq < associations[i+1].freq)
		{
			return false;
		}
	}

	cerr << "Passed optimal check..." << endl;

	return true;
}

// TODO normalize: I think we can get a rigorous definition of score
// If all versions are exactly the same, then the score should be 100
// What does it mean for all versions to be the same? Well each fragment found will occur in all versions. 
// So a term with count in it should really be (count / numVersions)
// And a term with fragSize should really be (fragSize / docSize)
// Try to express it all as a percentage (so denominator will have something with the doc size and num versions as well)

// Maybe just SUM ( ((count[i] / numVersions) * (fragSize / docSize)) / (numVersions * docSize) )
// Write that out and think about it

// If no redundancy was found, the score should be zero (although with repair, that isn't likely at all)
double getScore(unordered_map<string, FragInfo>& uniqueFrags, unsigned numVersions, ostream& os = cerr)
{
	double term;
	double sum(0);
	for (unordered_map<string, FragInfo>::iterator it = uniqueFrags.begin(); it != uniqueFrags.end(); it++)
	{
		term = it->second.count * it->second.fragSize;
		sum += term;
	}
	return sum / (uniqueFrags.size() * numVersions);
}


void writeAssociations(const vector<Association>& associations, ostream& os = cerr)
{
	for (size_t i = 0; i < associations.size(); i++)
	{
		os << associations[i];
	}
}

void writeResults(const vector<vector<unsigned> >& versions, unsigned* offsetsAllVersions, unsigned* versionPartitionSizes, const vector<Association>& associations, unordered_map<unsigned, string>& IDsToWords, const string& outFilename, bool printFragments = false, bool printAssociations = false)
{
	ofstream os(outFilename.c_str());

	os << "Results of re-pair partitioning..." << endl << endl;
	os << "*** Fragment boundaries ***" << endl;
	
	unsigned totalCountFragments(0);
	unsigned diff(0);
	unsigned numVersions = versions.size();
	for (unsigned v = 0; v < numVersions; v++)
	{
		unsigned numFragsInVersion = versionPartitionSizes[v];

		if (numFragsInVersion < 1)
		{
			continue;
		}
		os << "Version " << v << endl;
		for (unsigned i = 0; i < numFragsInVersion - 1; i++)
		{
			if (i < versionPartitionSizes[v] - 1)
			{
				unsigned currOffset = offsetsAllVersions[totalCountFragments + i];
				unsigned nextOffset = offsetsAllVersions[totalCountFragments + i + 1];
				diff = nextOffset - currOffset;
			}
			else
			{
				diff = 0;
			}

			os << "Fragment " << i << ": " << offsetsAllVersions[totalCountFragments + i] << "-" << 
				offsetsAllVersions[totalCountFragments + i + 1] << " (frag size: " << diff << ")" << endl;
		}
		totalCountFragments += numFragsInVersion;
		os << endl;
	}

	// os << "Number of fragment boundaries: " << starts.size() << endl;
	// os << "Number of fragments: " << (starts.size() - 1) << endl << endl;

	vector<vector<FragInfo > > fragmentHashes;
	os << "*** Fragments ***" << endl;
	fragmentHashes = getFragmentHashes(versions, offsetsAllVersions, versionPartitionSizes, IDsToWords, os, printFragments);

	// Assign fragment IDs and stick them in a hashmap
	unordered_map<string, FragInfo> uniqueFrags;
	updateFragmentHashMap(fragmentHashes, uniqueFrags);

	// Now decide on the score for this partitioning
	double score = getScore(uniqueFrags, versions.size(), os);
	os << "Score: " << score << endl;
	
	if (printAssociations)
	{
		os << "*** Associations (symbol -> pair) ***" << endl;
		writeAssociations(associations, os);
	}
}

void printIDtoWordMapping(unordered_map<unsigned, string>& IDsToWords, ostream& os = cerr)
{
	for (unordered_map<unsigned, string>::iterator it = IDsToWords.begin(); it != IDsToWords.end(); it++)
	{
		os << it->first << ": " << it->second << endl;
	}
}


double runRepairPartitioning(vector<vector<unsigned> > versions, unordered_map<unsigned, string>& IDsToWords, 
	unsigned*& offsetsAllVersions, unsigned*& versionPartitionSizes, vector<Association>& associations,
	unsigned minFragSize, unsigned repairStoppingPoint, bool printFragments = false)
{
	//Allocate the heap, hash table, array of associations, and list of pointers to neighbor structures
	
	RandomHeap myHeap;
	
	unordered_map<unsigned long long, HashTableEntry*> hashTable = unordered_map<unsigned long long, HashTableEntry*> ();
	
	associations = vector<Association>();
	
	vector<VersionDataItem> versionData = vector<VersionDataItem>();

	extractPairs(versions, myHeap, hashTable, versionData);

	int numPairs = hashTable.size();

	doRepair(myHeap, hashTable, associations, repairStoppingPoint, versionData);
	
	versionPartitionSizes = new unsigned[versions.size()];

	offsetsAllVersions = getPartitioningsAllVersions(myHeap, minFragSize, versionData, versionPartitionSizes);

	if (!offsetsAllVersions)
	{
		// Maybe repairStoppingPoint was too low
		return 0.0;
	}

	vector<vector<FragInfo > > fragmentHashes;
	fragmentHashes = getFragmentHashes(versions, offsetsAllVersions, versionPartitionSizes, IDsToWords, cerr, printFragments);

	// Assign fragment IDs and stick them in a hashmap
	unordered_map<string, FragInfo> uniqueFrags;
	updateFragmentHashMap(fragmentHashes, uniqueFrags);

	// Now decide on the score for this partitioning
	double score = getScore(uniqueFrags, versions.size(), cerr);
	if (printFragments)
		cerr << "Score: " << score << endl;

	return score;
}

unsigned currentFragID = 0;
unsigned currentID = 0;

int main(int argc, char* argv[])
{
	//createOutputDir();

	//heap, repair
	string test = "repair";

	if (test == "heap")
	{
		RandomHeapTest test = RandomHeapTest(1000);
		exit(0);
	}

	if (test == "repair")
	{
		Profiler::getInstance().start("all");
		string inputFilepath = "./Input/alice/";

		// Default param values
		/*
		Initial test show that minFragSize should be proportional to document size
		*/
		unsigned minFragSize = 2; //in words

		/*
		Initial tests show that repairStoppingPoint shouldn't be too small (repair goes too far for our purposes in this case)
		And it shouldn't be larger than the number of versions (this is trivial, we expect to get repetition 
		at most numVersions times for inter-version repetitions)
		*/
		unsigned repairStoppingPoint = 2; //pairs that occur less than this amount of times will not be replaced

		if (argc == 2 && (string) argv[1] == "help")
		{
			cerr << "Usage:" << endl;
			cerr << "\t" << argv[0] << " <directory> <minFragSize> <repairStoppingPoint>" << endl;
			cerr << "\t" << argv[0] << " <directory> <minFragSize>" << endl;
			cerr << "\t" << argv[0] << " <directory>" << endl;
			cerr << "\t" << argv[0] << "" << endl << endl;
			cerr << "Defaults: " << endl;
			cerr << "\tdirectory: " << inputFilepath << endl;
			cerr << "\tminFragSize: " << minFragSize << endl;
			cerr << "\trepairStoppingPoint: " << repairStoppingPoint << endl;
			exit(0);
		}

		if (argc > 1)
			inputFilepath = (string)argv[1];

		if (argc > 2)
			minFragSize = atoi(argv[2]);

		if (argc > 3)
			repairStoppingPoint = atoi(argv[3]);
		
		vector<string> inputFilenames = vector<string>();
		if (getFileNames(inputFilepath, inputFilenames))
			return errno;

		char* text;
		
		int fileSize;
		
		vector<vector<unsigned> > versions = vector<vector<unsigned> >(); //each inner vector is the wordIDs for one version
		
		vector<unsigned> wordIDs;
		
		unordered_map<unsigned, string> IDsToWords = unordered_map<unsigned, string>();

		unordered_map<unsigned, unsigned> uniqueWordIDs = unordered_map<unsigned, unsigned>();
		
		for (unsigned i = 0; i < inputFilenames.size(); i++)
		{
			stringstream filenameSS;
			filenameSS << inputFilepath << inputFilenames[i];
			string filename = filenameSS.str();
			text = getText(filename, fileSize);
			if (!text)
				continue;
			wordIDs = stringToWordIDs(text, IDsToWords, uniqueWordIDs);
			versions.push_back(wordIDs);
			delete text;
			text = NULL;
		}
		
		// By this time, IDsToWords should contain the mappings of IDs to words in all versions
		// printIDtoWordMapping(IDsToWords);
		// system("pause");

		vector<Association> associations;

		unsigned* offsetsAllVersions(NULL);
		unsigned* versionPartitionSizes(NULL);

		unsigned* bestOffsetsAllVersions(NULL);
		unsigned* bestVersionPartitionSizes(NULL);

		unsigned bestMinFragSize;
		unsigned bestRepairStoppingPoint;

		double score(0.0);
		double max(0.0);
		// This crashes after 4 lines of output
		for ( minFragSize = 10; minFragSize < 30; minFragSize += 10 )
		{
			for ( repairStoppingPoint = 2; repairStoppingPoint < 20; repairStoppingPoint += 2 )
			{
				score = runRepairPartitioning(versions, IDsToWords, offsetsAllVersions, versionPartitionSizes, associations, minFragSize, repairStoppingPoint, false);
				cerr << "minFragSize: " << minFragSize << ", repairStoppingPoint: " << repairStoppingPoint << ", score: " << score << endl;

				for (unsigned v = 0; v < versions.size(); v++)
				{
					if (versionPartitionSizes[v] < 1)
					{
						score = 0.0;
						break;
					}
				}

				if (score > max)
				{
					max = score;
					bestMinFragSize = minFragSize;
					bestRepairStoppingPoint = repairStoppingPoint;

					bestOffsetsAllVersions = offsetsAllVersions;
					bestVersionPartitionSizes = versionPartitionSizes;
				}
				else
				{
					delete offsetsAllVersions;
					offsetsAllVersions = NULL;

					delete versionPartitionSizes;
					versionPartitionSizes = NULL;
				}
			}
		}

		cerr << "Best partitioning happened with" << endl;
		cerr << "minFragSize: " << bestMinFragSize << ", repairStoppingPoint: " << bestRepairStoppingPoint << endl;

		// stringstream outFilenameStream;
		// outFilenameStream << "Output/results" << stripDot(inputFilepath);
		// string outputFilename = outFilenameStream.str();

		string outputFilename = "Output/results.txt";

		bool printFragments = true;
		bool printAssociations = false;
		writeResults(versions, bestOffsetsAllVersions, bestVersionPartitionSizes, associations, IDsToWords, outputFilename, printFragments, printAssociations);

		stringstream command;
		command << "start " << outputFilename.c_str();
		system(command.str().c_str());

		// stringstream ss;
		// ss << "Filename: " << inputFilepath << endl;
		// ss << "Size: " << fileSize << " bytes" << endl; 
		// ss << "Word Count: " << wordIDs.size() << endl;
		// ss << "Unique Pair Count (in original file): " << numPairs << endl;

		// cerr << "Checking output... " << endl;
		// if (checkOutput(associations, wordIDs))
		// 	cerr << "Output ok!";
		// else
		// 	cerr << "Check failed!";
		// cerr << endl;

		// Profiler::getInstance().end("all");
		// Profiler::getInstance().setInputSpec(ss.str());
		// Profiler::getInstance().writeResults("Output/profile-functions.txt");

		// cleanup(hashTable);
		// system("pause");
	}
}