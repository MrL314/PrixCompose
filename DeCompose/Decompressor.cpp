
#include "Utility.h"
#include "ErrorHandle.h"
#include <vector>
#include <stdexcept>


std::vector<u8> Decompress(std::vector<u8>& buffer)
{
	try
	{
		u32 srcOff = 0;
		u32 destOff = 0;

		u32 bufferSize = buffer.size();

		std::vector<u8> destBuff;

		u8 value;
		u8 command;
		u16 count;

		while (srcOff < bufferSize)
		{
			value = buffer[srcOff++];

			if (value == 0xff) break;


			command = (value & 0xe0) >> 5;

			if (command == 7)
			{
				command = (value & 0x1c) >> 2;
				count = ((u16)(value & 3) << 8) + ((u16)buffer[srcOff++]) + 1;
			}
			else
				count = (u16)(value & 0x1f) + 1;



			if (command == 0)
			{
				// Copy Direct Bytes
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(buffer[srcOff++]);
					destOff++;
				}
			}
			else if (command == 1)
			{
				// Fill Byte
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(buffer[srcOff]);
					destOff++;
				}
				srcOff++;
			}
			else if (command == 2)
			{
				// Fill Word
				u32 j = 0;

				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(buffer[srcOff + j]);
					destOff++;
					j = 1 - j;
				}
				srcOff += 2;
			}
			else if (command == 3)
			{
				// Fill Slide
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back((u8)((u16)buffer[srcOff] + i));
					destOff++;
				}
				srcOff++;
			}
			else if (command == 4)
			{
				// Copy Buffer From Pointer
				u16 pOffset = (u16)buffer[srcOff] + ((u16)buffer[srcOff + 1] << 8);
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(destBuff[pOffset + i]);
					destOff++;
				}
				srcOff += 2;
			}
			else if (command == 5)
			{
				// Copy Invert Buffer From Pointer
				u16 pOffset = (u16)buffer[srcOff] + ((u16)buffer[srcOff + 1] << 8);
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(destBuff[pOffset + i] ^ 0xff);
					destOff++;
				}
				srcOff += 2;
			}
			else if (command == 6)
			{
				// Buffer Relative Copy
				u16 pOffset = destOff - (u16)buffer[srcOff++];
				for (u16 i = 0; i < count; i++)
				{
					destBuff.push_back(destBuff[pOffset + i]);
					destOff++;
				}
			}
			else throw PrixException("Invalid Decompression Command");
		}

		/*
		u8* decompressionBuffer = new u8[destBuff.size()];

		int index = 0;

		for (auto it = destBuff.begin(); it != destBuff.end(); ++it)
		{
			decompressionBuffer[index++] = *it;
		}

		return decompressionBuffer;
		*/
		return destBuff;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw;
	}

}


std::vector<u8> Decompress(u8 buffer[], u32 bufferSize)
{
	try
	{
		std::vector<u8> decompBuff;

		for (u32 i = 0; i < bufferSize; i++)
		{
			decompBuff.push_back(buffer[i]);
		}

		return Decompress(decompBuff);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw;
	}
}