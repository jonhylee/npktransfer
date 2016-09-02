#pragma once

#include <string>
#include <map>

class JsonParsor
{
public:
	JsonParsor(void);
	~JsonParsor(void);

	bool Parse(const std::string buf, std::map<std::string, std::string> &outMap);
};
