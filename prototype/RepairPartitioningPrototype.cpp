#include "RepairPartitioningPrototype.h"
#include <time.h>
using namespace std;

double RepairPartitioningPrototype::getScore(ostream& os)
{
	double term;
	double sum(0);
	double totalFragSize(0);
	for (auto it = uniqueFrags.begin(); it != uniqueFrags.end(); it++)
	{
		term = it->second.count * it->second.fragSize;
		sum += term;
		totalFragSize += it->second.fragSize;
	}
	if (uniqueFrags.size() == 0) return 0.0;

	double avgFragSize = totalFragSize / uniqueFrags.size();
	return sum / (uniqueFrags.size() * avgFragSize);
}

void RepairPartitioningPrototype::updateUniqueFragmentHashMap()
{
	unordered_map<string, FragInfo>::iterator it;
	for (size_t i = 0; i < this->fragments.size(); i++)
	{
		for (size_t j = 0; j < this->fragments[i].size(); j++)
		{
			FragInfo frag = this->fragments[i][j];
			string myHash = frag.hash;

			unsigned theID;
			if (this->uniqueFrags.count(myHash))
			{
				// Found it, so use its ID
				theID = this->uniqueFrags[myHash].id;
				this->uniqueFrags[myHash].count++;
			}
			else
			{
				// Didn't find it, give it an ID
				theID = nextFragID();
				frag.id = theID;
				frag.count = 1;
				this->uniqueFrags[myHash] = frag;
			}
		}
	}
}

void RepairPartitioningPrototype::writeResults(
	const vector<vector<unsigned> >& versions, 
	const BaseFragmentsAllVersions& baseFragmentsAllVersions,
	unordered_map<unsigned, string>& IDsToWords,
	const string& outFilename)
{
	ofstream os(outFilename.c_str());

	os << "Results of re-pair partitioning..." << endl << endl;
	os << "*** Base Fragments ***" << endl;
	
	BaseFragment frag;
	BaseFragmentList baseFragList;
	assert(versions.size() == baseFragmentsAllVersions.size());
	for (auto it = baseFragmentsAllVersions.begin(); it != baseFragmentsAllVersions.end(); ++it) {
		baseFragList = (*it);
		unsigned versionNum = baseFragList.getVersionNum();
		cerr << "Version " << versionNum << "..." << endl;
		for (size_t i = 0; i < baseFragList.size(); ++i) {
			frag = baseFragList.get(i);
			cerr << "Start: " << frag.start <<  ", End: " << frag.end << endl;
		}
	}

	os.close();
}


void RepairPartitioningPrototype::printIDtoWordMapping(unordered_map<unsigned, 
	string>& IDsToWords, ostream& os) const
{
	for (auto it = IDsToWords.begin(); it != IDsToWords.end(); it++)
	{
		os << it->first << ": " << it->second << endl;
	}
}

void RepairPartitioningPrototype::writeAssociations(
	const unordered_map<unsigned, Association>& associations, ostream& os) const
{
	// TODO use iterators
//	for (size_t i = 0; i < associations.size(); i++)
//	{
//		os << associations[i];
//	}
}

// Break this up into api functions: repair + partitioning
/*

repairApi->doRepair(); // does repair and sets up associations
repairApi->getBaseFrags(numLevelsDown); // builds trees and returns base fragments

*/
double RepairPartitioningPrototype::runRepairPartitioning(
	vector<vector<unsigned> > versions,
	BaseFragmentsAllVersions& baseFragmentsAllVersions,
	unsigned numLevelsDown)
{
	bool debug = false;

	RepairAlgorithm repairAlg(versions);

	repairAlg.doRepair();

	repairAlg.clearRepairStructures();

	// Declare and instantiate here, populate in the function call below
	BaseFragmentsAllVersions baseFragsAllVersions = BaseFragmentsAllVersions();

	// Base fragments get populated here
	repairAlg.getBaseFragments(baseFragmentsAllVersions, numLevelsDown);
	
	repairAlg.clearAssociationsAndReset();

	// if (debug)
	// 	checkOffsets(versions, offsetsAllVersions, versionPartitionSizes);

	// We don't yet have a way to calculate score, TODO
	double score = 0.0;
	return score;
}

void RepairPartitioningPrototype::checkOffsets(
	const vector<vector<unsigned> >& versions,
	unsigned* offsetsAllVersions,
	unsigned* versionPartitionSizes) const
{
	// Offsets must be sorted increasing for each version
	unsigned totalOffsets = 0;
	for (size_t i = 0; i < versions.size(); i++)
	{
		for (size_t j = 0; j < versionPartitionSizes[i] - 1; j++)
		{
			if (!(offsetsAllVersions[totalOffsets] <= offsetsAllVersions[totalOffsets + 1])) {
				cerr << "offsetsAllVersions[" << totalOffsets - 1 << "]: " << offsetsAllVersions[totalOffsets - 1];
				cerr << "offsetsAllVersions[" << totalOffsets << "]: " << offsetsAllVersions[totalOffsets];
				cerr << "offsetsAllVersions[" << totalOffsets + 1 << "]: " << offsetsAllVersions[totalOffsets + 1];
				cerr << "offsetsAllVersions[" << totalOffsets + 2 << "]: " << offsetsAllVersions[totalOffsets + 2];
			}
			assert(offsetsAllVersions[totalOffsets] <= offsetsAllVersions[totalOffsets + 1]);
			totalOffsets++;
		}
		// Every version must have at least 2 fragment boundaries
		assert(versionPartitionSizes[i] > 1 && versionPartitionSizes[i] <= MAX_NUM_FRAGMENTS_PER_VERSION);
		totalOffsets++;
	}
}

