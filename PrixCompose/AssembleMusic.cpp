
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <stdexcept>

#include "AssembleMusic.h"
#include "ByteToString.h"
#include "SMKmusicdata.h"
#include "ErrorHandle.h"

#include "Compressor.h"



















int parseType(std::string& obj)
{
	try
	{
		std::vector<std::string> found_literals;
		for (int i = 0; i < NUM_TYPES; i++)
		{
			if (obj.find(TYPE_LITERALS[i]) != std::string::npos)
			{
				if (obj.find(TYPE_LITERALS[i]) == 0)
				{
					found_literals.push_back(std::string(TYPE_LITERALS[i]));
				}
			}
		}

		if (found_literals.size() == 0) return TYPE_COMMAND;

		std::string type_lit = found_literals[0];

		for (unsigned int i = 0; i < found_literals.size(); i++)
		{
			if (found_literals[i].length() > type_lit.length())
			{
				type_lit = found_literals[i];
			}
		}

		if (type_lit.compare(COMMENT_LIT) == 0) return TYPE_COMMENT;
		else if (type_lit.compare(POINTER_LIT) == 0) return TYPE_POINTER;
		else if (type_lit.compare(OBJECT_LIT) == 0) return TYPE_OBJECT;
		else if (type_lit.compare(TABLE_LIT) == 0) return TYPE_TABLE;
		else if (type_lit.compare(VARIABLE_LIT) == 0) return TYPE_VARIABLE;
		else if (type_lit.compare(REFERENCE_LIT) == 0) return TYPE_REFERENCE;
		else if (type_lit.compare(HEX_LIT) == 0) return TYPE_HEX;
		else if (type_lit.compare(BIN_LIT) == 0) return TYPE_BIN;
		else if (type_lit.compare(DEC_LIT) == 0) return TYPE_DEC;
		else if (type_lit.compare(NOTE_LIT) == 0) return TYPE_NOTE;
		else if (type_lit.compare(NULL_LIT) == 0) return TYPE_NULL;

		else return -1;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during parseType().", -1, e.what());
	}
}


std::string parseDataString(std::string raw_text, int type)
{
	try
	{
		if (type == TYPE_COMMAND) return raw_text;

		int type_len = std::string(TYPE_LITERALS[type]).length();

		raw_text.erase(0, type_len);

		return raw_text;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during parseDataString().", -1, e.what());
	}
}


void convertByteObjectDataToHex(ByteObject& b)
{
	try
	{
		if (b.type != TYPE_HEX)
		{
			int DATA_AS_INT;
			u16 new_size = 0;
			if (b.type == TYPE_BIN)
			{
				// binary to hex
				DATA_AS_INT = stoi(b.data, 0, 2);
				new_size = b.size;

			}
			else if (b.type == TYPE_DEC)
			{
				// decimal to hex
				new_size = 1;
				DATA_AS_INT = stoi(b.data);
			}
			else if (b.type == TYPE_NOTE)
			{
				// note to hex

				std::string old_data = b.data;

				int octave = (int)(b.data[b.data.length() - 1] - '0'); // octave. need to subtract 1 to get note hex later
				if (octave < 1 || octave > 6)
				{
					// throw error that the note is not in a valid octave
					throw PrixException("Invalid octave: " + std::to_string(octave));
				}
				else
				{
					int note_number = 0;
					b.data.erase(b.data.length() - 1, 1); // now b.data is the note name
					b.data[0] = toupper(b.data[0]);

					// find note
					if (b.data.compare("Cb") == 0) note_number = -1;
					else if (b.data.compare("C") == 0) note_number = 0;
					else if (b.data.compare("C#") == 0) note_number = 1;
					else if (b.data.compare("Db") == 0) note_number = 1;
					else if (b.data.compare("D") == 0) note_number = 2;
					else if (b.data.compare("D#") == 0) note_number = 3;
					else if (b.data.compare("Eb") == 0) note_number = 3;
					else if (b.data.compare("E") == 0) note_number = 4;
					else if (b.data.compare("E#") == 0) note_number = 5;
					else if (b.data.compare("Fb") == 0) note_number = 4;
					else if (b.data.compare("F") == 0) note_number = 5;
					else if (b.data.compare("F#") == 0) note_number = 6;
					else if (b.data.compare("Gb") == 0) note_number = 6;
					else if (b.data.compare("G") == 0) note_number = 7;
					else if (b.data.compare("G#") == 0) note_number = 8;
					else if (b.data.compare("Ab") == 0) note_number = 8;
					else if (b.data.compare("A") == 0) note_number = 9;
					else if (b.data.compare("A#") == 0) note_number = 10;
					else if (b.data.compare("Bb") == 0) note_number = 10;
					else if (b.data.compare("B") == 0) note_number = 11;
					else if (b.data.compare("B#") == 0) note_number = 12;
					else
					{
						// throw error not valid note name
						throw PrixException("Invalid note name: " + old_data);
					}

					if ((note_number == -1 && octave == 1) || (note_number == 12 && octave == 6))
					{
						// throw error note out of vocal range
						throw PrixException("Note out of vocal range: " + old_data);
					}
					else
					{
						new_size = 1;
						DATA_AS_INT = (12 * (octave - 1)) + note_number + 0x80;
					}
				}
				b.data = old_data; // just in case
			}
			else if (b.type == TYPE_NULL)
			{
				// null to hex
				new_size = 2;
				DATA_AS_INT = 0;
			}
			else if (b.type == TYPE_COMMAND)
			{
				// command to hex
				if (b.command_number >= PHRASE_COMMAND_BASE)
				{
					new_size = 2;
					DATA_AS_INT = stoi(b.data, 0, 16);
				}
				else if (b.command_number >= PARAMETER_COMMAND_BASE)
				{
					new_size = 1;
					DATA_AS_INT = (int)b.command_number;
				}
				else
				{
					if (b.command_number == NOTE_COMMAND_BASE + 0) // end command
					{
						new_size = 1;
						DATA_AS_INT = 0;
					}
					else if (b.command_number == NOTE_COMMAND_BASE + 1) // return command
					{
						new_size = 1;
						DATA_AS_INT = 0;
					}
					else if (b.command_number == NOTE_COMMAND_BASE + 6) // rest command
					{
						new_size = 1;
						DATA_AS_INT = 0xc9;
					}
					else if (b.command_number == NOTE_COMMAND_BASE + 7) // tie command
					{
						new_size = 1;
						DATA_AS_INT = 0xc8;
					}
				}
			}


			if (new_size != 0)
			{
				std::stringstream ss;
				ss << std::hex << DATA_AS_INT;
				std::string out_str = ss.str();
				while (out_str.length() < (unsigned int)(2 * b.size))
					out_str = '0' + out_str;
				b.data = out_str;

				b.size = new_size;
				b.type = TYPE_HEX;
			}
		}
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during convertByteObjectDataToHex().", -1, e.what());
	}
}










