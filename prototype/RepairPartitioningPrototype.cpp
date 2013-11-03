#include "RepairPartitioningPrototype.h"
using namespace std;

// void RepairPartitioningPrototype::setFragmentInfo(
// 	const vector<vector<unsigned> >& versions, 
// 	ostream& os, 
// 	bool print)
// {
// 	os << "*** Fragments ***" << endl;

// 	unsigned start, end, theID, fragSize;
// 	string word;
// 	vector<unsigned> wordIDs;
	
// 	MD5 md5;
// 	char* concatOfWordIDs;
	
// 	unsigned totalCountFragments(0);

// 	// Iterate over versions
// 	for (unsigned v = 0; v < versions.size(); v++)
// 	{
// 		wordIDs = versions[v]; // all the word IDs for version v
// 		this->fragments.push_back(vector<FragInfo >());
// 		if (print)
// 			os << "Version " << v << endl;

// 		// One version: iterate over the words in that version
// 		for (unsigned i = 0; i < this->versionSizes[v] - 1; i++)
// 		{			
// 			start = this->offsets[totalCountFragments + i];
// 			end = this->offsets[totalCountFragments + i + 1];
// 			fragSize = end - start;
// 			if (print)
// 				os << "Fragment " << i << ": ";
// 			stringstream ss;
// 			for (unsigned j = start; j < end; j++)
// 			{
// 				theID = wordIDs[j];

// 				// Need a delimiter to ensure uniqueness (1,2,3 is different from 12,3)
// 				ss << theID << ",";
// 			}

// 			// Store the concatenation of the IDs for this fragment
// 			concatOfWordIDs = new char[MAX_FRAG_LENGTH];
// 			strcpy(concatOfWordIDs, ss.str().c_str());
			
// 			// Calculate the hash of the fragment
// 			string hash; // = new char[128]; // md5 produces 128 bit output
// 			hash = md5.digestString(concatOfWordIDs);
// 			this->fragments[v].push_back(FragInfo(0, 0, fragSize, hash));
// 			ss.str("");
			
// 			if (print)
// 				os << "hash (" << hash << ")" << endl;
// 			delete [] concatOfWordIDs;
// 			concatOfWordIDs = NULL;
// 		}
// 		totalCountFragments += this->versionSizes[v];
// 		if (print)
// 			os << endl;
// 	}
// }


double RepairPartitioningPrototype::getScore(ostream& os)
{
	double term;
	double sum(0);
	double totalFragSize(0);
	for (unordered_map<string, FragInfo>::iterator it = uniqueFrags.begin(); it != uniqueFrags.end(); it++)
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
	unordered_map<unsigned, string>& IDsToWords, 
	const string& outFilename)
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
			if (i < numFragsInVersion - 1)
			{
				unsigned currOffset = this->offsetsAllVersions[totalCountFragments + i];
				unsigned nextOffset = this->offsetsAllVersions[totalCountFragments + i + 1];
				diff = nextOffset - currOffset;
			}
			else
			{
				diff = 0;
			}

			os << "Fragment " << i << ": " << this->offsetsAllVersions[totalCountFragments + i] << "-" << 
				this->offsetsAllVersions[totalCountFragments + i + 1] << " (frag size: " << diff << ")" << endl;
		}
		totalCountFragments += numFragsInVersion;
		os << endl;
	}

	// Assign fragment IDs and stick them in a hashmap
	unordered_map<string, FragInfo> uniqueFrags;
	this->updateUniqueFragmentHashMap();

	// Now decide on the score for this partitioning
	double score = this->getScore(os);
	os << "Score: " << score << endl;
	os.close();
}


void RepairPartitioningPrototype::printIDtoWordMapping(unordered_map<unsigned, string>& IDsToWords, ostream& os)
{
	for (unordered_map<unsigned, string>::iterator it = IDsToWords.begin(); it != IDsToWords.end(); it++)
	{
		os << it->first << ": " << it->second << endl;
	}
}

void RepairPartitioningPrototype::writeAssociations(const vector<Association>& associations, ostream& os)
{
	for (size_t i = 0; i < associations.size(); i++)
	{
		os << associations[i];
	}
}

