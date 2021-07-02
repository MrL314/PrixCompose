

#include <string>
#include <vector>
#include <map>

#include "Utility.h"
#include "Commands.h"



#ifndef ASSEMBLE_MUSIC_H
#define ASSEMBLE_MUSIC_H


#define PHRASE_INSTRUCTIONS_BLOCK 1
#define VOICE_POINTERS_BLOCK 2
#define VOICE_ROUTINE_BLOCK 3


/*
* phase 1: transform all references into bytes
* phase 2: find size of all object blocks
* phase 3: calculate addresses object blocks
* phase 4: transform pointers into addresses
* phase 5: combine all objects into a single buffer
* 
* 
*/



struct ByteObject
{
	std::string data;
	std::string type_str;

	int type; 
	u16 size;

	u16 ptr_offset;

	std::string data_literal;

	u8 command_number;


	ByteObject(std::string object_data);

	void setSize(u16 s) { size = s; };

	std::string raw_data();

};



struct InstructionLine
{

	std::string raw_line;

	std::vector<ByteObject*> byte_objects;

	int line_num;


	//InstructionLine(std::string line, std::map<std::string, ByteObject>& variables);
	InstructionLine(std::string line, int line_number);

	u16 getSize();

	~InstructionLine() {
		for (auto p : byte_objects)
		{
			if (p != nullptr) delete p;
		}
		byte_objects.clear();
	}

};


struct InstructionBlock
{

	std::string block_name;

	std::vector<InstructionLine*> block_lines;

	int block_type;


	InstructionBlock()
	{
		for (auto p : block_lines)
		{
			if (p != nullptr) delete p;
		}
		block_lines.clear();
		block_type = 0;
	}

	InstructionBlock(std::string blockname)
	{
		block_name = blockname;
		for (auto p : block_lines)
		{
			if (p != nullptr) delete p;
		}
		block_lines.clear();
		block_type = 0;
	}


	~InstructionBlock() {
		for (auto p : block_lines)
		{
			if (p != nullptr) delete p;
		}
		block_lines.clear();
	}

};




struct AssembledMusic
{

	std::vector<u8> assembled_bytes;


	std::map<std::string, std::string> pointer_names;




};



struct AssembledFile
{

	// in case I want to add headers to the assembled files for disassembly
	// or i need other data stored in the file

	std::vector<u8> assembled_music_bytes;
	std::string file_name;

	//AssembledFile(AssembledMusic* a);

	AssembledFile(std::string filename) { file_name = filename;	};

	void assemble_file();
	
	void compress_file();

	void remove_header();

	void writeFile();

};



int parseType(std::string& obj);

std::string parseDataString(std::string raw_text, int type);

void convertByteObjectDataToHex(ByteObject& b);


std::string cleanLine(std::string& LINE);

std::vector<std::string> splitString(std::string& s, char delim);

std::string joinString(std::vector<std::string>& v, char join_char);

AssembledMusic* assembleMusic(std::vector<std::string>& fileLines);

std::vector<u8> assembleFile(std::string file_name);

std::vector<u8> assembleLines(std::vector<std::string>& file_lines);

std::vector<std::string> getFileLines(std::string file_name);












#endif