std::string cleanLine(std::string& LINE)
{
	try
	{
		std::size_t ind = LINE.find("\t");
		while (ind != std::string::npos)
		{
			LINE.erase(ind, 1);
			ind = LINE.find("\t");
		}

		ind = LINE.find("\n");
		while (ind != std::string::npos)
		{
			LINE.erase(ind, 1);
			ind = LINE.find("\n");
		}

		ind = LINE.find("\r");
		while (ind != std::string::npos)
		{
			LINE.erase(ind, 1);
			ind = LINE.find("\r");
		}

		while (LINE[0] == ' ') LINE.erase(0, 1);
		while (LINE[LINE.length() - 1] == ' ') LINE.erase(LINE.length() - 1, 1);


		std::size_t comment_ind = LINE.find(COMMENT_LIT);

		if (comment_ind != std::string::npos)
			LINE.erase(comment_ind);

		if (LINE.length() == 0 || LINE.length() == 1) return std::string("");

		std::vector<std::string> splitt = splitString(LINE, ' ');

		std::string new_line = joinString(splitt, SEPARATOR_LIT[0]);

		int i = 0;
		while (i < new_line.length() - 1)
		{
			if (new_line[i] == SEPARATOR_LIT[0])
			{
				while (i + 1 < new_line.length() && new_line[i + 1] == SEPARATOR_LIT[0])
				{
					new_line.erase(i + 1, 1);
				}
			}
			i++;
		}

		while (new_line[0] == SEPARATOR_LIT[0]) new_line.erase(0, 1);
		while (new_line[new_line.length() - 1] == SEPARATOR_LIT[0]) new_line.erase(new_line.length() - 1, 1);

		if (new_line.length() < 2) new_line = "";

		return new_line;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during cleanLine().", -1, e.what());
	}
}

std::vector<std::string> splitString(std::string& s, char delim)
{
	try
	{
		std::vector<std::string> tokens;
		std::string tok;
		std::stringstream tokStrm(s);
		while (std::getline(tokStrm, tok, delim))
		{
			tokens.push_back(tok);
		}
		return tokens;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during splitString().", -1, e.what());
	}
}

std::string joinString(std::vector<std::string>& v, char join_char)
{
	try
	{
		std::stringstream joined;

		for (auto s : v)
		{
			if (!s.empty())
				joined << s << join_char;
		}
		std::string joined_string = joined.str();
		joined_string.erase(joined_string.end() - 1);

		return joined_string;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during joinString().", -1, e.what());
	}
}




