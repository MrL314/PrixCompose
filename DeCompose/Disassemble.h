

#ifndef MUSIC_FILE_DISASSEMBLE
#define MUSIC_FILE_DISASSEMBLE

#include "Utility.h"
#include <string>
#include <map>
#include <vector>




class MF_Disassembler
{

	u8* buffer;
	u32 bufferSize;
	u16 offset;
	u16 spc_addr;
	std::map<u16, std::string> object_names;

	bool has_identifier = false;
	bool has_object_names = false;


	std::vector<u16> song_ptrs;
	std::vector<u16> block_ptrs;
	std::vector<u16> voice_ptrs;
	std::vector<u16> call_ptrs;

public:

	MF_Disassembler();
	MF_Disassembler(u8* input_buffer, u32 buffer_size);
	MF_Disassembler(std::vector<u8>& input_buffer);

	void PrintBuffer();



	MF_Disassembler(const MF_Disassembler& d);
	MF_Disassembler& operator=(const MF_Disassembler& d);
	~MF_Disassembler();



	bool ContainsIdenitifier();
	
	bool ContainsObjectNames();

	void HandleObjectNames(u8* object_name_buffer);


	std::string ParsePointer(u16 p);

	std::string DisassemblePhraseInstruction();
	std::string DisassemblePhraseBlock();
	std::string DisassembleCommand();
	std::string DisassembleCommand(bool isRoutine);

	void DisassembleTo(std::vector<std::string>& output_lines);



};





#endif