// Run each step of the algorithm, checking for sanity in between
double RepairPartitioningPrototype::runRepairPartitioning(
	vector<vector<unsigned> > versions, 
	unordered_map<unsigned, string>& IDsToWords, 
	unsigned*& offsetsAllVersions, 
	unsigned*& versionPartitionSizes, 
	vector<Association>& associations,
	unsigned minFragSize, 
	float fragmentationCoefficient, 
	unsigned method)
{
	bool debug = false;

	// don't really need numLevelsDown for now
	unsigned numLevelsDown = 5;
	RepairAlgorithm repairAlg(versions, numLevelsDown, minFragSize, fragmentationCoefficient);

	associations = repairAlg.getAssociations();

	// associations should be sorted by the symbol on the left
	// repair assigns an incrementing ID
	if (debug) {
		for (size_t i = 0; i < associations.size() - 1; i++) {
			// cerr << associations[i].getSymbol() << ": (" << associations[i].getLeft() << ", " << associations[i].getRight() << ")" << endl;
			// cerr << "Versions: {" << associations[i].getVersionString() << "}" << endl;
			if (associations[i].getSymbol() > associations[i+1].getSymbol()) {
				throw 1;
			}
			std::multiset<unsigned> versionsForAssociation = associations[i].getVersions();
			for (std::multiset<unsigned>::iterator it = versionsForAssociation.begin(); it != versionsForAssociation.end(); it++) {
				// (*it) should be an unsigned representing the version number. that number can't be higher than numVersions
				unsigned vNum = *it;
				// cerr << "vNum: " << vNum << endl;
				if (vNum > versions.size()) {
					throw 2;
				}
			}
		}
		// cerr << associations.back().getSymbol() << ": (" << associations.back().getLeft() << ", " << associations.back().getRight() << ")" << endl;
		// cerr << "Versions: {" << associations.back().getVersionString() << "}" << endl;
	}

	this->offsetsAllVersions = offsetsAllVersions = repairAlg.getOffsetsAllVersions();

	this->versionPartitionSizes = versionPartitionSizes = repairAlg.getVersionPartitionSizes();

	// offsets must be sorted for each version
	// think about it, can a version be partitioned like this? [0, 15, 29, 23, ...] No.
	unsigned totalOffsets = 0;
	if (debug) {
		for (size_t i = 0; i < versions.size(); i++) {
			for (size_t j = 0; j < versionPartitionSizes[i] - 1; j++) {
				// cerr << offsetsAllVersions[totalOffsets] << ",";
				if (offsetsAllVersions[totalOffsets] > offsetsAllVersions[totalOffsets+1]) {
					throw 3;
				}
				totalOffsets++;
			}
			// cerr << endl;
			totalOffsets++;
		}
	}

	repairAlg.cleanup();

	// string outputFilename = "./Output/results.txt";

	// this->writeResults(versions, IDsToWords, outputFilename);

	// stringstream command;
	// command << "start " << outputFilename.c_str();
	// system(command.str().c_str());

	double score = 0.0;
	return score;
}

int RepairPartitioningPrototype::run(int argc, char* argv[])
{
	//createOutputDir();

	//heap, repair
	string test = "repair";
 
	if (test == "heap")
	{
		IndexedHeapTest test = IndexedHeapTest(100000);
		exit(0);
	}

	if (test == "repair")
	{
		Profiler::getInstance().start("all");
		string inputFilepath = "./Input/ints/";

		/*
		Initial tests show that minFragSize should be proportional to document size
		*/
		unsigned minFragSize = 10; //in words

		/*
		Initial tests show that repairStoppingPoint shouldn't be too small (repair goes too far for our purposes in this case)
		And it shouldn't be larger than the number of versions (this is trivial, we expect to get repetition 
		at most numVersions times for inter-version repetitions)
		*/
		unsigned repairStoppingPoint = 1; //pairs that occur less than this amount of times will not be replaced

		/* To what extent are we willing to fragment? See the partitioning algorithm for how this is used */
		float fragmentationCoefficient = 1.0;

		/* A variable used in the primitive way to partition the tree: just go n levels down */
		unsigned numLevelsDown = 3;

		/* The partitioning alg to use. See Partitioning.h for the enum */
		unsigned method = 1;

		if (argc == 2 && (string) argv[1] == "help")
		{
			cerr << "Usage:" << endl;
			cerr << "\t" << argv[0] << " <directory> <fragmentationCoefficient> <minFragSize> <method>" << endl;
			cerr << "\t" << argv[0] << " <directory> <fragmentationCoefficient> <minFragSize>" << endl;
			cerr << "\t" << argv[0] << " <directory> <fragmentationCoefficient>" << endl;
			cerr << "\t" << argv[0] << " <directory>" << endl;
			cerr << "\t" << argv[0] << "" << endl << endl;
			
			cerr << "Defaults: " << endl;
			cerr << "\tdirectory: " << inputFilepath << endl;
			cerr << "\tfragmentationCoefficient: " << fragmentationCoefficient << endl;
			cerr << "\tminFragSize: " << minFragSize << endl;
			cerr << "\tmethod: " << method << endl;
			exit(0);
		}

		if (argc > 1)
			inputFilepath = (string)argv[1];

		if (argc > 2)
			fragmentationCoefficient = atof(argv[2]);

		if (argc > 3)
			minFragSize = atoi(argv[3]);
		
		if (argc > 4)
			method = atoi(argv[4]);

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
			delete text;
			text = NULL;
		}
		
		// By this time, IDsToWords should contain the mappings of IDs to words in all versions
		// printIDtoWordMapping(IDsToWords);
		// system("pause");

		vector<Association> associations;

		unsigned* offsetsAllVersions(NULL);
		unsigned* versionPartitionSizes(NULL);

		double score = 0.0;
		/* Both overloads are shown below for testing. Just change the bool to switch. */
		if (false)
		{
			score = runRepairPartitioning(versions, IDsToWords, 
				offsetsAllVersions, versionPartitionSizes, 
				associations, minFragSize, 
				fragmentationCoefficient, 
				repairStoppingPoint, numLevelsDown,
				method, true, false);
		}
		else
		{
			try {
				score = runRepairPartitioning(
					versions, 
					IDsToWords,
					offsetsAllVersions,
					versionPartitionSizes, 
					associations,
					minFragSize,
					fragmentationCoefficient, 
					method);
			} catch (int e) {
				cerr << "Error code: " << e << endl;
				exit(e);
			}
		}

		return score;
	}
}