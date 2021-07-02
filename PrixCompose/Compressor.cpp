
#include <iostream>
#include <map>
#include <vector>


#include "Compressor.h"
#include "Range.h"
#include "ByteDictionary.h"
#include "ByteToString.h"
#include "ErrorHandle.h"



std::vector<u8> compress(u8* decompressedBuffer, u32 bufferSize)
{
	try
	{
		std::vector<u8> dbuff;
		for (u32 i = 0; i < bufferSize; i++)
		{
			dbuff.push_back(decompressedBuffer[i]);
		}

		return compress(dbuff);
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error when calling compress() with buffer pointer.");
	}
}


std::vector<u8> compress(std::vector<u8>& dBuff)
{

	try
	{
		ByteDictionary bdict(dBuff);

		/*
		for (auto it = bdict.byteDict.begin(); it != bdict.byteDict.end(); ++it)
		{
			std::cout << ToHexString(it->first) << ": ";
			for (auto i : it->second)
			{
				std::cout << i << " ";
			}
			std::cout << std::endl;
		}
		*/

		u32 dBuffSize = dBuff.size();


		// Phase 1: greedy compression
		std::vector<compBlock*> greedy_blocks = greedyCompress(dBuff, 0, dBuffSize, bdict);

		// Phase 2: create hunks

		/////

		std::vector<u8> compressed;
		//std::vector<u8>::iterator buffit = dBuff.begin();
		compBlock* curr_block = nullptr;
		for (auto git = greedy_blocks.begin(); git != greedy_blocks.end(); ++git)
		{
			curr_block = *git;
			u16 count = curr_block->dLen - 1;

			if (count > 0x1f) curr_block->extended = true;

			if (curr_block->extended)
			{
				//std::cout << "COUNT: " << ToHexString((u16)(count - 1)) << std::endl;

				//count += ((u16)curr_block->command) << 9;
				//count += 0xe0;

				compressed.push_back((u8)(0xe0 + (u8)(curr_block->command * 4) + ((count >> 8) & 0xff)));
				//std::cout << ToHexString((u8)(count / 256)) << " ";
				compressed.push_back((u8)(count & 255));
				//std::cout << ToHexString((u8)(count % 256)) << std::endl;
			}
			else
			{
				//std::cout << "COUNT: " << ToHexString((u8)(count - 1)) << std::endl;

				count += (u16)curr_block->command << 5;
				compressed.push_back((u8)count);
				//std::cout << ToHexString((u8)count) << std::endl;
			}

			u8 cmd = curr_block->command;
			u32 offset = curr_block->dBuffOff;

			if (cmd == 0)
			{
				for (u32 i = 0; i < curr_block->dLen; i++)
				{
					compressed.push_back(dBuff[offset + i]);
					//std::cout << ToHexString(dBuff[offset + i]) << " ";
				}
				//std::cout << std::endl;
			}
			else if (cmd == 1)
			{
				compressed.push_back(dBuff[offset]);
				//std::cout << ToHexString(dBuff[offset]) << std::endl;
			}
			else if (cmd == 2)
			{
				compressed.push_back(dBuff[offset]);
				compressed.push_back(dBuff[offset + 1]);
				//std::cout << ToHexString(dBuff[offset]) << " ";
				//std::cout << ToHexString(dBuff[offset+1]) << std::endl;
			}
			else if (cmd == 3)
			{
				compressed.push_back(dBuff[offset]);
				//std::cout << ToHexString(dBuff[offset]) << std::endl;
			}
			else if (cmd == 4)
			{
				u16 start_ptr = (u16)curr_block->backRange.start;
				compressed.push_back((u8)(start_ptr % 256) & 0xff);
				//std::cout << ToHexString((u8)(start_ptr % 256)) << " " ;
				compressed.push_back((u8)(start_ptr / 256) & 0xff);
				//std::cout << ToHexString((u8)(start_ptr / 256)) << std::endl;
			}
			else if (cmd == 5)
			{
				u16 start_ptr = (u16)curr_block->backRange.start;
				compressed.push_back((u8)(start_ptr % 256) & 0xff);
				//std::cout << ToHexString((u8)(start_ptr % 256)) << " ";
				compressed.push_back((u8)(start_ptr / 256) & 0xff);
				//std::cout << ToHexString((u8)(start_ptr / 256)) << std::endl;
			}
			else if (cmd == 6)
			{
				u8 back_ptr = (u8)(offset - curr_block->backRange.start);
				compressed.push_back(back_ptr);
				//std::cout << ToHexString((u16)offset) << ": " << ToHexString(dBuff[offset]) << " " << ToHexString((u8)back_ptr) << " " << ToHexString(dBuff[offset-back_ptr]) << " " << curr_block->dLen << std::endl;
			}
			else
			{
				for (u32 i = 0; i < curr_block->dLen; i++)
				{
					compressed.push_back(0xff);
					//std::cout << ToHexString((u8)0xff) << " ";
				}
				//std::cout << std::endl;
			}

			//std::cout << std::endl;
		}
		compressed.push_back(0xff);

		if (curr_block != nullptr) delete curr_block;


		return compressed;
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error during compression.");
	}
}


