#include "RepairPartitioningPrototype.h"
using namespace std;

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

double RepairPartitioningPrototype::runRepairPartitioning(vector<vector<unsigned> > versions, 
	unordered_map<unsigned, string>& IDsToWords, 
	unsigned*& offsetsAllVersions, 
	unsigned*& versionPartitionSizes, 
	vector<Association>& associations,
	unsigned minFragSize, 
	float fragmentationCoefficient, 
	unsigned repairStoppingPoint, 
	unsigned numLevelsDown, 
	unsigned method,
	bool printFragments, 
	bool printAssociations)
{
	RepairAlgorithm repairAlg(versions);

	repairAlg.run();

	vector<VersionDataItem> versionData = repairAlg.getVersionData();

	associations = repairAlg.getAssociations();

	// Use the result of repair to get a partitioning of the document
	// Hopefully this partitioning gives us fragments that occur many times
	RepairDocumentPartition partition = RepairDocumentPartition(versionData, associations, numLevelsDown, minFragSize, fragmentationCoefficient, method);

	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	offsetsAllVersions = partition.getOffsets();

	// The number of fragments in each version
	versionPartitionSizes = partition.getVersionSizes();

	string outputFilename = "./Output/results.txt";

	partition.writeResults(versions, IDsToWords, outputFilename, printFragments, printAssociations);

	if (printAssociations)
	{
		cerr << "*** Associations (symbol -> pair) ***" << endl;
		writeAssociations(associations, cerr);
	}

	stringstream command;
	command << "start " << outputFilename.c_str();
	system(command.str().c_str());

	return partition.getScore();
}


double RepairPartitioningPrototype::runRepairPartitioning(vector<vector<unsigned> > versions, 
	unsigned*& offsetsAllVersions, 
	unsigned*& versionPartitionSizes, 
	vector<Association>& associations,
	unsigned minFragSize, 
	float fragmentationCoefficient, 
	unsigned method)
{
	RepairAlgorithm repairAlg(versions);

	repairAlg.run();

	vector<VersionDataItem> versionData = repairAlg.getVersionData();

	associations = repairAlg.getAssociations();

	// Use the result of repair to get a partitioning of the document
	// Hopefully this partitioning gives us fragments that occur many times
	RepairDocumentPartition partition = RepairDocumentPartition(versionData, associations, 
		1, minFragSize, fragmentationCoefficient, method);

	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	offsetsAllVersions = partition.getOffsets();

	// The number of fragments in each version
	versionPartitionSizes = partition.getVersionSizes();

	// string outputFilename = "./Output/results.txt";

	// partition.writeResults(versions, IDsToWords, outputFilename, false, false);

	// if (printAssociations)
	// {
	// 	cerr << "*** Associations (symbol -> pair) ***" << endl;
	// 	writeAssociations(associations, cerr);
	// }

	// stringstream command;
	// command << "start " << outputFilename.c_str();
	// system(command.str().c_str());

	return partition.getScore();
}



int RepairPartitioningPrototype::run(int argc, char* argv[])
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
		unsigned method = 0;

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

		double score = runRepairPartitioning(versions, IDsToWords, offsetsAllVersions, versionPartitionSizes, 
			associations, minFragSize, fragmentationCoefficient, repairStoppingPoint, numLevelsDown, method, true, false);

		return score;
	}
}