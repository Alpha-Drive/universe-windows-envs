#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <sstream>
#include <vector>
#include <json/json.h>
#include <iostream>

using namespace std;

inline void split(const string &s, char delim, vector<string> &elems)
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


inline vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

inline std::string json_dumps(Json::Value const& root)
{
	std::ostringstream stream;
	stream << root;
	return stream.str();
}

inline Json::Value json_loads(std::string const& json_string) 
{
	Json::Value root;
	Json::Reader reader;
	bool parse_success = reader.parse( json_string.c_str(), root );
    if ( ! parse_success )
    {
        std::cout  << "Failed to parse JSON from: " << json_string << std::endl;
		return Json::nullValue;
    }
	else
	{
		return root;
	}
}

#endif // !UTILS_H_