compBlock* bestBlockAtIndex(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict)
{
	try
	{
		u32 c1_max = 0;
		while ((start_ind + c1_max < end_ind) && (dBuff[start_ind] == dBuff[start_ind + c1_max])) c1_max++;

		u32 c2_max = 0;
		u32 j = 0;
		while ((start_ind + c2_max < end_ind) && (dBuff[start_ind + j] == dBuff[start_ind + c2_max]))
		{
			j = 1 - j;
			c2_max++;
		}

		u32 c3_max = 0;
		while ((start_ind + c3_max < end_ind) && ((dBuff[start_ind] + c3_max) % 256 == dBuff[start_ind + c3_max])) c3_max++;

		Range backrange = bdict.GetMaxBackRange(start_ind);
		Range backinvrange = bdict.GetInvertMaxBackRange(start_ind);

		u32 c4_max = backrange.getLen();
		u32 c5_max = backinvrange.getLen();
		u32 c6_max = 0;

		if (start_ind - backrange.start <= 0xff)
		{
			c6_max = c4_max;
			if (c6_max > 0xff) c6_max = 0xff;
			c4_max = 0;
		}


		// command 1
		u32 best_compression = c1_max - 1;
		u8 cmd = 1;
		u8 cLen = 1;

		Range bestRange = Range();

		// command 2
		if ((c2_max - 2) > best_compression)
		{
			best_compression = c2_max - 2;
			cmd = 2;
			cLen = 2;
		}

		// command 3
		if ((c3_max - 1) > best_compression)
		{
			best_compression = c3_max - 1;
			cmd = 3;
			cLen = 1;
		}

		// command 4
		if (c4_max >= 2)
		{
			if ((c4_max - 2) > best_compression)
			{
				best_compression = c4_max - 2;
				cmd = 4;
				cLen = 2;
				bestRange = backrange;
			}
		}

		// command 5
		if (c5_max >= 2)
		{
			if ((c5_max - 2) > best_compression)
			{
				best_compression = c5_max - 2;
				cmd = 5;
				cLen = 2;
				bestRange = backinvrange;
			}
		}

		// command 6
		if (c6_max >= 1)
		{
			if ((c6_max - 1) > best_compression)
			{
				best_compression = c6_max - 1;
				cmd = 6;
				cLen = 1;
				bestRange = backrange;
			}
		}


		compBlock* GreedyBlock = new compBlock;

		//GreedyBlock->dBuffIt = dBuff.begin() + start_ind;
		GreedyBlock->dLen = best_compression + cLen;
		GreedyBlock->cLen = cLen;
		if (best_compression + cLen > 0x20) GreedyBlock->extended = true;
		GreedyBlock->command = cmd;
		GreedyBlock->dBuffOff = start_ind;
		GreedyBlock->backRange = bestRange;

		return GreedyBlock;
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error during call to bestBlockAtIndex().");
	}
}