int RepairPartitioningPrototype::run(int argc, char* argv[])
{
	// heap, repair
	string test = "repair";
 
	if (test == "heap")
	{
		IndexedHeapTest test = IndexedHeapTest(1000000);
		exit(0);
	}

	if (test == "repair")
	{
		string inputFilepath = "./Input/ints/";

		/*
		I think that minFragSize should be proportional to document size
		*/
		unsigned minFragSize = 10; //in words

		// pairs that occur less than this amount of times will not be replaced
//		unsigned repairStoppingPoint = 1;

		/*
		To what extent are we willing to fragment?
		See the partitioning algorithm for how this is used
		*/
		float fragmentationCoefficient = 1.0;

		/*
		A variable used in the naive way to partition the tree:
		just go n levels down and grab all the nodes
		*/
		unsigned numLevelsDown = 5;

		if (argc == 2 && (string) argv[1] == "help")
		{
			cerr << "Usage:" << endl;
			
			cerr << "\t" << argv[0] <<
				" <directory> <fragmentationCoefficient> <minFragSize> <numLevelsDown>"
				<< endl;

			cerr << "\t" << argv[0] <<
				" <directory> <fragmentationCoefficient> <minFragSize>" << endl;
			
			cerr << "\t" << argv[0] <<
				" <directory> <fragmentationCoefficient>" << endl;
			
			cerr << "\t" << argv[0] << " <directory>" << endl;
			
			cerr << "\t" << argv[0] << "" << endl << endl;
			
			cerr << "Defaults: " << endl;
			
			cerr << "\tdirectory: " << inputFilepath << endl;
			
			cerr << "\tfragmentationCoefficient: " <<
				fragmentationCoefficient << endl;
			
			cerr << "\tminFragSize: " << minFragSize << endl;
			
			cerr << "\tnumLevelsDown: " << numLevelsDown << endl;
			
			exit(0);
		}

		if (argc > 1)
			inputFilepath = (string)argv[1];

		if (argc > 2)
			fragmentationCoefficient = atof(argv[2]);

		if (argc > 3)
			minFragSize = atoi(argv[3]);
		
		if (argc > 4)
			numLevelsDown = atoi(argv[4]);

		
		vector<string> inputFilenames = vector<string>();
		if (getFileNames(inputFilepath, inputFilenames))
			return errno;

		char* text;
		
		int fileSize;
		
		//each inner vector is the wordIDs for one version
		vector<vector<unsigned> > versions = vector<vector<unsigned> >();
		
		vector<unsigned> wordIDs;
		
		unordered_map<unsigned, string> IDsToWords = unordered_map<unsigned, string>();

		unordered_map<unsigned, unsigned> uniqueWordIDs = unordered_map<unsigned, unsigned>();

		wordIDs = vector<unsigned>();

		// wordIDs.push_back(1);
		// wordIDs.push_back(2);
		// wordIDs.push_back(2);
		// wordIDs.push_back(2);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(4);
		// wordIDs.push_back(2);
		// wordIDs.push_back(3);
		// versions.push_back(wordIDs);
		// wordIDs.clear();

		// wordIDs.push_back(1);
		// wordIDs.push_back(2);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(0); // Important edge case
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(3);
		// wordIDs.push_back(4);
		// wordIDs.push_back(1);
		// wordIDs.push_back(2);
		// versions.push_back(wordIDs);
		// wordIDs.clear();

		// wordIDs.push_back(2);
		// versions.push_back(wordIDs); // Another edge case, adding an empty version

		// currentWordID = 4;

		for (unsigned i = 0; i < inputFilenames.size(); i++)
		{
			stringstream filenameSS;
			filenameSS << inputFilepath << inputFilenames[i];
			string filename = filenameSS.str();
			text = getText(filename, fileSize);
			if (!text)
				continue;
			wordIDs = stringToWordIDs(text, IDsToWords, uniqueWordIDs);

			if (false)
			{
				cerr << "Version " << i << endl;
				for (unsigned j = 0; j < wordIDs.size(); j++)
				{
					cerr << wordIDs[j] << ",";
				}
				cerr << endl << endl;
			}

			versions.push_back(wordIDs);
			delete [] text;
			text = NULL;
		}
		wordIDs.clear();
		
		// By this time, IDsToWords should contain the mappings of
		// IDs to words in all versions
		// printIDtoWordMapping(IDsToWords);
		// system("pause");

		// unsigned* versionPartitionSizes = new unsigned[versions.size()];
		// unsigned* offsetsAllVersions = 
		// 	new unsigned[versions.size() * MAX_NUM_FRAGMENTS_PER_VERSION];

		BaseFragmentsAllVersions baseFragmentsAllVersions = BaseFragmentsAllVersions();

		double score = 0.0;
		
		clock_t init, final;

		init = clock();

		try {
			score = runRepairPartitioning(
			 	versions,
			 	baseFragmentsAllVersions,
			 	numLevelsDown);

			string outputFilename = "./Output/results.txt";

			this->writeResults(versions, baseFragmentsAllVersions,
				 	IDsToWords, outputFilename);

			// stringstream command;
			// command << "start " << outputFilename.c_str();
			// system(command.str().c_str());
		} catch (int e) {
			cerr << "Error code: " << e << endl;
			exit(e);
		}

        // delete [] versionPartitionSizes;
        // delete [] offsetsAllVersions;

		final = clock()-init;
		cerr << (double)final / ((double)CLOCKS_PER_SEC) << endl;

		return score;
	}
	return 0;
}