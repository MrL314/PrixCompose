

#include "ByteDictionary.h"
#include "Range.h"
#include "Utility.h"

#include <iostream>
#include <map>
#include <vector>



ByteDictionary::ByteDictionary()
{
	buffer.clear();
	bufferLength = 0;

}

ByteDictionary::ByteDictionary(std::vector<u8>& buff)
{
	bufferLength = buff.size();

	buffer = std::vector<u8>(buff);

	for (u32 i = 0; i < bufferLength; i++)
		Add(buffer[i], i);
}


void ByteDictionary::Add(u8 value, u32 offset)
{
	if (byteDict.count(value) == 0)
	{
		std::vector<u32> new_dict_entry = std::vector<u32>();
		byteDict[value] = new_dict_entry;
	}

	byteDict[value].push_back(offset);
}


Range ByteDictionary::GetMaxBackRange(u32 offset)
{
	Range maxBackRange = Range();
	u8 value = buffer[offset];

	u32 otherOffset;
	u32 iterator;
	u32 backIterator;


	for (auto it = byteDict[value].begin(); it != byteDict[value].end(); ++it)
	{
		otherOffset = *it;

		if (otherOffset >= offset) break;

		iterator = offset;
		backIterator = otherOffset;

		do { 
			iterator++; backIterator++;
		} while ((iterator < bufferLength) && (buffer[backIterator] == buffer[iterator]));

		
		// start = otherOffset
		// end = backIterator

		Range backRange = Range(otherOffset, backIterator);

		if (backRange.getLen() > SUPER_COMMAND_MAX) backRange.setLen(0);

		if (backRange.getLen() >= maxBackRange.getLen())
		{
			maxBackRange.start = backRange.start;
			maxBackRange.end = backRange.end;
		}

	}

	return maxBackRange;
}


Range ByteDictionary::GetInvertMaxBackRange(u32 offset)
{
	Range maxBackRange = Range();
	u8 value = buffer[offset] ^ 0xff;

	if (byteDict.count(value) == 0) return maxBackRange;

	u32 otherOffset;
	u32 iterator;
	u32 backIterator;

	for (auto it = byteDict[value].begin(); it != byteDict[value].end(); ++it)
	{
		otherOffset = *it;

		if (otherOffset >= offset) break;

		iterator = offset;
		backIterator = otherOffset;

		do {
			iterator++; backIterator++;
		} while ((iterator < bufferLength) && (buffer[backIterator] == (buffer[iterator] ^ 0xff)));


		// start = otherOffset
		// end = backIterator

		Range backRange = Range(otherOffset, backIterator);

		if (backRange.getLen() > SUPER_COMMAND_MAX) backRange.setLen(0);

		if (backRange.getLen() >= maxBackRange.getLen())
		{
			maxBackRange.start = backRange.start;
			maxBackRange.end = backRange.end;
		}

	}

	return maxBackRange;
}


/*
Range* ByteDictionary::GetMaxBackRanges(u32 offset)
{
	Range maxRange4n = Range();
	Range maxRange4s = Range();
	Range maxRange6n = Range();
	Range maxRange6s = Range();

	u8 value = buffer[offset];

	u32 startPosition;
	if (offset < 256) startPosition = 0;
	else startPosition = offset - 256;




	u32 otherOffset;
	u32 iterator;
	u32 backIterator;


	for (auto it = byteDict[value].begin(); it != byteDict[value].end(); ++it)
	{
		otherOffset = *it;

		if (otherOffset >= offset) break;

		iterator = offset;
		backIterator = otherOffset;

		do {
			iterator++; backIterator++;
		} while ((iterator < bufferLength) && (buffer[backIterator] == buffer[iterator]));


		// start = otherOffset
		// end = backIterator

		Range backRange = Range(otherOffset, backIterator);

		if (backRange.getLen() > SUPER_COMMAND_MAX) backRange.setLen(0);

		if (otherOffset < startPosition)
		{
			if (backRange.getLen() >= maxRange4s.getLen())
			{
				maxRange4s.start = backRange.start;
				maxRange4s.end = backRange.end;
			}
		}
		else
		{
			if (backRange.getLen() >= maxRange6s.getLen())
			{
				maxRange4s.start = backRange.start;
				maxRange4s.end = backRange.end;
			}
		}
	}

	maxRange6n.start = maxRange6s.start;
	maxRange6n.end = maxRange6s.end;

	if (maxRange6n.getLen() <= NORMAL_COMMAND_MAX) maxRange6s = Range();
	else maxRange6n.setLen(NORMAL_COMMAND_MAX);

	maxRange4n.start = maxRange4s.start;
	maxRange4n.end = maxRange4s.end;

	if (maxRange4n.getLen() <= NORMAL_COMMAND_MAX) maxRange4s = Range();
	else maxRange4n.setLen(NORMAL_COMMAND_MAX);
	

	if (maxRange4n.getLen() == maxRange6n.getLen()) maxRange4n = Range();
	if (maxRange4s.getLen() == maxRange6s.getLen()) maxRange4s = Range();

	Range ranges[] = { maxRange4n, maxRange4s, maxRange6n, maxRange6s };

	return ranges;
}
*/