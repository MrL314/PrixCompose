

#include <iostream>
#include <string>
#include <algorithm>

#include "Disassemble.h"
#include "Commands.h"
#include "ByteToString.h"




template <typename T>
bool inVec(T item, std::vector<T> v)
{
	return (std::find(v.begin(), v.end(), item) != v.end());
}

template <typename T>
void addToVec(T item, std::vector<T>& v)
{
	if (!inVec(item, v))
	{
		for (int i = 0; i < v.size(); i++)
		{
			if (item < v[i])
			{
				v.insert(v.begin()+i, item);
				return;
			}
		}
		v.push_back(item);
	}
}




MF_Disassembler::MF_Disassembler()
{
	buffer = nullptr;
	bufferSize = 0;
	spc_addr = 0;
	offset = 0;
}

MF_Disassembler::MF_Disassembler(u8* input_buffer, u32 buffer_size)
{
	buffer = input_buffer;
	bufferSize = buffer_size;
	spc_addr = 0;
	offset = 0;
}

MF_Disassembler::MF_Disassembler(std::vector<u8>& input_buffer)
{
	buffer = new u8[input_buffer.size()-4];
	bufferSize = (u32)input_buffer.size()-4;
	spc_addr = (u16)0;
	offset = (u16)0;

	spc_addr = ((u16)input_buffer[3] << 8) + (u16)input_buffer[2];

	unsigned int idx = 0;
	int stall = 0;
	for (auto it = input_buffer.begin(); it != input_buffer.end(); ++it)
	{
		if (stall++ > 3)
			buffer[idx++] = *it;
	}
}



void MF_Disassembler::PrintBuffer()
{

	for (u32 i = 0; i < bufferSize; i++)
	{
		std::cout << ToHexString(buffer[i]) << " ";
	}
}



MF_Disassembler::MF_Disassembler(const MF_Disassembler& d) 
{
	if (this != &d)
	{
		if (d.buffer != nullptr)
		{
			u8* new_buffer = new u8[d.bufferSize];
			std::copy(d.buffer, d.buffer + d.bufferSize, new_buffer);

			if (buffer != nullptr) delete buffer;

			buffer = new_buffer;
		}
		else buffer = nullptr;
		bufferSize = d.bufferSize;
	}
}
MF_Disassembler& MF_Disassembler::operator=(const MF_Disassembler& d) 
{
	if (this != &d)
	{
		if (d.buffer != nullptr)
		{
			u8* new_buffer = new u8[d.bufferSize];
			std::copy(d.buffer, d.buffer + d.bufferSize, new_buffer);

			if (buffer != nullptr) delete buffer;

			buffer = new_buffer;
		}
		else buffer = nullptr;
		bufferSize = d.bufferSize;
	}
	return *this;
}
MF_Disassembler::~MF_Disassembler()
{
	if (buffer != nullptr)
	{
		delete buffer;
		buffer = nullptr;
	}
}




// TODO
bool MF_Disassembler::ContainsIdenitifier() { return false;  }

// TODO
bool MF_Disassembler::ContainsObjectNames() { return false; }

// TODO
void MF_Disassembler::HandleObjectNames(u8* object_name_buffer) {}

// TODO
std::string MF_Disassembler::ParsePointer(u16 p)
{
	// UPDATE TO HANDLE NAMES LATER
	return ToHexString(p);

}