compBlock* getGreedyBlock(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict)
{
	try
	{
		compBlock* best_block = bestBlockAtIndex(dBuff, start_ind, end_ind, bdict);

		if (best_block->dLen <= 2) best_block->command = 0;

		if (best_block->command == 0)
		{
			compBlock* next_block = nullptr;
			while (start_ind + best_block->dLen < end_ind)
			{
				next_block = bestBlockAtIndex(dBuff, start_ind + best_block->dLen, end_ind, bdict);

				if (next_block->dLen > 2) break;

				if (best_block->dLen + next_block->dLen > 0x200) break;

				best_block->dLen += next_block->dLen;
			}
			if (next_block != nullptr) delete next_block;
		}

		return best_block;
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error during call to getGreedyBlock().");
	}
}



std::vector<compBlock*> greedyCompress(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict)
{
	try
	{
		std::vector<compBlock*> greedy_blocks;


		u32 i = start_ind;
		while (i < end_ind)
		{
			compBlock* GreedyBlock = getGreedyBlock(dBuff, i, end_ind, bdict);

			greedy_blocks.push_back(GreedyBlock);

			i += GreedyBlock->dLen;

		}

		return greedy_blocks;
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error during call to greedyCompress().");
	}

}




std::vector<compBlock*> optimalCompress(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, ByteDictionary& bdict)
{
	try
	{
		return greedyCompress(dBuff, start_ind, end_ind, bdict);
	}
	catch (PrixException& pe)
	{
		throw pe;
	}
	catch (std::exception& e)
	{
		throw PrixException("Unhandled error during call to optimalCompress().");
	}
}





/*
std::vector<compBlock*> optimalCompress(std::vector<u8>& dBuff, u32 start_ind, u32 end_ind, u32 bufferSize, ByteDictionary& bdict)
{

	if (end_ind == start_ind)
	{
		std::vector<compBlock*> default_comp;
		return default_comp;
	}


	if (end_ind == start_ind + 1)
	{
		std::vector<compBlock*> default_comp;
		compBlock* default_block = new compBlock;

		//default_block->dBuffIt = dBuff.begin() + start_ind;
		default_block->dLen = 1;
		default_block->cLen = 1;
		//if (cLen > 0x20) default_block->extended = true;
		default_block->command = 0;
		default_block->dBuffOff = start_ind;

		default_comp.push_back(default_block);

		return default_comp;
	}

	std::vector<compBlock*> prevOptimal = optimalCompress(dBuff, start_ind, end_ind-1, bufferSize, bdict);


	compBlock* final_block = *(prevOptimal.rbegin());

	//prevOptimal.pop_back();

	u32 curr_ind = end_ind-1;

	if (final_block->dLen > curr_ind - final_block->dBuffOff)
	{

		compBlock* next_block = getGreedyBlock(dBuff, curr_ind, bufferSize, bdict);

		if (next_block->comp_factor() > final_block->comp_factor())


		final_block->dLen++;
		prevOptimal.pop_back();
		prevOptimal.push_back(final_block);

		return prevOptimal;
	}







	bool last_block_minimized = false;

	if (final_block->command == 4 || final_block->command == 6)
	{
		if (bdict.GetMaxBackRange(final_block->dBuffOff).getLen() == final_block->dLen) last_block_minimized = true;
	}
	else if (final_block->command == 5)
	{
		if (bdict.GetInvertMaxBackRange(final_block->dBuffOff).getLen() == final_block->dLen) last_block_minimized = true;
	}
	else if (final_block->command == 1)
	{
		if (dBuff[curr_ind] != dBuff[curr_ind - 1]) last_block_minimized = true;
	}
	else if (final_block->command == 2)
	{
		if (curr_ind > final_block->dBuffOff + 2)
		{
			if (dBuff[curr_ind] != dBuff[curr_ind - 2]) last_block_minimized = true;
		}
	}
	else if (final_block->command == 3)
	{
		if ((dBuff[curr_ind]%256) != ((dBuff[curr_ind - 1]+1)%256)) last_block_minimized = true;
	}



	if (final_block->command == 0)
	{


	}
	if (last_block_minimized)
	{
		compBlock* default_block = getGreedyBlock(dBuff, curr_ind, bufferSize, bdict);

		prevOptimal.push_back(default_block);

		return prevOptimal;
	}
	

	




}
*/