ByteObject::ByteObject(std::string object_data)
{
	try
	{
		type = parseType(object_data);
		data = parseDataString(object_data, type);
		ptr_offset = 0;

		if (type == TYPE_COMMAND) type_str = "";
		else if (type == TYPE_POINTER) type_str = POINTER_LIT;
		else if (type == TYPE_OBJECT) type_str = OBJECT_LIT;
		else if (type == TYPE_VARIABLE) type_str = VARIABLE_LIT;
		else if (type == TYPE_REFERENCE) type_str = REFERENCE_LIT;
		else if (type == TYPE_TABLE) type_str = TABLE_LIT;
		else if (type == TYPE_HEX) type_str = HEX_LIT;
		else if (type == TYPE_BIN) type_str = BIN_LIT;
		else if (type == TYPE_DEC) type_str = DEC_LIT;
		else if (type == TYPE_NOTE) type_str = NOTE_LIT;
		else if (type == TYPE_NULL) type_str = NULL_LIT;
		else if (type == TYPE_COMMENT) type_str = COMMENT_LIT;

		command_number = 0;


		if (type != TYPE_POINTER)
		{
			if (data.find(std::string(1, PTR_OFFSET_LIT[0])) != std::string::npos || data.find(std::string(1, PTR_OFFSET_LIT[1])) != std::string::npos)
			{
				// throw error cannot use pointer offset on non-pointer type
				throw PrixException("Cannot use pointer offset on non-pointer type.");
			}
		}



		// get size of byte object
		if (type == TYPE_COMMAND)
		{
			bool found_command = false;

			size = 1;




			if (!found_command)
			{
				for (int i = 0; i < 9; i++)
				{
					if (data.compare(VOICE_COMMANDS[i]) == 0)
					{
						//size = 1;
						command_number = NOTE_COMMAND_BASE + i;
						found_command = true;
						break;
					}
				}
			}

			if (!found_command)
			{
				for (int i = 0; i < 27; i++)
				{
					if (data.compare(NSPC_COMMANDS[i]) == 0)
					{
						//size = 1;
						command_number = PARAMETER_COMMAND_BASE + i;
						found_command = true;
						break;
					}
				}
			}

			if (!found_command)
			{
				for (int i = 0; i < 4; i++)
				{
					if (data.compare(PHRASE_COMMANDS[i]) == 0)
					{
						//size = 1;
						command_number = PHRASE_COMMAND_BASE + i;
						found_command = true;
						break;
					}
				}
			}



			if (!found_command)
			{
				// throw new error INVALID_COMMAND 
				throw PrixException("Invalid Command: " + std::string(data));
			}
			else
			{
				if (command_number >= PHRASE_COMMAND_BASE)
				{
					if (command_number == PHRASE_COMMAND_BASE + 0) size = 2; // end song command
					else if (command_number == PHRASE_COMMAND_BASE + 1) size = 0; // play phrase command
					else if (command_number == PHRASE_COMMAND_BASE + 2) size = 0; // loop command
					else if (command_number == PHRASE_COMMAND_BASE + 3) size = 2; // goto command
				}
				else if (command_number < PARAMETER_COMMAND_BASE)
				{
					//if (command_number == PARAMETER_COMMAND_BASE + 0) size = 1;
					//else if (command_number == PARAMETER_COMMAND_BASE + 1) size = 1;

					if (command_number == NOTE_COMMAND_BASE + 2) size = 0; // set duration
					else if (command_number == NOTE_COMMAND_BASE + 3) size = 0; // set dsv
					else if (command_number == NOTE_COMMAND_BASE + 4) size = 0; // set sus and vel
					else if (command_number == NOTE_COMMAND_BASE + 5) size = 0; // play note
				}
			}
		}
		else if (type == TYPE_POINTER)
		{
			size = 2;


			size_t offset_lit_ind = data.find(std::string(1, PTR_OFFSET_LIT[0]));
			if (offset_lit_ind != std::string::npos)
			{
				size_t offset_lit_end_ind = data.rfind(std::string(1, PTR_OFFSET_LIT[1]));
				if (offset_lit_end_ind != std::string::npos)
				{
					if (offset_lit_end_ind - offset_lit_ind > 1)
					{
						// get offset from pointer given
						ptr_offset = (u16)stoi(data.substr(offset_lit_ind + 1, (offset_lit_end_ind - offset_lit_ind) - 1));
						data.erase(offset_lit_ind);
					}
					else
					{
						// throw error that no offset was given
						throw PrixException("Offset value required when using pointer offset.");
					}
				}
				else
				{
					// throw error invalid syntax for offset
					throw PrixException("Invalid syntax for pointer offset.");
				}
			}
			else
			{
				size_t offset_lit_end_ind = data.rfind(std::string(1, PTR_OFFSET_LIT[1]));
				if (offset_lit_end_ind != std::string::npos)
				{
					// throw error invalid syntax for offset
					throw PrixException("Invalid syntax for pointer offset.");
				}
			}


		}
		else if (type == TYPE_OBJECT) size = 0;
		else if (type == TYPE_TABLE) size = 0;
		else if (type == TYPE_VARIABLE) size = 0;
		else if (type == TYPE_REFERENCE)
		{
			size = 0; // change later?
		}
		else if (type == TYPE_HEX)
		{
			size = ((data.length() - 1) / 2) + 1;
			while (data.length() < size * 2) data.insert(0, "0");
		}
		else if (type == TYPE_BIN) size = 1;
		else if (type == TYPE_DEC) size = 1;
		else if (type == TYPE_NOTE) size = 1;
		else if (type == TYPE_NULL) size = 2;
		else size = 0;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during ByteObject::ByteObject().", -1, e.what());
	}
	
}

std::string ByteObject::raw_data()
{
	try
	{
		std::stringstream ss;
		ss << type_str << data;
		return ss.str();
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during ByteObject::raw_data().", -1, e.what());
	}
}



//InstructionLine::InstructionLine(std::string line, std::map<std::string, ByteObject>& variables)
InstructionLine::InstructionLine(std::string line, int line_number)
{
	try
	{
		raw_line = line;
		line_num = line_number;
		byte_objects.clear();
		std::string cleaned_line = cleanLine(line);

		if (cleaned_line.length() == 0) return;

		std::vector<std::string> split_vec = splitString(cleaned_line, SEPARATOR_LIT[0]);

		for (int i = 0; i < split_vec.size(); i++)
		{
			try
			{
				byte_objects.push_back(new ByteObject(split_vec[i]));
			}
			catch (const PrixException& pe)
			{
				throw PrixException(pe.get_msg(), line_number, line);
			}
			catch (const std::exception& e)
			{
				throw PrixException("Unhandled error during creation of ByteObjects.", -1, e.what());
			}
		}
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during InstructionLine::InstructionLine().", -1, e.what());
	}

}


u16 InstructionLine::getSize()
{
	u16 s = 0;
	for (auto o : byte_objects)
		s += o->size;

	return s;
}