// TODO
std::string MF_Disassembler::DisassemblePhraseInstruction()
{
	
	std::stringstream line_output;

	if (SHOW_LINE_NUMBERS) line_output << ToHexString((u16)(offset + spc_addr)) << ": ";
	else line_output << "\t";

	
	if (buffer[offset] == 0x00 && buffer[offset + 1] == 0x00)
	{
		// end song
		line_output << END_SONG_CMD;
		offset += 2;
	}
	else if (buffer[offset] < 0x80 && buffer[offset + 1] == 0x00)
	{
		// loop
		line_output << LOOP_CMD << " ";
		u16 loop_amt = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];
		offset += 2;

		u16 ptr = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];
		block_ptrs.push_back(ptr);

		line_output << ParsePointer(ptr);
		offset += 2;

		line_output << SEPARATOR_LIT << HEX_LIT;
		line_output << ToHexString(loop_amt);
	}
	else if (buffer[offset] >= 0x80 && buffer[offset + 1] == 0x00)
	{
		// goto
		line_output << GOTO_CMD << " " << POINTER_LIT;
		offset += 2;
		u16 ptr = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];

		u16 ptr_off = 0;

		for (int i = song_ptrs.size()-1; i >= 0; i--)
		{
			if (ptr > song_ptrs[i])
			{
				ptr_off = ptr - song_ptrs[i];
				break;
			}
		}

		ptr -= ptr_off;


		line_output << ParsePointer(ptr);
		if (ptr_off != 0)
		{
			line_output << PTR_OFFSET_LIT[0] << ptr_off << PTR_OFFSET_LIT[1];
		}

		offset += 2;
	}
	else
	{
		// play phrase block
		line_output << PLAY_PHRASE_CMD << " " << POINTER_LIT;
		u16 ptr = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];
		block_ptrs.push_back(ptr);
		line_output << ParsePointer(ptr);
		offset += 2;

	}

	return line_output.str();
}



std::string MF_Disassembler::DisassemblePhraseBlock()
{
	std::stringstream line_output;

	if (SHOW_LINE_NUMBERS) line_output << ToHexString((u16)(offset + spc_addr)) << ": ";
	else line_output << "\t";

	line_output << TABLE_LIT << " ";

	for (int i = 0; i < 8; i++)
	{
		if (i != 0) line_output << SEPARATOR_LIT;

		if (buffer[offset] == 0x00 && buffer[offset + 1] == 0x00)
		{
			line_output << NULL_LIT;
		}
		else
		{
			line_output << POINTER_LIT;
			u16 ptr = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];
			line_output << std::string(ParsePointer(ptr));
			if (!inVec(ptr, voice_ptrs))
				voice_ptrs.push_back(ptr);
			
		}
		offset += 2;
	}

	return line_output.str();
}



std::string MF_Disassembler::DisassembleCommand(bool isRoutine) 
{
	std::stringstream line_output;

	if (SHOW_LINE_NUMBERS) line_output << ToHexString((u16)(offset + spc_addr)) << ": ";
	else line_output << "\t";


	u8 command = buffer[offset++];


	if (command == 0)
	{
		// end/return command

		if (isRoutine) line_output << RETURN_CMD;
		else line_output << END_CMD;
	}
	else if (command < 0x80)
	{
		// note parameters
		u8 next_cmd = buffer[offset];

		if (next_cmd < 0x80 && next_cmd != 0)
		{
			offset++;
			line_output << SET_DURATION_SUS_AND_VEL_CMD << " ";
			line_output << HEX_LIT << ToHexString(command) << SEPARATOR_LIT;
			line_output << HEX_LIT << ToHexString(next_cmd);
		}
		else
		{
			line_output << SET_DURATION_CMD << " ";
			line_output << HEX_LIT << ToHexString(command);
		}
	}
	else if (command < 0xc8)
	{
		// play_note
		line_output << PLAY_NOTE_CMD << " " << NOTE_LIT;

		u8 note_num = (command - 0x80) % 12;

		if (note_num == 0) line_output << "C";
		else if (note_num == 1) line_output << "C#";
		else if (note_num == 2) line_output << "D";
		else if (note_num == 3) line_output << "D#";
		else if (note_num == 4) line_output << "E";
		else if (note_num == 5) line_output << "F";
		else if (note_num == 6) line_output << "F#";
		else if (note_num == 7) line_output << "G";
		else if (note_num == 8) line_output << "G#";
		else if (note_num == 9) line_output << "A";
		else if (note_num == 10) line_output << "A#";
		else if (note_num == 11) line_output << "B";


		line_output << std::to_string(((int)(command - 0x80) / 12) + 1);

	}
	else if (command == 0xc8)
	{
		// tie
		line_output << TIE_CMD;
	}
	else if (command == 0xc9)
	{
		// rest
		line_output << REST_CMD;
	}
	else if (command < 0xe0)
	{
		// perc_note
		line_output << PLAY_PERCUSSION_SOUND_CMD << " ";
		line_output << HEX_LIT << ToHexString(buffer[offset++]);

	}
	else if (command < 0xfb && (command != 0xef))
	{
		// other commands

		line_output << NSPC_COMMANDS[command - 0xe0];

		for (int i = 0; i < NSPC_CMD_ARGS[command - 0xe0]; i++)
		{
			if (i == 0) line_output << " ";
			else line_output << SEPARATOR_LIT;

			line_output << HEX_LIT << ToHexString(buffer[offset++]);
		}
	}
	else if (command == 0xef)
	{
		// routine call
		line_output << CALL_ROUTINE_CMD << " " << POINTER_LIT;

		u16 ptr = ((u16)buffer[offset + 1] << 8) + (u16)buffer[offset];
		addToVec(ptr, call_ptrs);
		line_output << ParsePointer(ptr);
		offset += 2;

		line_output << SEPARATOR_LIT << HEX_LIT << ToHexString(buffer[offset++]);
	}
	else
	{
		// INVALID COMMAND
		line_output << "INVALID COMMAND 0x" << ToHexString(command);
	}

	//std::cout << output_line << std::endl;

	return line_output.str();
}


