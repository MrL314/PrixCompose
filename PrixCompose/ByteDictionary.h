
#include <iostream>
#include <vector>
#include <map>

#include "Range.h"

#ifndef BYTEDICTIONARY_H
#define BYTEDICTIONARY_H



struct ByteDictionary
{
	std::vector<u8> buffer;
	u32 bufferLength;

	std::map<u8, std::vector<u32>> byteDict;



	ByteDictionary();
	ByteDictionary(std::vector<u8>& buff);

	void Add(u8 value, u32 offset);

	Range GetMaxBackRange(u32 offset);
	Range GetInvertMaxBackRange(u32 offset);

	///Range* GetMaxBackRanges(u32 offset);

};






#endif