template <typename T>
void AddTo(std::vector<T>& vec, T val)
{
	try
	{
		bool found = false;
		for (auto v : vec)
		{
			if (v == val)
			{
				found = true;
				break;
			}
		}
		if (!found) vec.push_back(val);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during AddTo().", -1, e.what());
	}
}






AssembledMusic* assembleMusic(std::vector<std::string>& fileLines)
{

	try
	{

		std::string header_label = "header";
		std::string song_list_label = "songs";



		std::vector<InstructionLine*> InstructionLines;

		int line_num = 1;

		for (auto line : fileLines)
		{
			InstructionLine* curr_line;
			try
			{
				curr_line = new InstructionLine(line, line_num);
			}
			catch (const PrixException& pe)
			{
				throw;
			}
			catch (const std::exception& e)
			{
				throw PrixException("Unhandled error during instruction line creation.", -1, e.what());
			}

			line_num++;
			if (!curr_line->byte_objects.empty())
			{
				InstructionLines.push_back(curr_line);
			}
			else
			{
				if (curr_line != nullptr) delete curr_line;
			}
		}


		///////// TEST
		/*
		for (auto line : InstructionLines)
		{
			bool first = true;
			for (auto b : line->byte_objects)
			{
				if (!first) std::cout << SEPARATOR_LIT;
				else first = false;

				std::cout << b->type_str << b->data;
			}
			std::cout << std::endl;
		}
		*/
		///////// TEST


		std::map<std::string, ByteObject*> variables;
		std::map<std::string, ByteObject*> instruction_pointers;

		std::vector<std::string> used_song_pointers;
		std::vector<std::string> used_phrase_pointers;
		std::vector<std::string> used_voice_pointers;
		std::vector<std::string> goto_pointers;

		std::vector<InstructionBlock*> instruction_blocks;



		// phase 0: setting up variables and objects
		for (auto it = InstructionLines.begin(); it != InstructionLines.end(); ++it)
		{
			auto byte_objects = (*it)->byte_objects;
			for (int i = 0; i < byte_objects.size(); i++)
			{
				if (byte_objects[i]->type == TYPE_VARIABLE)
				{
					if (i < byte_objects.size() - 1)
					{
						variables[byte_objects[i]->data] = byte_objects[i + 1];
					}
				}
				else if (byte_objects[i]->type == TYPE_OBJECT)
				{
					instruction_pointers[byte_objects[i]->data] = byte_objects[i];
				}
			}
		}


		// phase 1: replace references with variable values
		for (auto it = InstructionLines.begin(); it != InstructionLines.end(); ++it)
		{
			auto byte_objects = (*it)->byte_objects;
			unsigned int INDEX = 0;
			for (auto it2 = byte_objects.begin(); it2 != byte_objects.end(); ++it2)
			{
				if ((*it2)->type == TYPE_REFERENCE)
				{
					std::string var_name = std::string((*it2)->data);

					if (*it2 != nullptr) delete *it2; // check shouldnt be necessary but just in case

					if (variables.count(var_name) == 0)
					{
						// throw error invalid variable
						throw PrixException("Variable does not exist: " + var_name, (*it)->line_num, (*it)->raw_line);
					}


					ByteObject var = *(variables[var_name]);
					ByteObject* ref = new ByteObject(NULL_LIT);
					ref->data = var.data;
					ref->type = var.type;
					ref->type_str = var.type_str;
					ref->size = var.size;
					ref->ptr_offset = var.ptr_offset;
					ref->data_literal = var.data_literal;
					ref->command_number = var.command_number;

					(*it)->byte_objects[INDEX] = ref;
				}
				INDEX++;
			}
		}





		instruction_blocks.push_back(new InstructionBlock(header_label));

		InstructionBlock* current_block = instruction_blocks.back();
		// phase 2: get instruction blocks
		// .. first block should always be header UNLESS header unspecified, in which case all data before first block will be header data
		bool header_defined = false;

		for (auto it = InstructionLines.begin(); it != InstructionLines.end(); ++it)
		{
			//auto byte_objects = (*it)->byte_objects;

			if ((*it)->byte_objects[0]->type == TYPE_OBJECT)
			{
				if ((*it)->byte_objects[0]->data.compare(current_block->block_name) != 0)
				{

					if ((*it)->byte_objects[0]->data.compare(header_label) == 0)
					{
						// throw error about misplaced header
						throw PrixException("Header block is misplaced. Make sure header is at beginning of file.", (*it)->line_num, (*it)->raw_line);
					}
					else
					{
						bool block_name_already_defined = false;
						for (auto b : instruction_blocks)
						{
							if ((*it)->byte_objects[0]->data.compare(b->block_name) == 0)
							{
								block_name_already_defined = true;
								break;
							}
						}

						if (block_name_already_defined)
						{
							// throw error about already defined block name
							throw PrixException("Cannot redefine block: " + (*it)->byte_objects[0]->data, (*it)->line_num, (*it)->raw_line);
						}
						else
						{
							instruction_blocks.push_back(new InstructionBlock((*it)->byte_objects[0]->data));
							current_block = instruction_blocks.back();
						}
					}
				}
				else
				{
					if ((*it)->byte_objects[0]->data.compare(header_label) != 0)
					{
						// throw error about redefining block
						throw PrixException("Cannot redefine block: " + current_block->block_name, (*it)->line_num, (*it)->raw_line);
					}
					else
					{
						if (!header_defined) header_defined = true;
						else
						{
							// throw error about redefining header block
							throw PrixException("Cannot redefine header block outside of correct scope.", (*it)->line_num, (*it)->raw_line);
						}
					}
				}
			}
			else
			{
				current_block->block_lines.push_back(*it);
			}

		}




		// phase 3: get song list
		auto songs_block_it = instruction_blocks.end();
		for (auto it = instruction_blocks.begin(); it != instruction_blocks.end(); ++it)
		{
			if ((*it)->block_name.compare(song_list_label) == 0)
			{
				songs_block_it = it;
				break;
			}
		}

		if (songs_block_it == instruction_blocks.end())
		{
			// throw error about songs list not included??
			throw PrixException(".songs block not present.");
		}
		else
		{
			for (auto it = (*songs_block_it)->block_lines.begin(); it != (*songs_block_it)->block_lines.end(); ++it)
			{
				for (auto it2 = (*it)->byte_objects.begin(); it2 != (*it)->byte_objects.end(); ++it2)
				{
					// if data is a pointer, then 
					// push back name of song pointer that will be used, with no repeats
					if ((*it2)->type == TYPE_POINTER) AddTo(used_song_pointers, (*it2)->data);
				}
			}
		}





		//int spc_var_val = (u16)stoi(variables["SPC_ADDR"]->data);


		u16 SPC_address = (u16)stoi(variables["SPC_ADDR"]->data, 0, 16);

		u16 offset = SPC_address;

		std::map<u16, std::string> pointer_names;

		std::vector<u8> assembled_bytes;


		std::vector<InstructionBlock*> used_blocks;




		// phase 4: get list of used phrases
		for (auto song_ptr : used_song_pointers)
		{
			used_phrase_pointers.clear();
			used_voice_pointers.clear();


			auto song_it = instruction_blocks.end();
			for (auto it2 = instruction_blocks.begin(); it2 != instruction_blocks.end(); ++it2)
			{
				if ((*it2)->block_name.compare(song_ptr) == 0)
				{
					song_it = it2;
					break;
				}
			}


			if (song_it == instruction_blocks.end())
			{
				// throw error about song not existing in given file
				throw PrixException("Song does not exist: " + song_ptr);
				break;
			}
			else
			{
				// parse phrase instructions here, and add corresponding phrase pointers to list
				std::string phrase_ptr = "";


				for (auto line : (*song_it)->block_lines)
				{
					if (!line->byte_objects.empty())
					{
						// instruction0 = line->byte_objects[0]

						if (line->byte_objects[0]->type == TYPE_COMMAND) // if line is an instruction
						{
							if (line->byte_objects[0]->command_number == PHRASE_COMMAND_BASE + 0) // end song
							{
								line->byte_objects[0]->data = "0000";
							}
							else if (line->byte_objects[0]->command_number == PHRASE_COMMAND_BASE + 1) // play phrase
							{
								if (line->byte_objects[1]->type == TYPE_POINTER) AddTo(used_phrase_pointers, line->byte_objects[1]->data);
								else
								{
									// throw error about "Play Phrase" instruction not having proper parameters
									throw PrixException("Improper parameters for \"Play Phrase\" command.", line->line_num, line->raw_line);
								}
							}
							else if (line->byte_objects[0]->command_number == PHRASE_COMMAND_BASE + 2) // loop phrase
							{
								// todo: probably fix so that loop command is easier to parse???
								if (line->byte_objects.size() > 2)
								{
									if (line->byte_objects[1]->type == TYPE_POINTER) AddTo(used_phrase_pointers, line->byte_objects[1]->data);
									else
									{
										// throw error about improper syntax for "Loop" command
										throw PrixException("\"Loop\" command deprecated for Super Mario Kart.", line->line_num, line->raw_line);
										//throw PrixException("Improper syntax for \"Loop\" command.", line->line_num, line->raw_line.c_str());
									}

									// reorder loop instruction
									ByteObject* temp = line->byte_objects[1];
									line->byte_objects[1] = line->byte_objects[2];
									line->byte_objects[2] = temp;

								}
							}
							else if (line->byte_objects[0]->command_number == PHRASE_COMMAND_BASE + 3) // goto
							{
								line->byte_objects[0]->data = "00ff";
								if (line->byte_objects[1]->type == TYPE_POINTER) AddTo(goto_pointers, line->byte_objects[1]->data);
							}
						}
						else
						{
							// throw error about improper line starter not being command
							throw PrixException("Improper syntax.", line->line_num, line->raw_line);
						}
					}
				}
			}





			// phase 5: get list of used voices
			for (auto phrase_ptr : used_phrase_pointers)
			{
				auto phrase_it = instruction_blocks.end();
				for (auto it2 = instruction_blocks.begin(); it2 != instruction_blocks.end(); ++it2)
				{
					if ((*it2)->block_name.compare(phrase_ptr) == 0)
					{
						phrase_it = it2;
						break;
					}
				}


				if (phrase_it == instruction_blocks.end())
				{
					// throw error about phrase not existing in given file
					throw PrixException("Phrase Block does not exist: " + phrase_ptr);
				}
				else
				{
					// parse voice pointers here, and add corresponding phrase pointers to list

					for (auto line : (*phrase_it)->block_lines)
					{
						if (!line->byte_objects.empty())
						{
							if (line->byte_objects[0]->type == TYPE_TABLE || line->byte_objects[0]->type == TYPE_POINTER)
							{
								for (auto byte : line->byte_objects)
								{
									if (byte->type == TYPE_POINTER) AddTo(used_voice_pointers, byte->data);
								}
							}
							else
							{
								// throw error about inproperly formatted table for phrase block
								throw PrixException("Improper format for Phrase Block.");
							}
						}
					}
				}
			}



			// phase 6: get routines used by voices that will be called
			unsigned int index = 0;
			while (index < used_voice_pointers.size())
			{
				std::string voice_ptr = used_voice_pointers[index++];

				auto voice_it = instruction_blocks.end();
				for (auto it2 = instruction_blocks.begin(); it2 != instruction_blocks.end(); ++it2)
				{
					if ((*it2)->block_name.compare(voice_ptr) == 0)
					{
						voice_it = it2;
						break;
					}
				}

				if (voice_it == instruction_blocks.end())
				{
					// throw error about voice not existing in given file
					throw PrixException("Voice does not exist: " + voice_ptr);
				}
				else
				{
					for (auto line : (*voice_it)->block_lines)
					{
						u8 p_command_num = 0;
						if (line->byte_objects[0]->type == TYPE_COMMAND)
						{
							if (line->byte_objects[0]->command_number == NOTE_COMMAND_BASE + 0) break; // END command
							if (line->byte_objects[0]->command_number == NOTE_COMMAND_BASE + 1) break; // RETURN command
							if (line->byte_objects[0]->command_number == 0xef) // CALL command
							{
								// add possibility to omit call amt, and default that amt to 1
								u16 size_of_call_line = 0;
								for (auto byte : line->byte_objects) size_of_call_line += byte->size;

								if (size_of_call_line - 1 == 2)
								{
									// assume that call amt is omitted
									std::stringstream ss;
									ss << HEX_LIT << "01";
									line->byte_objects.push_back(new ByteObject(ss.str()));
								}
								else if (size_of_call_line - 1 == 1)
								{
									// throw error not enough args for CALL command
									throw PrixException("Improper number of arguments for \"Call\" command. Requires call address and call amount.", line->line_num, line->raw_line);
								}

								if (line->byte_objects[1]->type == TYPE_POINTER) AddTo(used_voice_pointers, line->byte_objects[1]->data); // add routine pointer to voices that will be assembled
							}
							else if (line->byte_objects[0]->command_number == NOTE_COMMAND_BASE + 8) // percussion note command
							{
								if (line->byte_objects.size() != 2)
								{
									// throw error not enough args for percussion note command
									throw PrixException("Improper number of arguments for \"Play Percussion Note\" command.", line->line_num, line->raw_line);
								}
								else
								{
									// todo: add parsing for PERC_NOTE_x 
									int perc_num = stoi(line->byte_objects[1]->data, 0, 16);
									if (perc_num < 0xca) perc_num += 0xca;

									if (perc_num < 0xca || perc_num > 0xdf)
									{
										// throw error invalid argument for percussion note
										throw PrixException("Invalid argument for \"Play Percussion Note\" command.", line->line_num, line->raw_line);
									}
									else
									{
										std::stringstream ss;
										ss << HEX_LIT << ToHexString((u8)perc_num);
										line->byte_objects[1]->data = ss.str();
										line->byte_objects[1]->type = TYPE_HEX;
										line->byte_objects[1]->type_str = HEX_LIT;
										line->byte_objects[1]->size = 1;

										line->byte_objects[0]->size = 0;
									}


								}
							}
							else if (line->byte_objects[0]->command_number == NOTE_COMMAND_BASE + 4) // set sus and vel
							{
								if (p_command_num != NOTE_COMMAND_BASE + 2) // sus and vel can only be set if dur was set in last byte
								{
									// throw error CANNOT USE SUS/VEL SET WITHOUT SETTING DUR FIRST
									throw PrixException("Cannot independently set sustain and velocity. Must set duration in previous byte.", line->line_num, line->raw_line);
								}
							}
							else if (line->byte_objects[0]->command_number >= PARAMETER_COMMAND_BASE && line->byte_objects[0]->command_number < PHRASE_COMMAND_BASE)
							{
								// check if correct number of arguments
							}
							p_command_num = line->byte_objects[0]->command_number;
						}
						else
						{
							// throw error that instruction is not a command? 
							throw PrixException("Invalid Command.", line->line_num, line->raw_line);
						}
					}

				}
			}






			// phase 7: convert pointers into their respective addresses, and emplace in used_blocks in NSPC file order

			// first head file with phrase instructions,
			(*song_it)->block_type = PHRASE_INSTRUCTIONS_BLOCK;
			used_blocks.push_back(*song_it);
			//AddTo(used_blocks, *song_it);
			pointer_names[offset] = (*song_it)->block_name;

			for (auto line : (*song_it)->block_lines)
			{
				for (auto b : line->byte_objects)
				{
					offset += b->size;
				}
			}

			//std::cout << "MADE IT TO PAST PHASE 7.0" << std::endl;


			// then emplace all phrase blocks in current song
			for (auto phrase_ptr : used_phrase_pointers)
			{
				auto phrase_it = instruction_blocks.end();
				for (auto it2 = instruction_blocks.begin(); it2 != instruction_blocks.end(); ++it2)
				{
					if ((*it2)->block_name.compare(phrase_ptr) == 0)
					{
						phrase_it = it2;
						break;
					}
				}

				if (phrase_it != instruction_blocks.end())
				{
					(*phrase_it)->block_type = VOICE_POINTERS_BLOCK;
					//AddTo(used_blocks, *phrase_it);
					used_blocks.push_back(*phrase_it);
					pointer_names[offset] = (*phrase_it)->block_name;
				}

				unsigned int num_voice_ptrs_in_phrase_block = 0;
				//unsigned int sub_size = 0;

				unsigned int line_num = 0;

				for (auto line : (*phrase_it)->block_lines)
				{
					if (line_num != 0)
					{
						for (auto byte_it = line->byte_objects.begin(); byte_it != line->byte_objects.end(); byte_it++)
						{
							if ((*byte_it)->type != TYPE_TABLE) (*phrase_it)->block_lines[0]->byte_objects.push_back((*byte_it));
							*byte_it = nullptr;
						}
					}
					line++;
				}

				while ((*phrase_it)->block_lines.size() > 1)
				{
					if ((*phrase_it)->block_lines[1] != nullptr) delete (*phrase_it)->block_lines[1];
					(*phrase_it)->block_lines.erase((*phrase_it)->block_lines.begin() + 1);
				}

				// TODO: Ensure that the only data in the PHRASE BLOCK is a table of 8 pointers

				//if (line->byte_objects[0]->type == TYPE_TABLE) sub_size = 1;

				for (auto b : (*phrase_it)->block_lines[0]->byte_objects)
				{
					offset += b->size;
					if (b->type == TYPE_POINTER || b->type == TYPE_NULL) num_voice_ptrs_in_phrase_block++;
					else
					{
						if (b->type != TYPE_TABLE)
						{
							// throw error improper type in phrase block table
							throw PrixException("Improper type in phrase block table.", (*phrase_it)->block_lines[0]->line_num, (*phrase_it)->block_lines[0]->raw_line);
						}
					}
				}

				// fill in rest of table with NULL if pointers not given
				for (unsigned int i = 0; i < 8 - num_voice_ptrs_in_phrase_block; i++)
				{
					offset += 2;
					(*phrase_it)->block_lines[0]->byte_objects.push_back(new ByteObject(NULL_LIT));
				}

			}

			//std::cout << "MADE IT TO PAST PHASE 7.1" << std::endl;
			int try_num = 0;

			// then encode all voice routines in current song
			for (auto voice_ptr : used_voice_pointers)
			{
				auto voice_it = instruction_blocks.end();
				for (auto it2 = instruction_blocks.begin(); it2 != instruction_blocks.end(); ++it2)
				{
					if ((*it2)->block_name.compare(voice_ptr) == 0)
					{
						voice_it = it2;
						break;
					}
				}


				if (voice_it != instruction_blocks.end())
				{
					(*voice_it)->block_type = VOICE_ROUTINE_BLOCK;
					//AddTo(used_blocks, *voice_it);
					used_blocks.push_back(*voice_it);
					pointer_names[offset] = (*voice_it)->block_name;
				}


				bool hit_end = false;

				for (auto line : (*voice_it)->block_lines)
				{
					if (hit_end) break;
					for (auto b : line->byte_objects)
					{

						offset += b->size;
						if (b->type == TYPE_COMMAND)
						{
							if (b->command_number == NOTE_COMMAND_BASE + 0 || b->command_number == NOTE_COMMAND_BASE + 1)
							{
								hit_end = true;
								break;
							}
						}
					}
				}
			}
		}







		// phase 8: convert pointer addresses to hex for ease in next step
		std::map<std::string, std::string> names_to_addresses;
		for (auto it = pointer_names.begin(); it != pointer_names.end(); ++it)
		{

			std::stringstream ss;
			ss << std::hex << it->first;
			std::string out_str = ss.str();
			while (out_str.length() < 4)
				out_str = '0' + out_str;

			names_to_addresses[it->second] = out_str;

		}






		// phase 10: parse instructions in given blocks into hex
		// first add the SPC address to file header
		assembled_bytes.push_back((u8)(SPC_address & 255));
		assembled_bytes.push_back((u8)((SPC_address >> 8) & 255));

		for (auto block : used_blocks)
		{
			//std::cout << block->block_name << std::endl;
			if (block->block_type == PHRASE_INSTRUCTIONS_BLOCK)
			{
				for (auto line : block->block_lines)
				{
					for (auto b : line->byte_objects)
					{
						if (b->size != 0)
						{
							if (b->type == TYPE_POINTER)
							{
								b->data = names_to_addresses[b->data];
								b->size = 2;
							}
							else convertByteObjectDataToHex(*b);

							int data = stoi(b->data, 0, 16);

							if (b->type == TYPE_POINTER) data += b->ptr_offset;
							for (u16 i = 0; i < b->size; i++)
							{
								assembled_bytes.push_back((u8)(data & 255));
								data >>= 8;
							}
						}
					}
				}
			}
			else if (block->block_type == VOICE_POINTERS_BLOCK)
			{
				int num_ptrs = 0;
				for (auto line : block->block_lines)
				{
					for (auto b : line->byte_objects)
					{
						if (b->size != 0)
						{
							if (b->type == TYPE_POINTER)
							{
								b->data = names_to_addresses[b->data];
								b->size = 2;
							}
							else convertByteObjectDataToHex(*b);

							int data = stoi(b->data, 0, 16);

							if (b->type == TYPE_POINTER) data += b->ptr_offset;
							for (u16 i = 0; i < b->size; i++)
							{
								assembled_bytes.push_back((u8)(data & 255));
								data >>= 8;
							}
							num_ptrs++;
						}
					}
				}
				while (num_ptrs < 8)
				{
					assembled_bytes.push_back(0);
					assembled_bytes.push_back(0);
					num_ptrs++;
				}
			}
			else if (block->block_type == VOICE_ROUTINE_BLOCK)
			{
				bool hit_end = false;
				for (auto line : block->block_lines)
				{
					if (hit_end) break;

					for (auto b : line->byte_objects)
					{


						if (b->size != 0)
						{
							if (b->type == TYPE_POINTER)
							{
								b->data = names_to_addresses[b->data];
								b->size = 2;
							}
							else
							{
								if (b->type == TYPE_COMMAND)
								{
									if (b->command_number == NOTE_COMMAND_BASE + 0 || b->command_number == NOTE_COMMAND_BASE + 1)
									{
										hit_end = true;
									}
								}
								convertByteObjectDataToHex(*b);
							}

							int data = stoi(b->data, 0, 16);

							if (b->type == TYPE_POINTER) data += b->ptr_offset;
							for (u16 i = 0; i < b->size; i++)
							{
								assembled_bytes.push_back((u8)(data & 255));
								data >>= 8;
							}

							if (hit_end) break;
						}
					}
				}
			}
		}



		

		int size = assembled_bytes.size() - 2; // subtract 2 since location should not be included in assembled file

		assembled_bytes.insert(assembled_bytes.begin(), 1, (u8)((size >> 8) & 255));
		assembled_bytes.insert(assembled_bytes.begin(), 1, (u8)(size & 255));

		//add tail execute command
		assembled_bytes.push_back(0);
		assembled_bytes.push_back(0);
		assembled_bytes.push_back(0);
		assembled_bytes.push_back(8);


		std::map<std::string, std::string> names_of_used_pointers;
		for (auto it = names_to_addresses.begin(); it != names_to_addresses.end(); ++it)
		{
			names_of_used_pointers[it->second] = it->first;
		}


		AssembledMusic* assembled_music = new AssembledMusic();

		assembled_music->assembled_bytes = assembled_bytes;
		assembled_music->pointer_names = names_of_used_pointers;

		return assembled_music;

	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during AssembleMusic().", -1, e.what());
	}
}