std::string MF_Disassembler::DisassembleCommand()
{
	return DisassembleCommand(false);
}



// TODO
void MF_Disassembler::DisassembleTo(std::vector<std::string>& output_lines) 
{
	output_lines.push_back(OBJECT_LIT + std::string("header"));
	output_lines.push_back("\t" + (VARIABLE_LIT + ("SPC_ADDR " + (HEX_LIT + ToHexString((u16)spc_addr)))));
	output_lines.push_back("");

	output_lines.push_back(OBJECT_LIT + std::string("songs"));
	output_lines.push_back("\t" + (TABLE_LIT + (" " + (POINTER_LIT + ToHexString((u16)spc_addr)))));
	output_lines.push_back("");

	call_ptrs.push_back(spc_addr + bufferSize);
	song_ptrs.push_back(spc_addr);

	auto it = block_ptrs.begin();
	// phase 1: phrase instructions

	std::string object_name = OBJECT_LIT + ToHexString((u16)(offset + spc_addr));
	output_lines.push_back(object_name);
	bool end_song = false;
	do 
	{

		if (buffer[offset] == 0x00 && buffer[offset + 1] == 0x00) end_song = true;

		output_lines.push_back(DisassemblePhraseInstruction());
		
	} while (!(inVec((u16)(offset + spc_addr), block_ptrs)) && !end_song);

	//std::cout << ToHexString((u16)(offset + spc_addr)) << std::endl;

	if (!(inVec((u16)(offset + spc_addr), block_ptrs)))
	{
		std::stringstream nonsense_line;
		nonsense_line << std::endl << COMMENT_LIT << "UNKNOWN DATA\n" << OBJECT_LIT << ToHexString((u16)(offset + spc_addr));
		output_lines.push_back(nonsense_line.str());

		do 
		{
			std::string nonsense_line = ToHexString(buffer[offset++]);

			if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
			else nonsense_line = '\t' + nonsense_line;



			output_lines.push_back(nonsense_line);
		} while (!(inVec((u16)(offset + spc_addr), block_ptrs)));
	}




	

	// phase 2: phrase blocks
	do
	{
		std::stringstream object_name;
		object_name << std::endl << OBJECT_LIT << ToHexString((u16)(offset + spc_addr));
		output_lines.push_back(object_name.str());

		output_lines.push_back(std::string(DisassemblePhraseBlock()));


		if (!(inVec((u16)(offset + spc_addr), block_ptrs)))
		{
			if (!(inVec((u16)(offset + spc_addr), voice_ptrs)))
			{
				/*
				std::string nonsense_line = "\n";
				nonsense_line += COMMENT_LIT;
				nonsense_line += "UNKNOWN DATA\n";
				nonsense_line += OBJECT_LIT;
				nonsense_line += ToHexString((u16)(offset + spc_addr));
				output_lines.push_back(nonsense_line);
				*/
				
				/*
				do
				{
					std::string nonsense_line = ToHexString(buffer[offset++]);
					/*
					if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
					else nonsense_line = '\t' + nonsense_line;

					output_lines.push_back(nonsense_line);
					
				} while (!(inVec((u16)(offset + spc_addr), block_ptrs)));
				*/
			}
		}
	} while (!(inVec((u16)(offset + spc_addr), voice_ptrs)));


	
	// phase 3: voice instructions  (done?) 

	std::sort(voice_ptrs.begin(), voice_ptrs.end());

	bool end_voice = false;

	for (int i = 0; i < voice_ptrs.size(); i++)
	{
		u16 p = voice_ptrs[i];

		end_voice = false;

		while ((spc_addr + offset) < p)
		{
			std::string nonsense_line = ToHexString(buffer[offset++]);

			if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
			else nonsense_line = '\t' + nonsense_line;

			//output_lines.push_back(nonsense_line);
		}

		std::stringstream object_name;
		object_name << std::endl << OBJECT_LIT << ToHexString((u16)(offset + spc_addr));
		output_lines.push_back(object_name.str());


		if (i != voice_ptrs.size() - 1)
		{
			u16 q = voice_ptrs[i + 1];
			do
			{
				if (buffer[offset] == 0) end_voice = true;
				output_lines.push_back(std::string(DisassembleCommand()));

			} while (!end_voice && (spc_addr + offset) < q);
		}
		else
		{
			if (call_ptrs.size() != 0)
			{
				do
				{
					if (buffer[offset] == 0) end_voice = true;
					output_lines.push_back(std::string(DisassembleCommand()));

				} while (!(inVec((u16)(offset + spc_addr), call_ptrs)) && !end_voice);
			}
			else
			{
				do
				{
					if (buffer[offset] == 0) end_voice = true;
					output_lines.push_back(std::string(DisassembleCommand()));

				} while (!end_voice);
			}

		}
	}

	
	// phase 4: routines

	std::sort(call_ptrs.begin(), call_ptrs.end());

	call_ptrs.pop_back();

	end_voice = false;

	for (int i = 0; i < call_ptrs.size(); i++)
	{
		u16 p = call_ptrs[i];

		end_voice = false;

		while ((spc_addr + offset) < p)
		{
			std::string nonsense_line = ToHexString(buffer[offset++]);

			if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
			else nonsense_line = '\t' + nonsense_line;

			//output_lines.push_back(nonsense_line);
		}

		std::stringstream object_name;
		object_name << std::endl << OBJECT_LIT << ToHexString((u16)(offset + spc_addr));
		output_lines.push_back(object_name.str());

		if (i != call_ptrs.size() - 1)
		{
			u16 q = call_ptrs[i + 1];
			do
			{
				if (buffer[offset] == 0) end_voice = true;
				output_lines.push_back(std::string(DisassembleCommand(true)));

			} while ((spc_addr + offset) < q && !end_voice);
		}
		else
		{
			do
			{
				if (buffer[offset] == 0) end_voice = true;
				output_lines.push_back(std::string(DisassembleCommand(true)));

			} while (!end_voice);
		}
	}

	/*
	// phase 6: output song footer null data before next song
	bool in_footer = true;

	while (offset < bufferSize && buffer[offset] == 0 && in_footer)
	{
		if (offset < bufferSize - 1)
		{
			if (buffer[offset + 1] == 0)
			{
				std::string nonsense_line = "00";
				if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
				else nonsense_line = '\t' + nonsense_line;


				output_lines.push_back(nonsense_line);
				offset++;
			}
			else
			{
				u16 idx = 0;
				bool exists_another_song = false;
				while (offset + idx < bufferSize-1)
				{
					if (buffer[offset + idx] == 0 && buffer[offset + idx + 1] == 0)
					{
						exists_another_song = true;
						break;
					}
					idx++;
				}

				if (exists_another_song)
				{
					if (idx % 2 != 0)
					{
						std::string nonsense_line = "00";
						if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
						else nonsense_line = '\t' + nonsense_line;
						output_lines.push_back(nonsense_line);
						offset++;
					}
					in_footer = false;
				}
			}
		}
		else
		{
			std::string nonsense_line = "00";
			if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
			else nonsense_line = '\t' + nonsense_line;

			output_lines.push_back(nonsense_line);

			nonsense_line = ToHexString(buffer[++offset]);
			if (SHOW_LINE_NUMBERS) nonsense_line = ToHexString((u16)(offset + spc_addr)) + ':' + ' ' + nonsense_line;
			else nonsense_line = '\t' + nonsense_line;

			output_lines.push_back(nonsense_line);
		}
	}

	*/

}