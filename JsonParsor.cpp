#include "JsonParsor.h"
#include <json/json.h>

JsonParsor::JsonParsor(void)
{
}

JsonParsor::~JsonParsor(void)
{
}

bool JsonParsor::Parse(const std::string buf, std::map<std::string, std::string> &outMap)
{
	char buffer[512] = {0};
	Json::Reader reader;
	Json::Value value;
	if(reader.parse(buf, value))
	{
		Json::Value::Members member = value.getMemberNames();
		for(Json::Value::Members::iterator it = member.begin(); it != member.end(); it++)
		{
			const Json::Value keyVal = value[*it];
			std::string keyValue;
			if (keyVal.isString())
			{
				keyValue = keyVal.asString();
			}
			else if (keyVal.isInt())
			{
				sprintf(buffer, "%d", keyVal.asInt());
				keyValue.append(std::string(buffer));
			}
			else if (keyVal.isNumeric())
			{
				sprintf(buffer, "%f", keyVal.asDouble());
				keyValue.append(std::string(buffer));
			}
			else if (keyVal.isUInt())
			{
				sprintf(buffer, "%d", keyVal.asUInt());
				keyValue.append(std::string(buffer));
			}
			outMap.insert(std::make_pair(*it, keyValue));
		}
	}
	return true;
}
