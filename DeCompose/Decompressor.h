

#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include "Utility.h"
#include <vector>

std::vector<u8> Decompress(std::vector<u8>& buffer);
std::vector<u8> Decompress(u8 buffer[], u32 bufferSize);


#endif
