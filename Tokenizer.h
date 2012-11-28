#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

/*
Borrowed from Sergey N.
*/
void tokenize(const std::string& str,  std::vector<std::string>& tokens,
              const std::string& delimiters, const bool trimEmpty);

#endif