std::vector<u8> assembleFile(std::string file_name)
{

	try
	{
		std::vector<std::string> file_lines = getFileLines(file_name);

		AssembledMusic* assembled = assembleMusic(file_lines);

		std::vector<u8> assembled_bytes;
		
		for (auto b : assembled->assembled_bytes)
		{
			u8 copied_byte = b;
			assembled_bytes.push_back((u8)copied_byte);
		}

		delete assembled;

		return assembled_bytes;
	}
	catch (const PrixException& pe)
	{
		throw PrixException(pe.get_msg(), pe.get_line(), pe.get_info(), file_name);
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during assembleFile().");
	}
}


std::vector<u8> assembleLines(std::vector<std::string>& file_lines)
{
	try
	{
		AssembledMusic* assembled = assembleMusic(file_lines);

		std::vector<u8> assembled_bytes;
			
		for (auto b : assembled->assembled_bytes)
		{
			u8 copied_byte = b;
			assembled_bytes.push_back((u8)copied_byte);
		}

		delete assembled;

		return assembled_bytes;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during assembleLines().", -1, e.what());
	}

}




std::vector<std::string> getFileLines(std::string file_name)
{
	try
	{
		std::vector<std::string> file_lines;
		std::ifstream music_file(file_name);

		if (music_file)
		{
			std::string line;

			while (std::getline(music_file, line))
			{
				file_lines.push_back(line);
			}

			music_file.close();
		}
		else
		{
			// throw error CANNOT OPEN FILE
			throw PrixException("Cannot open file: " + file_name);
		}

		return file_lines;
	}
	catch (const PrixException& pe)
	{
		throw PrixException(pe.get_msg(), pe.get_line(), pe.get_info());
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during getFileLines().", -1, e.what());
	}
}





