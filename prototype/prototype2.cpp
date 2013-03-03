#include "prototype2.h"
using namespace std;

double Prototype2::getScore(unordered_map<string, FragInfo>& uniqueFrags, unsigned numVersions, ostream& os)
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

void Prototype2::writeAssociations(const vector<Association>& associations, ostream& os)
{
	for (size_t i = 0; i < associations.size(); i++)
	{
		os << associations[i];
	}
}

void Prototype2::writeResults(const vector<vector<unsigned> >& versions, unsigned* offsetsAllVersions, unsigned* versionPartitionSizes, 
	const vector<Association>& associations, unordered_map<unsigned, string>& IDsToWords, const string& outFilename, 
	bool printFragments, bool printAssociations)
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

void Prototype2::printIDtoWordMapping(unordered_map<unsigned, string>& IDsToWords, ostream& os)
{
	for (unordered_map<unsigned, string>::iterator it = IDsToWords.begin(); it != IDsToWords.end(); it++)
	{
		os << it->first << ": " << it->second << endl;
	}
}

double Prototype2::runRepairPartitioning(vector<vector<unsigned> > versions, unordered_map<unsigned, string>& IDsToWords, 
	unsigned*& offsetsAllVersions, unsigned*& versionPartitionSizes, vector<Association>& associations,
	unsigned minFragSize, unsigned repairStoppingPoint, bool printFragments)
{
	//Allocate the heap, hash table, array of associations, and list of pointers to neighbor structures
	
	RandomHeap myHeap;
	
	unordered_map<unsigned long long, HashTableEntry*> hashTable = unordered_map<unsigned long long, HashTableEntry*> ();
	
	associations = vector<Association>();
	
	vector<VersionDataItem> versionData = vector<VersionDataItem>();

	RepairTree repairTree;

	extractPairs(versions, myHeap, hashTable, versionData, repairTree);

	// int numPairs = hashTable.size();

	// doRepair(myHeap, hashTable, associations, repairStoppingPoint, versionData, repairTree);

	// cerr << repairTree.getHead();
	// system("pause");

	// repairTree is now set
	/*
		Magic goes here
	*/

	double score = 0.0;
	return score;
}


int Prototype2::run(int argc, char* argv[])
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

		double score = runRepairPartitioning(versions, IDsToWords, offsetsAllVersions, versionPartitionSizes, associations, minFragSize, repairStoppingPoint, false);

		return score;
	}
}