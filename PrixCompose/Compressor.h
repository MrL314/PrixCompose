
#include <vector>

#include "Utility.h"
#include "Range.h"
#include "ByteDictionary.h"

#ifndef COMPRESSOR_H
#define COMPRESSOR_H




struct compBlock
{
	//std::vector<u8>::iterator dBuffIt;
	u32 dLen;
	u16 cLen;

	u8 command;
	u32 dBuffOff;
	bool extended = false;


	u32 comp_factor()
	{
		u32 c_size = (u32)extended + (u32)cLen + 1;
		if (c_size > dLen) return 0;
		else return dLen - c_size;
	}

	Range backRange;

};



struct Hunk
{

	u32 numBlocks;
	std::vector<compBlock*> blocks;

	u32 dBuff_start;
	u32 Len;


	u32 getLen()
	{
		Len = 0;
		for (auto b : blocks)
		{

			Len += b->cLen; // add size of compressed payload
			Len++; // add command byte

			if (b->extended) Len++; // add extended size byte

		}

		return Len;
	}


};





std::vector<u8> compress(u8* decompressedBuffer, u32 bufferSize);
std::vector<u8> compress(std::vector<u8>& decompressedBuffer);


compBlock* bestBlockAtIndex(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict);

compBlock* getGreedyBlock(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict);

std::vector<compBlock*> greedyCompress(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict);

//std::vector<compBlock*> optimalCompress(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, u32 bufferSize, ByteDictionary& bdict);



#endif