void AssembledFile::assemble_file()
{
	try
	{
		assembled_music_bytes = assembleFile(file_name + unassembled_extension);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during assemble_file().", -1, e.what());
	}
}

void AssembledFile::compress_file()
{
	try
	{
		assembled_music_bytes = compress(assembled_music_bytes);
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during compress_file().", -1, e.what());
	}
}

void AssembledFile::remove_header()
{
	try
	{
		assembled_music_bytes.erase(assembled_music_bytes.begin());
		assembled_music_bytes.erase(assembled_music_bytes.begin());
		assembled_music_bytes.erase(assembled_music_bytes.begin());
		assembled_music_bytes.erase(assembled_music_bytes.begin());

		// remove tail execute as well
		assembled_music_bytes.pop_back();
		assembled_music_bytes.pop_back();
		assembled_music_bytes.pop_back();
		assembled_music_bytes.pop_back();
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during remove_header().", -1, e.what());
	}

}



void AssembledFile::writeFile()
{
	try
	{
		std::ofstream outfile(file_name + assembled_extension, std::ios::out | std::ios_base::binary);

		char* buffer = new char[assembled_music_bytes.size()];
		int ind = 0;
		for (auto b : assembled_music_bytes)
		{
			buffer[ind++] = b;
		}

		outfile.write(buffer, assembled_music_bytes.size());
		outfile.close();

		delete[] buffer;
	}
	catch (const PrixException& pe)
	{
		throw;
	}
	catch (const std::exception& e)
	{
		throw PrixException("Unhandled error during writeFile().", -1, e.what());
	}

}
