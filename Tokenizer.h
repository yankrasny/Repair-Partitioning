#include <string>

/*
Borrowed from Sergey N.
*/
void tokenize(const std::string& str,  std::vector<std::string>& tokens,
              const std::string& delimiters, const bool trimEmpty) 
{
        std::string::size_type pos, lastPos = 0;
        while(true)
        {
                pos = str.find_first_of(delimiters, lastPos);
                if(pos == std::string::npos)
                {
                        pos = str.length();

                        if(pos != lastPos || !trimEmpty)
                                tokens.push_back(std::string(str.data()+lastPos,
                                                (std::string::size_type)pos-lastPos ));

                        break;
                }
                else
                {
                        if(pos != lastPos || !trimEmpty)
                                tokens.push_back(std::string(str.data()+lastPos,
                                                (std::string::size_type)pos-lastPos ));
                }

                lastPos = pos + 1;
        }
}