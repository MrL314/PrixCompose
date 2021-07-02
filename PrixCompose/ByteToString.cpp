
#include <string>

#include "ByteToString.h"
#include "Utility.h"



std::string ToHexString(u8 val)
{
	std::stringstream ss;
	ss << std::hex << (int)val;
	std::string out_str = ss.str();
	while (out_str.length() < 2)
		out_str = '0' + out_str;
	return out_str;
}

std::string ToHexString(u16 val)
{
	std::stringstream ss;
	ss << std::hex << (int)val;
	std::string out_str = ss.str();
	while (out_str.length() < 4)
		out_str = '0' + out_str;

	return out_str;
}