#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <dirent.h>
#include <string>
#include <vector>
#include <iostream>
#include "Tokenizer.h"

inline std::string getFileName(const std::string& filepath)
{
	std::vector<std::string> tokens;
	std::string delimiters = "/\\";
	bool trimEmpty = false;
	tokenize(filepath.c_str(), tokens, delimiters, trimEmpty);
	return tokens.back();
}

inline std::string stripDot(const std::string& filepath)
{
// 	std::vector<std::string> tokens;
// 	bool trimEmpty = false;
// 	tokenize(filepath, tokens, ".", trimEmpty);
// 	std::string s = tokens[0];
	return filepath.substr(1, filepath.size());
}

inline int getFileNames(const std::string& dir, std::vector<std::string>& files)
{
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        std::cerr << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
    	if (! (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == ".." ) )
        	files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}


inline char* getText(const std::string& filename, int& length)
{
    char* buffer;

    std::ifstream is;
    is.open ( filename.c_str(), std::ios::binary );

    if (is.fail())
    {
        std::cerr << "Could not open input file (it might be a directory). Moving on..." << std::endl;
        return NULL;
    }
    // get length of file:
    is.seekg (0, std::ios::end);
    length = is.tellg();
    is.seekg (0, std::ios::beg);

    // allocate memory:
    buffer = new char [length];

    // read data as a block:
    is.read (buffer,length);
    is.close();

    return buffer;
}

#endif