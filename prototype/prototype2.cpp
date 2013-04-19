#include "prototype2.h"
using namespace std;

void Prototype2::printIDtoWordMapping(unordered_map<unsigned, string>& IDsToWords, ostream& os)
{
	for (unordered_map<unsigned, string>::iterator it = IDsToWords.begin(); it != IDsToWords.end(); it++)
	{
		os << it->first << ": " << it->second << endl;
	}
}

void Prototype2::writeAssociations(const vector<Association>& associations, ostream& os)
{
	for (size_t i = 0; i < associations.size(); i++)
	{
		os << associations[i];
	}
}

double Prototype2::runRepairPartitioning(vector<vector<unsigned> > versions, unordered_map<unsigned, string>& IDsToWords, 
	unsigned*& offsetsAllVersions, unsigned*& versionPartitionSizes, vector<Association>& associations,
	unsigned minFragSize, unsigned repairStoppingPoint, unsigned numLevelsDown, bool printFragments, bool printAssociations)
{
	// Allocate the heap, hash table, array of associations, and list of pointers to neighbor structures	
	RandomHeap myHeap;
	
	unordered_map<unsigned long long, HashTableEntry*> hashTable = unordered_map<unsigned long long, HashTableEntry*> ();
	
	associations = vector<Association>();
	
	vector<VersionDataItem> versionData = vector<VersionDataItem>();

	RepairTree repairTree;

	// Run through the string and grab all the initial pairs
	// Add them to all the structures
	extractPairs(versions, myHeap, hashTable, versionData, repairTree);

	// Replace pairs with symbols until done (either some early stop condition or one symbol left)
	doRepair(myHeap, hashTable, associations, repairStoppingPoint, versionData, repairTree);

	// Use the output of repair to build a set of repair trees (one per version)
	getTrees(associations, versionData);

	// Use the result of repair to get a partitioning of the document
	// Hopefully this partitioning gives us fragments that occur many times
	RepairDocumentPartition partition = RepairDocumentPartition(repairTree, versionData, numLevelsDown);

	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	offsetsAllVersions = partition.getOffsets();

	// The number of fragments in each version
	versionPartitionSizes = partition.getVersionSizes();

	// unsigned totalFrags(0);
	// for (unsigned i = 0; i < versions.size(); i++)
	// {
	// 	cerr << "Version: " << i << endl;
	// 	for (unsigned j = 0; j < versionPartitionSizes[i]; j++)
	// 	{
	// 		cerr << offsetsAllVersions[totalFrags] << ",";
	// 		totalFrags++;
	// 	}
	// 	cerr << endl;
	// }

	partition.writeResults(versions, IDsToWords, "./Output/results.txt", printFragments, printAssociations);

	if (printAssociations)
	{
		cerr << "*** Associations (symbol -> pair) ***" << endl;
		writeAssociations(associations, cerr);
	}

	return partition.getScore();
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
		string inputFilepath = "./Input/ints/";

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
		unsigned repairStoppingPoint = 1; //pairs that occur less than this amount of times will not be replaced

		unsigned numLevelsDown = 3;

		if (argc == 2 && (string) argv[1] == "help")
		{
			cerr << "Usage:" << endl;
			cerr << "\t" << argv[0] << " <directory> <numLevelsDown> <minFragSize>" << endl;
			cerr << "\t" << argv[0] << " <directory> <numLevelsDown>" << endl;
			cerr << "\t" << argv[0] << " <directory>" << endl;
			cerr << "\t" << argv[0] << "" << endl << endl;
			cerr << "Defaults: " << endl;
			cerr << "\tdirectory: " << inputFilepath << endl;
			cerr << "\tnumLevelsDown: " << numLevelsDown << endl;
			cerr << "\tminFragSize: " << minFragSize << endl;			
			// cerr << "\trepairStoppingPoint: " << repairStoppingPoint << endl;
			exit(0);
		}

		if (argc > 1)
			inputFilepath = (string)argv[1];

		if (argc > 2)
			numLevelsDown = atoi(argv[2]);

		if (argc > 3)
			minFragSize = atoi(argv[3]);

		// if (argc > 3)
		// 	repairStoppingPoint = atoi(argv[3]);
		
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
			// cerr << "Version " << i << endl;
			// for (unsigned j = 0; j < wordIDs.size(); j++)
			// {
			// 	cerr << wordIDs[j] << ",";
			// }
			// cerr << endl << endl;
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

		double score = runRepairPartitioning(versions, IDsToWords, offsetsAllVersions, versionPartitionSizes, 
			associations, minFragSize, repairStoppingPoint, numLevelsDown, false, false);

		return score;
	}
}